#include "NFmiVisualizationSpaceoutSettings.h"
#include "catlog/catlog.h"
#include "NFmiValueString.h"
#include "NFmiFastQueryInfo.h"
#include "NFmiQueryDataUtil.h"
#include "NFmiArea.h"
#include "NFmiWorldXyBoundingBoxCalculator.h"

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
    baseSpaceoutGridSize_ = ::CreateRegValue<CachedRegInt>(baseRegistryPath_, sectionName_, "\\baseSpaceoutGridSize", usedKey, 100);
    baseSpaceoutGridSize(*baseSpaceoutGridSize_); // Tarkistetään rekisteristä luetut arvot
    useGlobalVisualizationSpaceoutFactorOptimization_ = ::CreateRegValue<CachedRegBool>(baseRegistryPath_, sectionName_, "\\useGlobalVisualizationSpaceoutFactorOptimization", usedKey, false);
    spaceoutDataGatheringMethod_ = ::CreateRegValue<CachedRegInt>(baseRegistryPath_, sectionName_, "\\spaceoutDataGatheringMethod", usedKey, 0);
    spaceoutDataGatheringMethod(*spaceoutDataGatheringMethod_); // Tarkistetään rekisteristä luetut arvot

    return true;
}

template<typename RegistryValue, typename CheckFunction>
static bool IsRegistryValueChangedInUpdate(RegistryValue & registryValue, CheckFunction & checkFunction)
{
    // 1. Otetaan vanha arvo talteen
    auto originalValue = *registryValue;
    // 2. Tehdään uuden arvon asteus tarkastelufunktion avulla
    checkFunction();
    // 3. Jos arvo muuttui, verrattuna originaaliin (ei annettuun arvoon), pitää päivitys tehdä
    return *registryValue != originalValue;
}

// Metodi saa Visualization settings -dialogilta kaikki nykyiset asetukset.
// Jos mitään asetus oikeasti muuttuu niin että karttanäyttöjä pitää päivittää ja tehdä dirty asetuksia, palautetaan true, muuten false.
bool NFmiVisualizationSpaceoutSettings::updateFromDialog(double newPixelToGridPointRatio, bool newUsePixelToGridPointRatioSafetyFeature, int newBaseSpaceoutGridSize, bool newUseGlobalVisualizationSpaceoutFactorOptimization, int newSpaceoutDataGatheringMethod)
{
    bool needsToUpdateViews = false;
    if(::IsRegistryValueChangedInUpdate(pixelToGridPointRatio_, [=](){this->pixelToGridPointRatio(newPixelToGridPointRatio); }))
    {
        needsToUpdateViews = true;
    }
    if(usePixelToGridPointRatioSafetyFeature() != newUsePixelToGridPointRatioSafetyFeature)
    {
        usePixelToGridPointRatioSafetyFeature(newUsePixelToGridPointRatioSafetyFeature);
        needsToUpdateViews = true;
    }
    if(::IsRegistryValueChangedInUpdate(baseSpaceoutGridSize_, [=]() {this->baseSpaceoutGridSize(newBaseSpaceoutGridSize); }))
    {
        needsToUpdateViews = true;
    }
    if(::IsRegistryValueChangedInUpdate(useGlobalVisualizationSpaceoutFactorOptimization_, [=]() {this->useGlobalVisualizationSpaceoutFactorOptimization(newUseGlobalVisualizationSpaceoutFactorOptimization); }))
    {
        needsToUpdateViews = true;
    }

    return needsToUpdateViews;
}

void NFmiVisualizationSpaceoutSettings::doViewUpdateWarningLogsIfNeeded()
{
    // Tehdään varoitus lokiviestejä tarpeen mukaan.
    // Tehdään maksimissaan yksi lokitus koskien PixelToGridPointRatio asetuksia (tehdään niitä 'pahemmuus' järjestyksessä).
    if(usePixelToGridPointRatioSafetyFeature_ == false)
    {
        std::string warningMessage = "Pixel-To-Grid-Point-Ratio safety feature is set off, isoline visualizations may crash SmartMet, set it back on from Edit - Visualization settings...";
        CatLog::logMessage(warningMessage, CatLog::Severity::Warning, CatLog::Category::Visualization, true);
    }
    else if(*pixelToGridPointRatio_ < criticalPixelToGridPointRatioLimit_)
    {
        std::string warningMessage = "Pixel-To-Grid-Point-Ratio safety feature is set under critical limit ";
        warningMessage += NFmiValueString::GetStringWithMaxDecimalsSmartWay(criticalPixelToGridPointRatioLimit_, 1);
        warningMessage += ", isoline visualizations may crash SmartMet, set it to or over critical limit from Edit - Visualization settings...";
        CatLog::logMessage(warningMessage, CatLog::Severity::Warning, CatLog::Category::Visualization, true);
    }

    if(*useGlobalVisualizationSpaceoutFactorOptimization_)
    {
        std::string logMessage = "Global-Visualization-Spaceout-Grid-Size optimization is set on, with base size ";
        logMessage += NFmiValueString::GetStringWithMaxDecimalsSmartWay(*baseSpaceoutGridSize_, 1);
        logMessage += ", map visualizations are faster, but lose information. If you wan't to see full accuracy of data, set it off from Edit - Visualization settings...";
        CatLog::logMessage(logMessage, CatLog::Severity::Info, CatLog::Category::Visualization);
    }
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

int NFmiVisualizationSpaceoutSettings::baseSpaceoutGridSize() const
{
    return *baseSpaceoutGridSize_;
}

void NFmiVisualizationSpaceoutSettings::baseSpaceoutGridSize(int newValue)
{
    if(newValue < minBaseSpaceoutGridSize_)
        newValue = minBaseSpaceoutGridSize_;
    if(newValue > maxBaseSpaceoutGridSize_)
        newValue = maxBaseSpaceoutGridSize_;
    *baseSpaceoutGridSize_ = newValue;
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

void NFmiVisualizationSpaceoutSettings::spaceoutDataGatheringMethod(int /* newValue */ )
{
    //if(newValue < 0)
    //    newValue = 0;
    //if(newValue > 1)
    //    newValue = 1;
    *spaceoutDataGatheringMethod_ = 0; // newValue;
}

static NFmiPoint calcApproximationDataGridSizeOverMapBoundingBox(NFmiFastQueryInfo& fastInfo, const NFmiRect& dataOverMapWorldXyBoundingBox)
{
    auto fastInfoWorldRect = fastInfo.Area()->WorldRect();
    auto gridCellSizeInMetersX = fastInfoWorldRect.Width() / (fastInfo.GridXNumber() - 1);
    auto gridCellSizeInMetersY = fastInfoWorldRect.Height() / (fastInfo.GridYNumber() - 1);
    double gridSizeX = dataOverMapWorldXyBoundingBox.Width() / gridCellSizeInMetersX;
    double gridSizeY = dataOverMapWorldXyBoundingBox.Height() / gridCellSizeInMetersY;
    return NFmiPoint(gridSizeX, gridSizeY);
}

static NFmiPoint calcBaseGridSizeOverMapBoundingBox(NFmiPoint& baseGridSize, const NFmiRect& mapWorldXyRect, const NFmiRect& dataOverMapWorldXyBoundingBox)
{
    auto widthFactor = dataOverMapWorldXyBoundingBox.Width() / mapWorldXyRect.Width();
    auto heigthFactor = dataOverMapWorldXyBoundingBox.Height() / mapWorldXyRect.Height();
    return NFmiPoint(baseGridSize.X() * widthFactor, baseGridSize.Y() * heigthFactor);
}

static bool shouldOptimizedGridBeUsed(const NFmiPoint& baseGridSizeOverMapBoundingBox, const NFmiPoint& approximationDataGridSizeOverMapBoundingBox)
{
    auto widthRatio = std::round(baseGridSizeOverMapBoundingBox.X()) / std::round(approximationDataGridSizeOverMapBoundingBox.X());
    auto heigthRatio = std::round(baseGridSizeOverMapBoundingBox.Y()) / std::round(approximationDataGridSizeOverMapBoundingBox.Y());
    if(((widthRatio + heigthRatio)/2.) < 0.9)
        return true;
    else
        return false;
}

// Huom! On jo tarkastettu (NFmiIsoLineView::FillGridRelatedData metodissa), että näkyykö data kartta-alueella, joten sitä ei tavitse tarkastella enää.
bool NFmiVisualizationSpaceoutSettings::checkIsOptimizationsUsed(NFmiFastQueryInfo& fastInfo, const NFmiArea& mapArea, NFmiGrid& optimizedGridOut, int viewSubGridSize) const
{
    if(useGlobalVisualizationSpaceoutFactorOptimization() && fastInfo.IsGrid())
    {
        std::unique_ptr<NFmiArea> normalizedAreaPtr(mapArea.Clone());
        // Käytetyssä optimoidussa gridissä pitää olla peruskartta-aluen suhteen 0,0 - 1,1 alue
        normalizedAreaPtr->SetXYArea(NFmiRect(0, 0, 1, 1));
        auto baseGridSize = calcAreaGridSize(*normalizedAreaPtr, viewSubGridSize);
        auto dataOverMapWorldXyBoundingBox = calcInfoAreaOverMapAreaWorldXyBoundingBox(fastInfo, *normalizedAreaPtr);
        auto approximationDataGridSizeOverMapBoundingBox = ::calcApproximationDataGridSizeOverMapBoundingBox(fastInfo, dataOverMapWorldXyBoundingBox);
        auto baseGridSizeOverMapBoundingBox = ::calcBaseGridSizeOverMapBoundingBox(baseGridSize, normalizedAreaPtr->WorldRect(), dataOverMapWorldXyBoundingBox);
        if(::shouldOptimizedGridBeUsed(baseGridSizeOverMapBoundingBox, approximationDataGridSizeOverMapBoundingBox))
        {
            NFmiPoint bottomLeftLatlon = normalizedAreaPtr->WorldXYToLatLon(dataOverMapWorldXyBoundingBox.BottomLeft());
            NFmiPoint topRightLatlon = normalizedAreaPtr->WorldXYToLatLon(dataOverMapWorldXyBoundingBox.TopRight());
            std::unique_ptr<NFmiArea> optimizedArea(normalizedAreaPtr->CreateNewArea(bottomLeftLatlon, topRightLatlon));
            optimizedGridOut = NFmiGrid(optimizedArea.get(), boost::math::iround(baseGridSizeOverMapBoundingBox.X()), boost::math::iround(baseGridSizeOverMapBoundingBox.Y()));
            return true;
        }
    }
    return false;
}

static NFmiPoint getSmallerAreaPoint(const NFmiPoint& gridSize1, const NFmiPoint& gridSize2)
{
    auto area1 = gridSize1.X() * gridSize1.Y();
    auto area2 = gridSize2.X() * gridSize2.Y();
    if(area1 <= area2)
        return gridSize1;
    else
        return gridSize2;
}

NFmiPoint NFmiVisualizationSpaceoutSettings::getCheckedPossibleOptimizedGridSize(const NFmiPoint& suggestedGridSize, NFmiArea& mapArea, int viewSubGridSize) const
{
    if(useGlobalVisualizationSpaceoutFactorOptimization())
    {
        auto baseGridSize = calcAreaGridSize(mapArea, viewSubGridSize);
        return ::getSmallerAreaPoint(suggestedGridSize, baseGridSize);
    }
    return suggestedGridSize;
}

double NFmiVisualizationSpaceoutSettings::calcViewSubGridFactor(int viewSubGridSize)
{
    // Lineaarien ratkaisu 1 -> 1, 2 -> 0.99, 50 -> 0.4
//    double factor = -0.013265f * viewSubGridSize + 1.013265f;

    // Käänteisluvun potenssi potenssi ratkaisu:
    // 1 -> 1.0, 2 -> 0.812252,.., 4 -> 0.659754,.., 6 -> 0.584191,.., 9 -> 0.517282,.., 16 -> 0.435275,.., 50 -> 0.309249
    double factor = std::pow(std::pow(1./viewSubGridSize, 0.2), 1.5);
    return factor;
}

// Lasketaan perus hilakoko nykyisillä harvennus asetuksilla ja katsotaan minkälainen
// harvennettu hila saadaan annetun area:n avulla. Tässä otetaan huomioon alueen WorldXY rectin leveys/korkeus 
// suhde, niin että metreissä leveämpi kantti saa maksimi hilakoon ja kapeampi saa jakosuhteessa vähemmän hilapisteita.
NFmiPoint NFmiVisualizationSpaceoutSettings::calcAreaGridSize(NFmiArea& area, int viewSubGridSize) const
{
    double baseGridSize = baseSpaceoutGridSize();
    baseGridSize *= calcViewSubGridFactor(viewSubGridSize);
    auto areaWidthPerHeightFactor = area.WorldRect().Width() / area.WorldRect().Height();
    double gridsizeX = std::round(baseGridSize);
    double gridsizeY = std::round(baseGridSize);
    if(areaWidthPerHeightFactor >= 1)
        gridsizeY = std::round(gridsizeY / areaWidthPerHeightFactor);
    else
        gridsizeX = std::round(gridsizeX * areaWidthPerHeightFactor);
    return NFmiPoint(gridsizeX, gridsizeY);
}

// Käy hiladatan reunapisteet läpi ja katsoo miten data osuu mapArea:n WorldXy-arean avulla laskettuun boundinboxiin.
// Boundingbox pidetään kuitenkin rajattuna mapArea:n sisälle.
NFmiRect NFmiVisualizationSpaceoutSettings::calcInfoAreaOverMapAreaWorldXyBoundingBox(NFmiFastQueryInfo& fastInfo, NFmiArea& mapArea) const
{
    return NFmiWorldXyBoundingBoxCalculator::calcDataWorldXyBoundingBoxOverMapArea(fastInfo, mapArea);
}
