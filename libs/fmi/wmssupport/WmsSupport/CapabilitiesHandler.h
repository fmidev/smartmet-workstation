#pragma once

#include "wmssupport/Setup.h"
#include "wmssupport/BitmapCache.h"
#include "wmssupport/QueryBuilder.h"
#include "wmssupport/CapabilityTree.h"
#include "wmssupport/ChangedLayers.h"
#include "wmssupport/CapabilitiesHandlerHashes.h"

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
        
        std::shared_ptr<CapabilitiesHandlerHashes> hashesPtr_;
        // hashes_ olion käyttö pitää suojata thread turvallisella lukolla.
        mutable std::mutex hashesMutex_;
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
        int getCapabilitiesTimeoutInSeconds_;
        // Jos tämä muuttuu true:ksi, se tarkoittaa että startFetchingCapabilitiesInBackground
        // metodi on mennyt kerran läpi, löytyi servereiltä mitään järkevää tai ei.
        bool getCapabilitiesHaveBeenRetrieved_ = false;
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
        std::shared_ptr<CapabilitiesHandlerHashes> getHashes() const;
        void setHashes(std::shared_ptr<CapabilitiesHandlerHashes> hashesPtr);
        std::shared_ptr<CapabilityTree> getCapabilityTree() const;
        void setCapabilityTree(std::shared_ptr<CapabilityTree> capabilityTree);
        bool isCapabilityTreeAvailable() const;
        bool getCapabilitiesHaveBeenRetrieved() const;
        static void setParameterSelectionUpdateCallback(std::function<void()>& parameterSelectionUpdateCallback);
        static void firstTimeUpdateCallbackWrapper();
    };
}