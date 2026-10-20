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
    std::mutex CapabilitiesHandler::callbackMutex_;

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
        std::lock_guard<std::mutex> lock(callbackMutex_);
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
        std::lock_guard<std::mutex> lock(workerMutex_);
        if(bManager_->isKillSignalSet())
            return;
        if(workerFuture_.valid())
        {
            if(workerFuture_.wait_for(std::chrono::seconds(0)) != std::future_status::ready)
                return;
            try { workerFuture_.get(); }
            catch(const std::exception& e)
            {
                CatLog::logMessage(std::string("GetCapabilities worker stopped: ") + e.what(), CatLog::Severity::Error, CatLog::Category::NetRequest, true);
            }
            catch(...)
            {
                CatLog::logMessage("GetCapabilities worker stopped with an unknown exception", CatLog::Severity::Error, CatLog::Category::NetRequest, true);
            }
        }
        workerFuture_ = bManager_->addTask([this]()
            {
                std::string workingThreadName = "GetCapabilityTree";
                // Owned only by this worker; published trees are independent copies.
                std::map<int, std::unique_ptr<CapabilityTree>> lastGoodTrees;
                auto workingHashesPtr = std::make_shared<CapabilitiesHandlerHashes>(*getHashes());
                while(!bManager_->isKillSignalSet())
                {
                    try
                    {
                        auto children = std::vector<std::unique_ptr<CapabilityTree>>{};
                        bool foundAnyWmsServerData = false;

                        for(auto& serverKV : servers_)
                        {
                            if(bManager_->isKillSignalSet())
                                return;
                            const auto& server = serverKV.second;
                            try
                            {
                                NFmiLedLightStatusBlockReporter blockReporter(NFmiLedChannel::WmsData, workingThreadName, MakeLedChannelGetCapabilityTreeReport(server.generic.host));
                                auto query = QueryBuilder{}.setScheme(server.generic.scheme)
                                    .setHost(server.generic.host)
                                    .setPath(server.generic.path)
                                    .setService("WMS")
                                    .setVersion(server.version)
                                    .setRequest("GetCapabilities")
                                    .setToken(server.generic.token)
                                    .build();
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
                            catch(const std::exception& e)
                            {
                                if(bManager_->isKillSignalSet())
                                    return;
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
                                if(bManager_->isKillSignalSet())
                                    return;
                                CatLog::logMessage("Unknown GetCapabilities failure for " + server.generic.host + "; keeping previous capabilities if available.", CatLog::Severity::Error, CatLog::Category::NetRequest, true);
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
                        auto publishedHashes = std::make_shared<CapabilitiesHandlerHashes>(*workingHashesPtr);
                        {
                            std::scoped_lock lock(hashesMutex_, capabilityTreeMutex_);
                            hashesPtr_ = std::move(publishedHashes);
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

                    }
                    catch(const std::exception& e)
                    {
                        if(bManager_->isKillSignalSet())
                            return;
                        CatLog::logMessage(std::string("GetCapabilities refresh cycle failed; retrying after polling interval: ") + e.what(), CatLog::Severity::Error, CatLog::Category::NetRequest, true);
                    }
                    catch(...)
                    {
                        if(bManager_->isKillSignalSet())
                            return;
                        CatLog::logMessage("GetCapabilities refresh cycle failed with an unknown exception; retrying after polling interval", CatLog::Severity::Error, CatLog::Category::NetRequest, true);
                    }
                    using namespace std::literals;
                    try
                    {
                        bManager_->sleepInIntervals(intervalToPollGetCapabilities_, 500ms, "CapabilitiesHandler::BackgroundFetching");
                    }
                    catch(const cppback::TaskStoppedByKillSignal&)
                    {
                        return;
                    }
                }
            }
        );
    }

    void CapabilitiesHandler::getCapabilitiesUpdateCallbackWrapper()
    {
        std::function<void()> callback;
        {
            std::lock_guard<std::mutex> lock(callbackMutex_);
            callback = parameterSelectionUpdateCallback_;
        }
        if(callback)
        {
            // Aina kun on haettu uudet GetCapabilities haut, kutsutaan tätä
            callback();
        }
    }

}
