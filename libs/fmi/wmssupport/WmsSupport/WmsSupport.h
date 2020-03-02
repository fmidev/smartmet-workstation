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

    // Yhden karttan�yt�n (map-view 1-3), yhden kartta-alueen kokonaistila (suomi/skand/euro/maailma)
    class StaticMapClientState
    {
    public:
        std::unique_ptr<WmsClient> backgroundClient_;
        std::unique_ptr<WmsClient> overlayClient_;
        std::unique_ptr<WmsSupportState> state_;

        StaticMapClientState();
        StaticMapClientState(const StaticMapClientState &) = delete;
        StaticMapClientState(StaticMapClientState &&state);
        StaticMapClientState& operator=(const StaticMapClientState &) = delete;
    };

    class WmsSupport
    {
        // T�h�n talletetaan yhden karttan�yt�n kartta-alueiden 1-4 kokonaistilat
        using MapViewStaticMapClientState = std::map<unsigned int, StaticMapClientState>;
        // T�h�n talletetaan kaikkien karttan�ytt�jen (1-3) kaikkien kartta-alueiden tilat
        using TotlalMapViewStaticMapClientState = std::map<unsigned int, MapViewStaticMapClientState>;

        // Tee jokaiselle map-view:lle (1-3) oma client ratkaisu ja tee oma jokaiselle kartta-alueelle (suomi, skand, euro, maailma) ,eli yht. 12 kpl.
        TotlalMapViewStaticMapClientState totalMapViewStaticMapClientState_;

        std::map<int, std::unique_ptr<WmsClient>> dynamicClients_;
        std::unique_ptr<CapabilitiesHandler> capabilitiesHandler_;
        std::unique_ptr<BackgroundFetcher> backgroundFetcher_;
        std::unique_ptr<Setup> setup_;
        std::unique_ptr<LegendHandler> legendHandler_;

        std::shared_ptr<cppback::BackgroundManager> bManager_;
    public:
        WmsSupport();
        ~WmsSupport();

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
        NFmiImageHolder getBackground(unsigned int mapViewIndex, unsigned int mapAreaIndex, const NFmiArea& area, int resolutionX, int resolutionY);
        NFmiImageHolder getOverlay(unsigned int mapViewIndex, unsigned int mapAreaIndex, const NFmiArea& area, int resolutionX, int resolutionY);
        void nextBackground(unsigned int mapViewIndex, unsigned int mapAreaIndex);
        void nextOverlay(unsigned int mapViewIndex, unsigned int mapAreaIndex);
        void previousBackground(unsigned int mapViewIndex, unsigned int mapAreaIndex);
        void previousOverlay(unsigned int mapViewIndex, unsigned int mapAreaIndex);
        void initialSetUp(unsigned int mapViewCount, unsigned int mapAreaCount, bool doVerboseLogging);
        void fillDynamicClients(const std::unordered_map<int, DynamicServerSetup> &serverSetups, const std::string& proxyUrl);
        std::unique_ptr<WmsClient> createClient(const DynamicServerSetup &setup, const std::string& proxyUrl);
        StaticMapClientState& getStaticMapClientState(unsigned int mapViewIndex, unsigned int mapAreaIndex);
    private:
        StaticMapClientState createStaticMapClientState();
        MapViewStaticMapClientState createMapViewStaticMapClientState(unsigned int mapAreaCount);
    };
}

