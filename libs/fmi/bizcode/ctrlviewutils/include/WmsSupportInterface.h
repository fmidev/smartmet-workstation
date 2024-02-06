#pragma once

#include "WmsSupportDefines.h"
#include "NFmiSatelliteImageCacheHelpers.h"
#include <functional>
#include <chrono>

class NFmiArea;

namespace Wms
{
    class StaticMapClientState;
    class Setup;
    class LayerInfo;
}

// Interface that is meant to be used instead actual Wms::WmsSupport implementation class.
class WmsSupportInterface
{
public:
    using GetWmsSupportInterfaceImplementationCallBackType = std::function<WmsSupportInterface*(void)>;
    // Tämä pitää asettaa johonkin konkreettiseen funktioon, jotta käyttäjä koodi saa käyttöönsä halutun interface toteutuksen
    static GetWmsSupportInterfaceImplementationCallBackType GetWmsSupportInterfaceImplementation;

    virtual ~WmsSupportInterface();

    virtual bool isConfigured() const = 0;
    virtual void initialSetUp(unsigned int mapViewCount, unsigned int mapAreaCount, bool doVerboseLogging) = 0;
    virtual void nextBackground(unsigned int mapViewIndex, unsigned int mapAreaIndex) = 0;
    virtual void nextOverlay(unsigned int mapViewIndex, unsigned int mapAreaIndex) = 0;
    virtual void previousBackground(unsigned int mapViewIndex, unsigned int mapAreaIndex) = 0;
    virtual void previousOverlay(unsigned int mapViewIndex, unsigned int mapAreaIndex) = 0;
    virtual NFmiImageHolder getBackground(unsigned int mapViewIndex, unsigned int mapAreaIndex, const NFmiArea& area, int resolutionX, int resolutionY) = 0;
    virtual NFmiImageHolder getOverlay(unsigned int mapViewIndex, unsigned int mapAreaIndex, const NFmiArea& area, int resolutionX, int resolutionY) = 0;
    virtual const Wms::LegendIdentSet& getRegisteredLayers(int row, int col, int descTop) = 0;
    virtual std::vector<NFmiImageHolder> getLegends(int row, int col, int descTop) = 0;
    virtual void registerDynamicLayer(int row, int col, int descTop, const NFmiDataIdent& dataIdent) = 0;
    virtual void unregisterDynamicLayer(int row, int col, int descTop, const NFmiDataIdent& dataIdent) = 0;
    virtual std::string getFullLayerName(const NFmiDataIdent& dataIdent) const = 0;
    virtual NFmiImageHolder getDynamicImage(const NFmiDataIdent &dataIdent, const NFmiArea& area, const NFmiMetTime& time, int resolutionX, int resolutionY, int editorTimeStepInMinutes) = 0;
    virtual void kill() = 0;
    virtual bool isDead(std::chrono::milliseconds wait) const = 0;
    virtual Wms::StaticMapClientState& getStaticMapClientState(unsigned int mapViewIndex, unsigned int mapAreaIndex) = 0;
    virtual const std::unique_ptr<Wms::Setup>& getSetup() const = 0;
    virtual Wms::LayerInfo getHashedLayerInfo(const NFmiDataIdent& dataIdent) const = 0;
    virtual bool isTotalMapViewStaticMapClientStateAvailable() const = 0;

#ifndef DISABLE_CPPRESTSDK
    virtual std::shared_ptr<Wms::CapabilityTree> getCapabilityTree() const = 0;
    // Kun varsinaisia kyselyjä on tarkoitus tehdä, kannattaa varmistaa tällä että onko systeemi jo käytössä
    virtual bool isCapabilityTreeAvailable() const = 0;
#endif // DISABLE_CPPRESTSDK

};
