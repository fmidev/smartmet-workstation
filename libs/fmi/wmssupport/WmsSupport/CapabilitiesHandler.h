#pragma once

#include "wmssupport/Setup.h"
#include "wmssupport/BitmapCache.h"
#include "wmssupport/QueryBuilder.h"
#include "wmssupport/CapabilityTree.h"
#include "wmssupport/CapabilityTreeParser.h"
#include "wmssupport/ChangedLayers.h"

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
        // Käytetään std::shared_ptr:ia, jotta olio voidaan antaa eri threadeissa käyttöön turvallisesti 
        // (shared_ptr kopio) vaikka kyseinen capabilityTree_ vaihdetaan lennossa päivitys working-threadeissa.
        std::shared_ptr<CapabilityTree> capabilityTree_;
        // capabilityTree_ olion käyttö pitää suojata thread turvallisella lukolla.
        mutable std::mutex capabilityTreeMutex_;

        std::unique_ptr<Web::Client> client_;
        std::function<void(long, const std::set<LayerInfo>&)> cacheDirtyCallback_;
        std::function<bool(long, const std::string&)> cacheHitCallback_;
        static std::function<void()> parameterSelectionUpdateCallback_;
        std::shared_ptr<cppback::BackgroundManager> bManager_;

        std::unordered_map<int, Wms::DynamicServerSetup> servers_;
        std::string proxyUrl_;
        std::chrono::seconds intervalToPollGetCapabilities_;
        int getCapabilitiesTimeoutInSeconds;
    public:
        CapabilitiesHandler(
            std::unique_ptr<Web::Client> client,
            const std::shared_ptr<cppback::BackgroundManager> &bManager,
            std::chrono::seconds intervalToPollGetCapabilities,
            const std::string &proxyUrl,
            const std::unordered_map<int, DynamicServerSetup> &servers,
            std::function<void(long, const std::set<LayerInfo>&)> cacheDirtyCallback,
            std::function<bool(long, const std::string&)> cacheHitCallback,
            int capabilitiesTimeoutInSeconds
            );

        void startFetchingCapabilitiesInBackground();
		const std::map<long, std::map<long, LayerInfo>>& peekHashes() const;
        std::shared_ptr<CapabilityTree> getCapabilityTree() const;
        void setCapabilityTree(std::shared_ptr<CapabilityTree> capabilityTree);
        bool isCapabilityTreeAvailable() const;
        static void setParameterSelectionUpdateCallback(std::function<void()>& parameterSelectionUpdateCallback);
        static void firstTimeUpdateCallbackWrapper();
    };
}