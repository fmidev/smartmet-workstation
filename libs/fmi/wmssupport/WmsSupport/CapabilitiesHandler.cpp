#include "wmssupport/CapabilitiesHandler.h"
#include "wmssupport/WmsQuery.h"
#include "wmssupport/SetupParser.h"
#include "xmlliteutils/XmlHelperFunctions.h"
#include "../../q2clientlib/include/NFmiQ2Client.h"
#include "NFmiFileSystem.h"
#include "NFmiLedLightStatus.h"

#include <webclient/Client.h>

#include <catlog/catlog.h>
#include <cppback/background-manager.h>
#include <boost/property_tree/xml_parser.hpp>

namespace Wms
{
    namespace
    {
        boost::property_tree::ptree parseXmlToPropertyTree(const std::string& xml)
        {
            auto pTree = boost::property_tree::ptree{};
            boost::property_tree::read_xml(std::stringstream{ xml }, pTree);
            return pTree;
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
                httpResponseFut.wait();
                responseStr = httpResponseFut.get();

                // Lis�tty testauskoodeja sek� olemassa olevan getCapabilities responsin tiedostotalletukseen ett� sielt� lukuun. Pid� normaalisti kommenteissa!!
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
                return "";
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
        : client_(std::move(client))
        , cacheDirtyCallback_(cacheDirtyCallback)
        , bManager_{ bManager }
        , proxyUrl_{ proxyUrl }
        , servers_{ servers }
        , intervalToPollGetCapabilities_{ intervalToPollGetCapabilities }
        , getCapabilitiesTimeoutInSeconds{capabilitiesTimeoutInSeconds}
        , cacheHitCallback_{ cacheHitCallback }
    {}

    void CapabilitiesHandler::setParameterSelectionUpdateCallback(std::function<void()>& parameterSelectionUpdateCallback)
    {
        parameterSelectionUpdateCallback_ = parameterSelectionUpdateCallback;
    }

    const std::map<long, std::map<long, LayerInfo>>& CapabilitiesHandler::peekHashes() const
    {
        return hashes_;
    }

    const CapabilityTree& CapabilitiesHandler::peekCapabilityTree() const
    {
        if(!capabilityTree_)
        {
            throw std::runtime_error("CapabilitiesHandler: peekCapabilityTree called before capabilitiesTree was initialized.");
        }
        return *capabilityTree_;
    }

    bool CapabilitiesHandler::isCapabilityTreeAvailable() const
    {
        return capabilityTree_ != nullptr;
    }

    void CapabilitiesHandler::startFetchingCapabilitiesInBackground()
    {
        bManager_->addTask([&]()
            {
                std::string workingThreadName = "GetCapabilityTree";
                while(true)
                {
                    auto children = std::vector<std::unique_ptr<CapabilityTree>>{};
                    bool foundAnyWmsServerData = false;

                    for(auto& serverKV : servers_)
                    {
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
                            auto capabilityTreeParser = CapabilityTreeParser{ server.producer, server.delimiter, cacheHitCallback_ };
                            auto xml = fetchCapabilitiesXml(*client_, query, server.logFetchCapabilitiesRequest, server.doVerboseLogging, getCapabilitiesTimeoutInSeconds);
                            changedLayers_.changedLayers.clear();
                            children.push_back(capabilityTreeParser.parseXmlGeneral(xml, hashes_, changedLayers_));
                            foundAnyWmsServerData = true;
                            if(!changedLayers_.changedLayers.empty())
                            {
                                cacheDirtyCallback_(server.producer.GetIdent(), changedLayers_.changedLayers);
                            }
                        }
                        catch(std::exception& e)
                        {
                            std::string errorMessage = "Error with dynamic Wms server '";
                            errorMessage += server.generic.host;
                            errorMessage += "', while parsing getCapabilities response: ";
                            errorMessage += e.what();
                            CatLog::logMessage(errorMessage, CatLog::Severity::Error, CatLog::Category::NetRequest, true);
                        }
                        catch(...)
                        {
                            // Mahdollinen ongelma on jo lokitettu, t�ll� pyrit��n est�m��n ett� poikkeus jonkun serverin k�sittelyss� ei est� muiden toimintaa
                        }
                    }
                    capabilityTree_ = std::make_unique<CapabilityNode>(rootValue_, std::move(children));
                    if(foundAnyWmsServerData)
                    {
                        firstTimeUpdateCallbackWrapper();
                    }

                    using namespace std::literals;
                    bManager_->sleepInIntervals(intervalToPollGetCapabilities_, 500ms, "CapabilitiesHandler::BackgroundFetching");
                }
            }
        );
    }

    void CapabilitiesHandler::firstTimeUpdateCallbackWrapper()
    {
        static bool firstTime = true;

        if(firstTime && parameterSelectionUpdateCallback_ != nullptr)
        {
            firstTime = false;
            CatLog::logMessage("CapabilitiesHandler::firstTimeUpdateCallbackWrapper activated", CatLog::Severity::Debug, CatLog::Category::NetRequest);
            parameterSelectionUpdateCallback_();
        }
    }

}
