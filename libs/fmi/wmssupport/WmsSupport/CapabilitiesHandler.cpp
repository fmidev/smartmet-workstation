#include "CapabilitiesHandler.h"
#include "WmsQuery.h"
#include "SetupParser.h"

#include <webclient/Client.h>

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
        std::string fetchCapabilitiesXml(const Web::Client& client, const WmsQuery& query)
        {
            try
            {
                auto httpResponseFut = client.queryFor(toBaseUri(query), toRequest(query));
                httpResponseFut.wait();
                return httpResponseFut.get();
            }
            catch(const std::exception&)
            {
                return "";
            }
        }
    }

    CapabilitiesHandler::CapabilitiesHandler(
        std::unique_ptr<Web::Client> client,
        std::shared_ptr<cppback::BackgroundManager> bManager,
        std::chrono::seconds intervalToPollGetCapabilities,
        std::string proxyUrl,
        std::unordered_map<int, DynamicServerSetup> servers,
        std::function<void(long, const std::set<LayerInfo>&)> cacheDirtyCallback,
        std::function<bool(long, const std::string&)> cacheHitCallback
    )
        : client_(std::move(client))
        , cacheDirtyCallback_(cacheDirtyCallback)
        , bManager_{ bManager }
        , proxyUrl_{ proxyUrl }
        , servers_{ servers }
        , intervalToPollGetCapabilities_{ intervalToPollGetCapabilities }
        , cacheHitCallback_{ cacheHitCallback }
    {}

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

    void CapabilitiesHandler::startFetchingCapabilitiesInBackground()
    {
        bManager_->addTask([&]()
        {
            while(true)
            {
                auto children = std::vector<std::unique_ptr<CapabilityTree>>{};

                for(const auto& serverKV : servers_)
                {
                    auto server = serverKV.second;
                    auto query = QueryBuilder{}.setScheme(server.generic.scheme)
                        .setHost(server.generic.host)
                        .setPath(server.generic.path)
                        .setService("WMS")
                        .setVersion(server.version)
                        .setFormat("image/png")
                        .setRequest("GetCapabilities")
                        .setStyles("")
                        .setTransparency(true)
                        .setWidth(0)
                        .setHeight(0)
                        .setToken(server.generic.token)
                        .build();

                    auto capabilityTreeParser = CapabilityTreeParser{ server.producer, server.delimiter, cacheHitCallback_ };
                    auto capabilities = parseXmlToPropertyTree(fetchCapabilitiesXml(*client_, query));
                    changedLayers_.changedLayers.clear();
                    children.push_back(capabilityTreeParser.parse(capabilities.get_child("WMS_Capabilities.Capability.Layer"), hashes_, changedLayers_));
                    if(!changedLayers_.changedLayers.empty())
                    {
                        cacheDirtyCallback_(server.producer.GetIdent(), changedLayers_.changedLayers);
                    }
                }
                capabilityTree_ = std::make_unique<CapabilityNode>(rootValue_, std::move(children));

                using namespace std::literals;
                bManager_->sleepInIntervals(intervalToPollGetCapabilities_, 500ms, "CapabilitiesHandler::BackgroundFetching");
            }
        });
    }
}
