#pragma once

#include "Setup.h"
#include "BitmapCache.h"
#include "QueryBuilder.h"
#include "CapabilityTree.h"
#include "CapabilityTreeParser.h"
#include "ChangedLayers.h"

#include <webclient/Client.h>

#include <cppext/split.h>
#include <cppback/background-manager.h>

#include <chrono>
#include <map>
#include <string>
#include <set>
#include <future>
#include <utility>
#include <mutex>
#include <list>
#include <functional>

namespace Wms
{
    class CapabilitiesHandler
    {
    private:
        Capability rootValue_ = { NFmiProducer{ 455234234, "NoProducer" }, kFmiLastParameter, "WmsData" };
        
        std::map<long, std::map<long, LayerInfo>> hashes_;
        ChangedLayers changedLayers_;
        std::unique_ptr<CapabilityTree> capabilityTree_;

        std::unique_ptr<Web::Client> client_;
        std::function<void(long, const std::set<LayerInfo>&)> cacheDirtyCallback_;
        std::function<bool(long, const std::string&)> cacheHitCallback_;
        std::shared_ptr<cppback::BackgroundManager> bManager_;

        std::unordered_map<int, Wms::DynamicServerSetup> servers_;
        std::string proxyUrl_;
        std::chrono::seconds intervalToPollGetCapabilities_;
    public:
        CapabilitiesHandler(
            std::unique_ptr<Web::Client> client,
            std::shared_ptr<cppback::BackgroundManager> bManager,
            std::chrono::seconds intervalToPollGetCapabilities,
            std::string proxyUrl,
            std::unordered_map<int, DynamicServerSetup> servers,
            std::function<void(long, const std::set<LayerInfo>&)> cacheDirtyCallback,
            std::function<bool(long, const std::string&)> cacheHitCallback
        );

        void startFetchingCapabilitiesInBackground();
		const std::map<long, std::map<long, LayerInfo>>& peekHashes() const;
        const CapabilityTree& peekCapabilityTree() const;
    };
}