#include "NFmiVisualizationSpaceoutSettings.h"

NFmiVisualizationSpaceoutSettings::NFmiVisualizationSpaceoutSettings() = default;

bool NFmiVisualizationSpaceoutSettings::Init(const std::string & baseRegistryPath)
{
    if(initialized_)
        throw std::runtime_error("NFmiVisualizationSpaceoutSettings::Init: was al ready initialized.");

    initialized_ = true;
    baseRegistryPath_ = baseRegistryPath;
    sectionName_ = "\\VisualizationSettings";

    // HKEY_CURRENT_USER -keys
    HKEY usedKey = HKEY_CURRENT_USER;

    pixelToGridPointRatio_ = ::CreateRegValue<CachedRegDouble>(baseRegistryPath_, sectionName_, "\\pixelToGridPointRatio", usedKey, criticalPixelToGridPointRatioLimit_);
    pixelToGridPointRatio(*pixelToGridPointRatio_); // Tarkistetään rekisteristä luetut arvot
    globalVisualizationSpaceoutFactor_ = ::CreateRegValue<CachedRegDouble>(baseRegistryPath_, sectionName_, "\\globalVisualizationSpaceoutFactor", usedKey, 1.0);
    globalVisualizationSpaceoutFactor(*globalVisualizationSpaceoutFactor_); // Tarkistetään rekisteristä luetut arvot
    useGlobalVisualizationSpaceoutFactorOptimization_ = ::CreateRegValue<CachedRegBool>(baseRegistryPath_, sectionName_, "\\useGlobalVisualizationSpaceoutFactorOptimization", usedKey, false);
    spaceoutDataGatheringMethod_ = ::CreateRegValue<CachedRegInt>(baseRegistryPath_, sectionName_, "\\spaceoutDataGatheringMethod", usedKey, 0);
    spaceoutDataGatheringMethod(*spaceoutDataGatheringMethod_); // Tarkistetään rekisteristä luetut arvot

    return true;
}

double NFmiVisualizationSpaceoutSettings::pixelToGridPointRatio() const
{
    return *pixelToGridPointRatio_;
}

void NFmiVisualizationSpaceoutSettings::pixelToGridPointRatio(double newValue)
{
    if(newValue < minPixelToGridPointRatioValue_)
        newValue = minPixelToGridPointRatioValue_;
    if(newValue > maxPixelToGridPointRatioValue_)
        newValue = maxPixelToGridPointRatioValue_;
    *pixelToGridPointRatio_ = newValue;
}

double NFmiVisualizationSpaceoutSettings::globalVisualizationSpaceoutFactor() const
{
    return *globalVisualizationSpaceoutFactor_;
}

void NFmiVisualizationSpaceoutSettings::globalVisualizationSpaceoutFactor(double newValue)
{
    if(newValue < minVisualizationSpaceoutFactor_)
        newValue = minVisualizationSpaceoutFactor_;
    if(newValue > maxVisualizationSpaceoutFactor_)
        newValue = maxVisualizationSpaceoutFactor_;
    *globalVisualizationSpaceoutFactor_ = newValue;
}

bool NFmiVisualizationSpaceoutSettings::useGlobalVisualizationSpaceoutFactorOptimization() const
{
    return *useGlobalVisualizationSpaceoutFactorOptimization_;
}

void NFmiVisualizationSpaceoutSettings::useGlobalVisualizationSpaceoutFactorOptimization(bool newState)
{
    *useGlobalVisualizationSpaceoutFactorOptimization_ = newState;
}

int NFmiVisualizationSpaceoutSettings::spaceoutDataGatheringMethod() const
{
    return *spaceoutDataGatheringMethod_;
}

void NFmiVisualizationSpaceoutSettings::spaceoutDataGatheringMethod(int newValue)
{
    if(newValue < 0)
        newValue = 0;
    if(newValue > 1)
        newValue = 1;
    *spaceoutDataGatheringMethod_ = newValue;
}
