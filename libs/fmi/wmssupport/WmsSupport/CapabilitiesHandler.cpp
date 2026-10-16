#include "wmssupport/CapabilitiesHandler.h"
#include "wmssupport/WmsQuery.h"
#include "wmssupport/SetupParser.h"
#include "wmssupport/CapabilityTreeParser.h"
#include "xmlliteutils/XmlHelperFunctions.h"
#include "../../q2clientlib/include/NFmiQ2Client.h"
#include "NFmiFileSystem.h"
#include "NFmiLedLightStatus.h"

#include <webclient/Client.h>

#include <catlog/catlog.h>
#include <cppback/background-manager.h>


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

namespace Wms
{
    namespace
    {
        std::unique_ptr<CapabilityTree> cloneCapabilityTree(const CapabilityTree& tree)
        {
            if(const auto* node = dynamic_cast<const CapabilityNode*>(&tree))
            {
                std::vector<std::unique_ptr<CapabilityTree>> children;
                children.reserve(node->children.size());
                for(const auto& child : node->children)
                    children.push_back(cloneCapabilityTree(*child));
                return std::make_unique<CapabilityNode>(node->value, std::move(children));
            }
            return std::make_unique<CapabilityLeaf>(tree.value);
        }

        std::string fetchCapabilitiesXml(const Web::Client& client, const WmsQuery& query, bool doLogging, bool doVerboseLogging, int getCapabilitiesTimeoutInSeconds)
        {
            try
            {
                auto baseUriStr = toBaseUri(query);
                auto requestStr = toRequest(query);
                if(doLogging)
                {
                    CatLog::logMessage(std::string("fetchCapabilitiesXml Wms request, base-uri: ") + baseUriStr + " , request: " + requestStr, CatLog::Severity::Debug, CatLog::Category::NetRequest);
                }
                std::string responseStr;
                auto httpResponseFut = client.queryFor(baseUriStr, requestStr, getCapabilitiesTimeoutInSeconds);
                responseStr = httpResponseFut.get();

                // Lisätty testauskoodeja sekä olemassa olevan getCapabilities responsin tiedostotalletukseen että sieltä lukuun. Pidä normaalisti kommenteissa!!
//                NFmiFileSystem::SafeFileSave("D:\\data\\wms\\dynamicServer_flat.xml", responseStr);
//                NFmiFileSystem::ReadFile2String("D:\\data\\wms\\dynamicServer_flat_mst.xml", responseStr);

                if(doLogging)
                {
                    const size_t maxLength = 1000;
                    if(doVerboseLogging || responseStr.size() < maxLength)
                        CatLog::logMessage(std::string("fetchCapabilitiesXml response: ") + responseStr, CatLog::Severity::Debug, CatLog::Category::NetRequest);
                    else
                    {
                        auto shortLogString = std::string(responseStr.begin(), responseStr.begin() + maxLength);
                        CatLog::logMessage(std::string("fetchCapabilitiesXml response: ") + shortLogString, CatLog::Severity::Debug, CatLog::Category::NetRequest);
                    }
                }
                return responseStr;
            }
            catch(const std::exception &e)
            {
                // Errors are logged always
                CatLog::logMessage(std::string(__FUNCTION__) + " request failed: " + e.what(), CatLog::Severity::Error, CatLog::Category::NetRequest);
                throw;
            }
        }

        std::string MakeLedChannelGetCapabilityTreeReport(const std::string& server)
        {
            std::string reportStr = "Doing getcapability request parsing from server:\n";
            reportStr += server;
            return reportStr;
        }

    } // unnamed namespace ends

    std::function<void()> CapabilitiesHandler::parameterSelectionUpdateCallback_ = nullptr;

    CapabilitiesHandler::CapabilitiesHandler(
        std::unique_ptr<Web::Client> client,
        const std::shared_ptr<cppback::BackgroundManager> &bManager,
        std::chrono::seconds intervalToPollGetCapabilities,
        const std::string &proxyUrl,
        const std::unordered_map<int, DynamicServerSetup> &servers,
        std::function<void(long, const std::set<LayerInfo>&)> cacheDirtyCallback,
        std::function<bool(long, const std::string&)> cacheHitCallback,
        int capabilitiesTimeoutInSeconds
    )
        : hashesPtr_(std::make_shared<CapabilitiesHandlerHashes>())
        , client_(std::move(client))
        , cacheDirtyCallback_(cacheDirtyCallback)
        , bManager_{ bManager }
        , proxyUrl_{ proxyUrl }
        , servers_{ servers }
        , intervalToPollGetCapabilities_{ intervalToPollGetCapabilities }
        , getCapabilitiesTimeoutInSeconds_{capabilitiesTimeoutInSeconds}
        , cacheHitCallback_{ cacheHitCallback }
    {}

    void CapabilitiesHandler::setParameterSelectionUpdateCallback(std::function<void()>& parameterSelectionUpdateCallback)
    {
        parameterSelectionUpdateCallback_ = parameterSelectionUpdateCallback;
    }

    std::shared_ptr<CapabilitiesHandlerHashes> CapabilitiesHandler::getHashes() const
    {
        std::lock_guard<std::mutex> lock(hashesMutex_);
        return hashesPtr_;
    }

    void CapabilitiesHandler::setHashes(std::shared_ptr<CapabilitiesHandlerHashes> hashesPtr)
    {
        std::lock_guard<std::mutex> lock(hashesMutex_);
        hashesPtr_ = hashesPtr;
    }

    std::shared_ptr<CapabilityTree> CapabilitiesHandler::getCapabilityTree() const
    {
        std::lock_guard<std::mutex> lock(capabilityTreeMutex_);
        if(!capabilityTree_)
        {
            throw std::runtime_error("CapabilitiesHandler: peekCapabilityTree called before capabilitiesTree was initialized.");
        }
        return capabilityTree_;
    }

    void CapabilitiesHandler::setCapabilityTree(std::shared_ptr<CapabilityTree> capabilityTree)
    {
        std::lock_guard<std::mutex> lock(capabilityTreeMutex_);
        // Tehdään sijoitus, ei swap, koska tässä haluataan korvata olion sisältö ja reference-count:it
        capabilityTree_ = capabilityTree;
    }

    bool CapabilitiesHandler::isCapabilityTreeAvailable() const
    {
        std::lock_guard<std::mutex> lock(capabilityTreeMutex_);
        return capabilityTree_ != nullptr;
    }

    bool CapabilitiesHandler::getCapabilitiesHaveBeenRetrieved() const
    {
        return getCapabilitiesHaveBeenRetrieved_;
    }

    void CapabilitiesHandler::cancel()
    {
        client_->cancel();
    }

    void CapabilitiesHandler::startFetchingCapabilitiesInBackground()
    {
        bManager_->addTask([this]()
            {
                std::string workingThreadName = "GetCapabilityTree";
                // Owned only by this worker; published trees are independent copies.
                std::map<int, std::unique_ptr<CapabilityTree>> lastGoodTrees;
                while(!bManager_->isKillSignalSet())
                {
                    auto children = std::vector<std::unique_ptr<CapabilityTree>>{};
                    bool foundAnyWmsServerData = false;
                    // Tehdään kopio hashes tietorakenteesta, jotta sitä voidaan päivityksen aikan rauhassa päivitellä
                    // Wms servereiden muuttuneilla sisällöillä.
                    auto workingHashesPtr = std::make_shared<CapabilitiesHandlerHashes>(*getHashes());

                    for(auto& serverKV : servers_)
                    {
                        if(bManager_->isKillSignalSet())
                            return;
                        const auto& server = serverKV.second;
                        NFmiLedLightStatusBlockReporter blockReporter(NFmiLedChannel::WmsData, workingThreadName, MakeLedChannelGetCapabilityTreeReport(server.generic.host));
                        auto query = QueryBuilder{}.setScheme(server.generic.scheme)
                            .setHost(server.generic.host)
                            .setPath(server.generic.path)
                            .setService("WMS")
                            .setVersion(server.version)
                            .setRequest("GetCapabilities")
                            .setToken(server.generic.token)
                            .build();

                        try
                        {
                            auto capabilityTreeParser = CapabilityTreeParser{ server.producer, server.delimiter, cacheHitCallback_, server.acceptTimeDimensionalLayersOnly };
                            auto xml = fetchCapabilitiesXml(*client_, query, server.logFetchCapabilitiesRequest, server.doVerboseLogging, getCapabilitiesTimeoutInSeconds_);
                            // Parsing must not modify published hashes or change-detection history.
                            std::map<long, std::map<long, LayerInfo>> serverHashes;
                            auto candidateChanges = changedLayers_;
                            candidateChanges.changedLayers.clear();
                            auto candidateTree = capabilityTreeParser.parseXmlGeneral(xml, serverHashes, candidateChanges);
                            if(!candidateChanges.changedLayers.empty())
                                cacheDirtyCallback_(server.producer.GetIdent(), candidateChanges.changedLayers);

                            // Replace this producer only after the entire refresh has succeeded.
                            workingHashesPtr->getHashes()[server.producer.GetIdent()].swap(serverHashes[server.producer.GetIdent()]);
                            lastGoodTrees[serverKV.first] = std::move(candidateTree);
                            changedLayers_ = std::move(candidateChanges);
                            foundAnyWmsServerData = true;
                        }
                        catch(std::exception& e)
                        {
                            std::string errorMessage = "Error with dynamic Wms server '";
                            errorMessage += server.generic.host;
                            errorMessage += "', while fetching or parsing getCapabilities response: ";
                            errorMessage += e.what();
                            errorMessage += lastGoodTrees.count(serverKV.first)
                                ? "; retaining last successful capabilities."
                                : "; no previous successful capabilities available.";
                            CatLog::logMessage(errorMessage, CatLog::Severity::Error, CatLog::Category::NetRequest, true);
                        }
                        catch(...)
                        {
                            // Mahdollinen ongelma on jo lokitettu, tällä pyritään estämään että poikkeus jonkun serverin käsittelyssä ei estä muiden toimintaa
                        }
                    }
                    if(bManager_->isKillSignalSet())
                        return;
                    for(const auto& serverKV : servers_)
                    {
                        auto previous = lastGoodTrees.find(serverKV.first);
                        if(previous != lastGoodTrees.end())
                            children.push_back(cloneCapabilityTree(*previous->second));
                    }
                    auto newTree = std::make_shared<CapabilityNode>(rootValue_, std::move(children));
                    {
                        std::scoped_lock lock(hashesMutex_, capabilityTreeMutex_);
                        hashesPtr_ = std::move(workingHashesPtr);
                        capabilityTree_ = std::move(newTree);
                    }
                    getCapabilitiesHaveBeenRetrieved_ = true;
                    if(foundAnyWmsServerData)
                    {
                        try
                        {
                            getCapabilitiesUpdateCallbackWrapper();
                        }
                        catch(const std::exception& e)
                        {
                            CatLog::logMessage(std::string("GetCapabilities update callback failed: ") + e.what(), CatLog::Severity::Error, CatLog::Category::NetRequest, true);
                        }
                        catch(...)
                        {
                            CatLog::logMessage("GetCapabilities update callback failed with an unknown exception", CatLog::Severity::Error, CatLog::Category::NetRequest, true);
                        }
                    }

                    using namespace std::literals;
                    bManager_->sleepInIntervals(intervalToPollGetCapabilities_, 500ms, "CapabilitiesHandler::BackgroundFetching");
                }
            }
        );
    }

    void CapabilitiesHandler::getCapabilitiesUpdateCallbackWrapper()
    {
        if(parameterSelectionUpdateCallback_)
        {
            // Aina kun on haettu uudet GetCapabilities haut, kutsutaan tätä
            parameterSelectionUpdateCallback_();
        }
    }

}
