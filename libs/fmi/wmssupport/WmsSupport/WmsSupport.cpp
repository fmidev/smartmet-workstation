#include "wmssupport/WmsSupport.h"
#include "wmssupport/Setup.h"
#include "wmssupport/WmsClient.h"
#include "wmssupport/GdiplusBitmapCache.h"
#include "wmssupport/SetupParser.h"
#include "wmssupport/WmsSupportState.h"
#include "wmssupport/BackgroundFetcher.h"
#include "wmssupport/CapabilitiesHandler.h"
#include "bitmaphandler/GdiplusBitmapParser.h"
#include <cppback/background-manager.h>
#include <webclient/CppRestClient.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/* // Katso WmsSupport::getDynamicImage metodin kommentoitujen koodien kommentti teksteistä, miksi tämä kohta on vielä kommentissa.
#include "NFmiLedLightStatus.h"
namespace
{
    std::string MakeLedChannelGetDynamicLayerReport(const std::string& hostStr, const std::string& layerStr)
    {
        std::string str = "Getting dynamic image from wms server:\n";
        str += hostStr + "(";
        str += layerStr + ")";
        return str;
    }
}
*/

namespace Wms
{
    StaticMapClientState::StaticMapClientState() = default;
    StaticMapClientState::StaticMapClientState(StaticMapClientState&&) = default;


    WmsSupport::WmsSupport()
    {
        bManager_ = std::make_shared<cppback::BackgroundManager>();
        legendHandler_ = std::make_unique<LegendHandler>();
    }

    WmsSupport::~WmsSupport() = default;

    bool WmsSupport::isConfigured() const
    {
        return setup_ && setup_->isConfigured;
    }

    void WmsSupport::kill()
    {
        for(auto& client : dynamicClients_)
        {
            client.second->kill();
        }
        bManager_->kill();
    }

    bool WmsSupport::isDead(std::chrono::milliseconds wait) const
    {
        return bManager_->isDead(wait);
    }

    std::shared_ptr<Wms::CapabilityTree> WmsSupport::getCapabilityTree() const
    {
        if(capabilitiesHandler_)
        {
            return capabilitiesHandler_->getCapabilityTree();
        }
        else
            return nullptr;
    }

    bool WmsSupport::isCapabilityTreeAvailable() const
    {
        if(capabilitiesHandler_)
        {
            return capabilitiesHandler_->isCapabilityTreeAvailable();
        }

        return false;
    }

    LayerInfo WmsSupport::getHashedLayerInfo(const NFmiDataIdent& dataIdent) const
    {
        if(!capabilitiesHandler_)
        {
            throw std::runtime_error("Error: calling WmsSupport::getHashedLayerInfo function before capabilitiesHandler system is initialized");
        }

        return capabilitiesHandler_->getHashes()->getLayerInfo(dataIdent);
    }

    std::string WmsSupport::getFullLayerName(const NFmiDataIdent& dataIdent) const
    {
        return getHashedLayerInfo(dataIdent).name;
    }

    NFmiImageHolder WmsSupport::getDynamicImage(const NFmiDataIdent& dataIdent, const NFmiArea& area, const NFmiMetTime& time, int resolutionX, int resolutionY, int editorTimeStepInMinutes)
    {
        std::string workingThreadName = "GetDynamicLayer";
        auto layerInfo = getHashedLayerInfo(dataIdent);
        if(layerInfo.hasTimeDimension && (time > layerInfo.endTime || time < layerInfo.startTime))
        {
            if(CatLog::logLevel() <= CatLog::Severity::Debug)
            {
                const NFmiString usedTimeFormat = "YYYY.MM.DD HH:mm";
                std::string noTimeDebugLevelMessage = __FUNCTION__;
                noTimeDebugLevelMessage += " requested time ";
                noTimeDebugLevelMessage += time.ToStr(usedTimeFormat);
                noTimeDebugLevelMessage += " was out of available server time range ";
                noTimeDebugLevelMessage += layerInfo.startTime.ToStr(usedTimeFormat);
                noTimeDebugLevelMessage += " - ";
                noTimeDebugLevelMessage += layerInfo.endTime.ToStr(usedTimeFormat);
                noTimeDebugLevelMessage += " for layer: ";
                noTimeDebugLevelMessage += layerInfo.name;
                CatLog::logMessage(noTimeDebugLevelMessage, CatLog::Severity::Debug, CatLog::Category::NetRequest);
            }
            return nullptr;
        }

        auto producerId = dataIdent.GetProducer()->GetIdent();
        auto qb = dynamicClients_[producerId]->getQB();
        auto query = qb
            .setLayers(layerInfo.name)
            .setWidth(resolutionX)
            .setHeight(resolutionY)
            .setCrsAndBbox(area)
            .setTime(time)
            .setRequest("GetMap")
            .setStyles(layerInfo.style.name)
            .build();
        /* // HUOM! Ei kannata laittaa kuvien haun yhteyteen led-channel viestitystä, koska pää-thread kutsuttu metodi odottaa aina
        *    loppuun asti että kuva ladataan.
        *    Tämä led-raportointi voidaan ottaa käyttöön jos/kun kuvan hakuja ei enää odotella katkeraan loppuun asti.
        NFmiLedLightStatusBlockReporter blockReporter(NFmiLedChannel::WmsData, workingThreadName, MakeLedChannelGetDynamicLayerReport(query.host, layerInfo->name));
        */
        backgroundFetcher_->fetch(*dynamicClients_[producerId], qb, time, editorTimeStepInMinutes);
        return dynamicClients_[producerId]->getImage(query);
    }

    std::vector<NFmiImageHolder> WmsSupport::getLegends(int row, int col, int descTop)
    {
        auto legends = std::vector<NFmiImageHolder>{};
        if(capabilitiesHandler_)
        {
            auto legendDataIdents = legendHandler_->getLegends(row, col, descTop);
            for(const auto& dataIdent : legendDataIdents)
            {
                auto layerInfo = capabilitiesHandler_->getHashes()->getLayerInfo(dataIdent);
                auto holder = dynamicClients_[dataIdent.GetProducer()->GetIdent()]->getLegend(layerInfo.style.legendDomain, layerInfo.style.legendRequest);
                if(holder)
                {
                    legends.push_back(holder);
                }
            }
        }
        return legends;
    }

    void WmsSupport::registerDynamicLayer(int row, int col, int descTop, const NFmiDataIdent &dataIdent)
    {
        legendHandler_->registerLayer(row, col, descTop, dataIdent);
    }

    void WmsSupport::unregisterDynamicLayer(int row, int col, int descTop, const NFmiDataIdent &dataIdent)
    {
        legendHandler_->unregisterLayer(row, col, descTop, dataIdent);
    }

    const LegendIdentSet& WmsSupport::getRegisteredLayers(int row, int col, int descTop)
    {
        return legendHandler_->getRegisteredLayers(row, col, descTop);
    }

    NFmiImageHolder WmsSupport::getBackground(unsigned int mapViewIndex, unsigned int mapAreaIndex, const NFmiArea& area, int resolutionX, int resolutionY)
    {
        auto &staticMapClientState = getStaticMapClientState(mapViewIndex, mapAreaIndex);
        auto& bkClient = staticMapClientState.backgroundClient_;
        auto& state = staticMapClientState.state_;
        auto query = bkClient->getUserUrlQB(state->getCurrentBackgroundIndex())
            .setLayers(setup_->background.parsedServers[state->getCurrentBackgroundIndex()].layerGroup)
            .setWidth(resolutionX)
            .setHeight(resolutionY)
            .setCrsAndBbox(area)
            .setRequest("GetMap")
            .setStyles("")
            .build();

        return bkClient->getImage(query);
    }

    NFmiImageHolder WmsSupport::getOverlay(unsigned int mapViewIndex, unsigned int mapAreaIndex, const NFmiArea& area, int resolutionX, int resolutionY)
    {
        auto& staticMapClientState = getStaticMapClientState(mapViewIndex, mapAreaIndex);
        auto& ovClient = staticMapClientState.overlayClient_;
        auto& state = staticMapClientState.state_;
        if(state->getCurrentOverlayIndex() == -1)
        {
            return nullptr;
        }
        auto query = ovClient->getUserUrlQB(state->getCurrentOverlayIndex())
            .setWidth(resolutionX)
            .setHeight(resolutionY)
            .setLayers(setup_->overlay.parsedServers[state->getCurrentOverlayIndex()].layerGroup)
            .setCrsAndBbox(area)
            .setRequest("GetMap")
            .setStyles("")
            .build();

        return ovClient->getImage(query);
    }

    void WmsSupport::nextBackground(unsigned int mapViewIndex, unsigned int mapAreaIndex)
    {
        auto& state = getStaticMapClientState(mapViewIndex, mapAreaIndex).state_;
        state->setBackgroundIndex(state->getCurrentBackgroundIndex() + 1);
    }

    void WmsSupport::nextOverlay(unsigned int mapViewIndex, unsigned int mapAreaIndex)
    {
        auto& state = getStaticMapClientState(mapViewIndex, mapAreaIndex).state_;
        state->setOverlayIndex(state->getCurrentOverlayIndex() + 1);
    }

    void WmsSupport::previousBackground(unsigned int mapViewIndex, unsigned int mapAreaIndex)
    {
        auto& state = getStaticMapClientState(mapViewIndex, mapAreaIndex).state_;
        state->setBackgroundIndex(state->getCurrentBackgroundIndex() - 1);
    }

    void WmsSupport::previousOverlay(unsigned int mapViewIndex, unsigned int mapAreaIndex)
    {
        auto& state = getStaticMapClientState(mapViewIndex, mapAreaIndex).state_;
        state->setOverlayIndex(state->getCurrentOverlayIndex() - 1);
    }

    void WmsSupport::initialSetUp(unsigned int mapViewCount, unsigned int mapAreaCount, bool doVerboseLogging)
    {
        try
        {
            setup_ = std::make_unique<Setup>(SetupParser::parse(doVerboseLogging));
            if(!setup_->isConfigured)
            {
                CatLog::logMessage(std::string("No meaningful Wms configurations were given, no WMS support available"), CatLog::Severity::Info, CatLog::Category::Configuration);
                return;
            }
        }
        catch(const std::exception & e)
        {
            CatLog::logMessage(std::string("Wms support initialization failed: ") + e.what(), CatLog::Severity::Error, CatLog::Category::Configuration, true);
            setup_ = std::make_unique<Setup>();
            setup_->isConfigured = false;
            return;
        }

        capabilitiesHandler_ = std::make_unique<CapabilitiesHandler>(
            std::make_unique<Web::CppRestClient>(bManager_, setup_->proxyUrl),
            bManager_,
            setup_->intervalToPollGetCapabilities,
            setup_->proxyUrl,
            setup_->dynamics,
            [&](long producerId, const std::set<LayerInfo>& layers)
            {
                for(const auto& layer : layers)
                {
                    dynamicClients_[producerId]->dirtyLayer(layer.name);
                }
            },
            [&](long producerId, const std::string& layerName)
            {
                return dynamicClients_[producerId]->isCached(layerName);
            },
            setup_->getCapabilitiesTimeoutInSeconds
            );

        for(auto index = 0u; index < mapViewCount; index++)
            totalMapViewStaticMapClientState_.emplace(index, createMapViewStaticMapClientState(mapAreaCount));

        fillDynamicClients(setup_->dynamics, setup_->proxyUrl);

        capabilitiesHandler_->startFetchingCapabilitiesInBackground();
    }

    StaticMapClientState WmsSupport::createStaticMapClientState()
    {
        StaticMapClientState mapClientState;
        mapClientState.state_ = std::make_unique<WmsSupportState>(*setup_);
        backgroundFetcher_ = std::make_unique<BackgroundFetcher>(bManager_, setup_->backgroundBackwardAmount, setup_->backgroundForwardAmount);

        mapClientState.backgroundClient_ = std::make_unique<WmsClient>(
            std::make_unique<GdiplusBitmapCache>(
                setup_->numberOfCaches, setup_->numberOfLayersPerCache
                ),
            std::make_unique<BitmapHandler::GdiplusBitmapParser>(),
            std::make_unique<Web::CppRestClient>(bManager_, setup_->proxyUrl),
            bManager_,
            std::make_unique<QueryBuilder>(),
            setup_->imageTimeoutInSeconds,
            setup_->legendTimeoutInSeconds
            );
        mapClientState.backgroundClient_->initializeUserUrl(setup_->background, setup_->proxyUrl);

        mapClientState.overlayClient_ = std::make_unique<WmsClient>(
            std::make_unique<GdiplusBitmapCache>(
                setup_->numberOfCaches, setup_->numberOfLayersPerCache
                ),
            std::make_unique<BitmapHandler::GdiplusBitmapParser>(),
            std::make_unique<Web::CppRestClient>(bManager_, setup_->proxyUrl),
            bManager_,
            std::make_unique<QueryBuilder>(),
            setup_->imageTimeoutInSeconds,
            setup_->legendTimeoutInSeconds
            );
        mapClientState.overlayClient_->initializeUserUrl(setup_->overlay, setup_->proxyUrl);


        return mapClientState;
    }

    WmsSupport::MapViewStaticMapClientState WmsSupport::createMapViewStaticMapClientState(unsigned int mapAreaCount)
    {
        MapViewStaticMapClientState mapViewStaticMapClientState;
        for(auto index = 0u; index < mapAreaCount; index++)
            mapViewStaticMapClientState.emplace(index, createStaticMapClientState());

        return std::move(mapViewStaticMapClientState);
    }

    void WmsSupport::fillDynamicClients(const std::unordered_map<int, DynamicServerSetup> &serverSetups, const std::string& proxyUrl)
    {
        for(const auto& setup : serverSetups)
        {
            try
            {
                dynamicClients_[setup.first] = createClient(setup.second, proxyUrl);
            }
            catch(std::exception & e)
            {
                std::string errorMessage = "Initializing WMS client failed: ";
                errorMessage += e.what();
                CatLog::logMessage(errorMessage, CatLog::Severity::Error, CatLog::Category::NetRequest, true);
            }
        }
    }

    std::unique_ptr<WmsClient> WmsSupport::createClient(const DynamicServerSetup &setup, const std::string& proxyUrl)
    {
        auto wmsClient = std::make_unique<WmsClient>(
            std::make_unique<GdiplusBitmapCache>(
                setup_->numberOfCaches, setup_->numberOfLayersPerCache
                ),
            std::make_unique<BitmapHandler::GdiplusBitmapParser>(),
            std::make_unique<Web::CppRestClient>(bManager_, setup_->proxyUrl),
            bManager_,
            std::make_unique<QueryBuilder>(),
            setup_->imageTimeoutInSeconds,
            setup_->legendTimeoutInSeconds
            );
        wmsClient->initializeDynamic(setup, proxyUrl);
        return std::move(wmsClient);
    }

    StaticMapClientState& WmsSupport::getStaticMapClientState(unsigned int mapViewIndex, unsigned int mapAreaIndex)
    {
        return totalMapViewStaticMapClientState_.at(mapViewIndex).at(mapAreaIndex);
    }

    const std::unique_ptr<Setup>& WmsSupport::getSetup() const
    {
        return setup_;
    }

    bool WmsSupport::isTotalMapViewStaticMapClientStateAvailable() const
    {
        return !totalMapViewStaticMapClientState_.empty();
    }
}

