#pragma once

#include "WmsSupportInterface.h"
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
    class LayerInfo;

    // Yhden karttanäytön (map-view 1-3), yhden kartta-alueen kokonaistila (suomi/skand/euro/maailma)
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

    class WmsSupport : public WmsSupportInterface
    {
        // Tähän talletetaan yhden karttanäytön kartta-alueiden 1-4 kokonaistilat
        using MapViewStaticMapClientState = std::map<unsigned int, StaticMapClientState>;
        // Tähän talletetaan kaikkien karttanäyttöjen (1-3) kaikkien kartta-alueiden tilat
        using TotalMapViewStaticMapClientState = std::map<unsigned int, MapViewStaticMapClientState>;

        // Tee jokaiselle map-view:lle (1-3) oma client ratkaisu ja tee oma jokaiselle kartta-alueelle (suomi, skand, euro, maailma) ,eli yht. 12 kpl.
        TotalMapViewStaticMapClientState totalMapViewStaticMapClientState_;

        std::map<int, std::unique_ptr<WmsClient>> dynamicClients_;
        std::unique_ptr<CapabilitiesHandler> capabilitiesHandler_;
        std::unique_ptr<BackgroundFetcher> backgroundFetcher_;
        std::unique_ptr<Setup> setup_;
        std::unique_ptr<LegendHandler> legendHandler_;

        std::shared_ptr<cppback::BackgroundManager> bManager_;
    public:
        WmsSupport();
        ~WmsSupport();

        // Interface's override methods
        bool isConfigured() const override;
        void initialSetUp(unsigned int mapViewCount, unsigned int mapAreaCount, bool doVerboseLogging) override;
        void nextBackground(unsigned int mapViewIndex, unsigned int mapAreaIndex) override;
        void nextOverlay(unsigned int mapViewIndex, unsigned int mapAreaIndex) override;
        void previousBackground(unsigned int mapViewIndex, unsigned int mapAreaIndex) override;
        void previousOverlay(unsigned int mapViewIndex, unsigned int mapAreaIndex) override;
        std::shared_ptr<Wms::CapabilityTree> getCapabilityTree() const override;
        bool isCapabilityTreeAvailable() const override;
        NFmiImageHolder getBackground(unsigned int mapViewIndex, unsigned int mapAreaIndex, const NFmiArea& area, int resolutionX, int resolutionY) override;
        NFmiImageHolder getOverlay(unsigned int mapViewIndex, unsigned int mapAreaIndex, const NFmiArea& area, int resolutionX, int resolutionY) override;
        const LegendIdentSet& getRegisteredLayers(int row, int col, int descTop) override;
        std::vector<NFmiImageHolder> getLegends(int row, int col, int descTop) override;
        void registerDynamicLayer(int row, int col, int descTop, const NFmiDataIdent &dataIdent) override;
        void unregisterDynamicLayer(int row, int col, int descTop, const NFmiDataIdent &dataIdent) override;
        std::string getFullLayerName(const NFmiDataIdent& dataIdent) const override;
        NFmiImageHolder getDynamicImage(const NFmiDataIdent& dataIdent, const NFmiArea& area, const NFmiMetTime& time, int resolutionX, int resolutionY, int editorTimeStepInMinutes) override;
        void kill() override;
        bool isDead(std::chrono::milliseconds wait) const override;
        StaticMapClientState& getStaticMapClientState(unsigned int mapViewIndex, unsigned int mapAreaIndex) override;
        const std::unique_ptr<Setup>& getSetup() const override;
        const LayerInfo* getHashedLayerInfo(const NFmiDataIdent& dataIdent) const override;

        void fillDynamicClients(const std::unordered_map<int, DynamicServerSetup> &serverSetups, const std::string& proxyUrl);
        std::unique_ptr<WmsClient> createClient(const DynamicServerSetup &setup, const std::string& proxyUrl);
        bool isTotalMapViewStaticMapClientStateAvailable() const override;
    private:
        StaticMapClientState createStaticMapClientState();
        MapViewStaticMapClientState createMapViewStaticMapClientState(unsigned int mapAreaCount);
    };
}

