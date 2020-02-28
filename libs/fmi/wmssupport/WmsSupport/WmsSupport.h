#pragma once

#include "NFmiSatelliteImageCacheHelpers.h"
#include "wmssupport/LegendHandler.h"
#include "wmssupport/CapabilityTree.h"

#include <chrono>
#include <map>
#include <memory>
#include <string>
#include <unordered_map>

class NFmiArea;

namespace cppback
{
    class BackgroundManager;
}

namespace Wms
{
    class WmsClient;
    class CapabilitiesHandler;
    class BackgroundFetcher;
    class Setup;
    class WmsSupportState;
    class LegendHandler;
    class DynamicServerSetup;

    class WmsSupport
    {
        std::unique_ptr<WmsClient> backgroundClient_;
        std::unique_ptr<WmsClient> overlayClient_;
        std::map<int, std::unique_ptr<WmsClient>> dynamicClients_;
        std::unique_ptr<CapabilitiesHandler> capabilitiesHandler_;
        std::unique_ptr<BackgroundFetcher> backgroundFetcher_;
        std::unique_ptr<Setup> setup_;
        std::unique_ptr<WmsSupportState> state_;
        std::unique_ptr<LegendHandler> legendHandler_;

        std::shared_ptr<cppback::BackgroundManager> bManager_;
    public:
        WmsSupport();

        bool isConfigured() const;
        void kill();
        bool isDead(std::chrono::milliseconds wait) const;
        const CapabilityTree& peekCapabilityTree() const;
        std::string getFullLayerName(long producerId, long paramId) const;
        NFmiImageHolder getDynamicImage(long producerId, long paramId, const NFmiArea& area, const NFmiMetTime& time, int resolutionX, int resolutionY, int editorTimeStepInMinutes);
        std::vector<NFmiImageHolder> getLegends(int row, int col, int descTop);
        void registerDynamicLayer(int row, int col, int descTop, const NFmiDataIdent &dataIdent);
        void unregisterDynamicLayer(int row, int col, int descTop, const NFmiDataIdent &dataIdent);
        const LegendIdentSet& getRegisteredLayers(int row, int col, int descTop);
        NFmiImageHolder getBackground(const NFmiArea& area, int resolutionX, int resolutionY);
        NFmiImageHolder getOverlay(const NFmiArea& area, int resolutionX, int resolutionY);
        void nextBackground();
        void nextOverlay();
        void previousBackground();
        void previousOverlay();
        void initialSetUp(bool doVerboseLogging);
        void fillDynamicClients(const std::unordered_map<int, DynamicServerSetup> &serverSetups, const std::string& proxyUrl);
        std::unique_ptr<WmsClient> createClient(const DynamicServerSetup &setup, const std::string& proxyUrl);
    };
}

