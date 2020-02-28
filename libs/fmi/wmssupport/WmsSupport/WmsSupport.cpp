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


namespace Wms
{
    WmsSupport::WmsSupport()
    {
        bManager_ = std::make_shared<cppback::BackgroundManager>();
        legendHandler_ = std::make_unique<LegendHandler>();
    }

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

    const CapabilityTree& WmsSupport::peekCapabilityTree() const
    {
        return capabilitiesHandler_->peekCapabilityTree();
    }

    std::string WmsSupport::getFullLayerName(long producerId, long paramId) const
    {
        return capabilitiesHandler_->peekHashes().at(producerId).at(paramId).name;
    }

    NFmiImageHolder WmsSupport::getDynamicImage(long producerId, long paramId, const NFmiArea& area, const NFmiMetTime& time, int resolutionX, int resolutionY, int editorTimeStepInMinutes)
    {
        auto layerInfo = capabilitiesHandler_->peekHashes().at(producerId).at(paramId);

        if(time > layerInfo.endTime || time < layerInfo.startTime)
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

        backgroundFetcher_->fetch(*dynamicClients_[producerId], qb, time, editorTimeStepInMinutes);
        return dynamicClients_[producerId]->getImage(query);
    }

    std::vector<NFmiImageHolder> WmsSupport::getLegends(int row, int col, int descTop)
    {
        auto legends = std::vector<NFmiImageHolder>{};
        auto legendDataIdents = legendHandler_->getLegends(row, col, descTop);
        for(const auto& dataIdent : legendDataIdents)
        {
            auto producerId = dataIdent.GetProducer()->GetIdent();
            auto paramId = dataIdent.GetParamIdent();
            const auto& layerInfo = capabilitiesHandler_->peekHashes().at(producerId).at(paramId);

            auto holder = dynamicClients_[producerId]->getLegend(layerInfo.style.legendDomain, layerInfo.style.legendRequest);
            if(holder)
            {
                legends.push_back(holder);
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

    NFmiImageHolder WmsSupport::getBackground(const NFmiArea& area, int resolutionX, int resolutionY)
    {
        auto query = backgroundClient_->getUserUrlQB(state_->getCurrentBackgroundIndex())
            .setLayers(setup_->background.parsedServers[state_->getCurrentBackgroundIndex()].layerGroup)
            .setWidth(resolutionX)
            .setHeight(resolutionY)
            .setCrsAndBbox(area)
            .setRequest("GetMap")
            .setStyles("")
            .build();

        return backgroundClient_->getImage(query);
    }

    NFmiImageHolder WmsSupport::getOverlay(const NFmiArea& area, int resolutionX, int resolutionY)
    {
        if(state_->getCurrentOverlayIndex() == -1)
        {
            return nullptr;
        }
        auto query = overlayClient_->getUserUrlQB(state_->getCurrentOverlayIndex())
            .setWidth(resolutionX)
            .setHeight(resolutionY)
            .setLayers(setup_->overlay.parsedServers[state_->getCurrentOverlayIndex()].layerGroup)
            .setCrsAndBbox(area)
            .setRequest("GetMap")
            .setStyles("")
            .build();

        return overlayClient_->getImage(query);
    }

    void WmsSupport::nextBackground()
    {
        state_->setBackgroundIndex(state_->getCurrentBackgroundIndex() + 1);
    }

    void WmsSupport::nextOverlay()
    {
        state_->setOverlayIndex(state_->getCurrentOverlayIndex() + 1);
    }

    void WmsSupport::previousBackground()
    {
        state_->setBackgroundIndex(state_->getCurrentBackgroundIndex() - 1);
    }

    void WmsSupport::previousOverlay()
    {
        state_->setOverlayIndex(state_->getCurrentOverlayIndex() - 1);
    }

    void WmsSupport::initialSetUp(bool doVerboseLogging)
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
            }
            );

        state_ = std::make_unique<WmsSupportState>(*setup_);
        backgroundFetcher_ = std::make_unique<BackgroundFetcher>(bManager_, setup_->backgroundBackwardAmount, setup_->backgroundForwardAmount);

        backgroundClient_ = std::make_unique<WmsClient>(
            std::make_unique<GdiplusBitmapCache>(
                setup_->numberOfCaches, setup_->numberOfLayersPerCache
                ),
            std::make_unique<BitmapHandler::GdiplusBitmapParser>(),
            std::make_unique<Web::CppRestClient>(bManager_, setup_->proxyUrl),
            bManager_,
            std::make_unique<QueryBuilder>()
            );
        backgroundClient_->initializeUserUrl(setup_->background, setup_->proxyUrl);

        overlayClient_ = std::make_unique<WmsClient>(
            std::make_unique<GdiplusBitmapCache>(
                setup_->numberOfCaches, setup_->numberOfLayersPerCache
                ),
            std::make_unique<BitmapHandler::GdiplusBitmapParser>(),
            std::make_unique<Web::CppRestClient>(bManager_, setup_->proxyUrl),
            bManager_,
            std::make_unique<QueryBuilder>()
            );
        overlayClient_->initializeUserUrl(setup_->overlay, setup_->proxyUrl);

        fillDynamicClients(setup_->dynamics, setup_->proxyUrl);

        capabilitiesHandler_->startFetchingCapabilitiesInBackground();
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
            std::make_unique<QueryBuilder>()
            );
        wmsClient->initializeDynamic(setup, proxyUrl);
        return std::move(wmsClient);
    }
}

