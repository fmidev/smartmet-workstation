#include "NFmiVisualizationSpaceoutSettings.h"
#include "catlog/catlog.h"
#include "NFmiValueString.h"
#include "MathHelper.h"
#include "NFmiFastQueryInfo.h"
#include "NFmiQueryDataUtil.h"
#include "NFmiArea.h"

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

// Metodi saa Visualization settings -dialogilta kaikki nykyiset asetukset.
// Jos mitään asetus oikeasti muuttuu niin että karttanäyttöjä pitää päivittää ja tehdä dirty asetuksia, palautetaan true, muuten false.
bool NFmiVisualizationSpaceoutSettings::updateFromDialog(double pixelToGridPointRatio, bool usePixelToGridPointRatioSafetyFeature, double globalVisualizationSpaceoutFactor, bool useGlobalVisualizationSpaceoutFactorOptimization, int spaceoutDataGatheringMethod)
{
    bool needsToUpdateViews = false;
    if(*pixelToGridPointRatio_ != pixelToGridPointRatio)
    {
        *pixelToGridPointRatio_ = pixelToGridPointRatio;
        needsToUpdateViews = true;
    }
    if(usePixelToGridPointRatioSafetyFeature_ != usePixelToGridPointRatioSafetyFeature)
    {
        usePixelToGridPointRatioSafetyFeature_ = usePixelToGridPointRatioSafetyFeature;
        needsToUpdateViews = true;
    }
    if(*globalVisualizationSpaceoutFactor_ != globalVisualizationSpaceoutFactor)
    {
        *globalVisualizationSpaceoutFactor_ = globalVisualizationSpaceoutFactor;
        needsToUpdateViews = true;
    }
    if(*useGlobalVisualizationSpaceoutFactorOptimization_ != useGlobalVisualizationSpaceoutFactorOptimization)
    {
        *useGlobalVisualizationSpaceoutFactorOptimization_ = useGlobalVisualizationSpaceoutFactorOptimization;
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
        std::string logMessage = "Global-Visualization-Spaceout-Factor optimization is set on, with value ";
        logMessage += NFmiValueString::GetStringWithMaxDecimalsSmartWay(*globalVisualizationSpaceoutFactor_, 1);
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

void NFmiVisualizationSpaceoutSettings::spaceoutDataGatheringMethod(int /* newValue */ )
{
    //if(newValue < 0)
    //    newValue = 0;
    //if(newValue > 1)
    //    newValue = 1;
    *spaceoutDataGatheringMethod_ = 0; // newValue;
}

double NFmiVisualizationSpaceoutSettings::calcBaseOptimizedGridSize(double usedSpaceoutFactor) const
{
    return MathHelper::InterpolateWithTwoPoints(usedSpaceoutFactor, minVisualizationSpaceoutFactor_, maxVisualizationSpaceoutFactor_, maxVisualizationSpaceoutGridSize_, minVisualizationSpaceoutGridSize_, minVisualizationSpaceoutGridSize_, maxVisualizationSpaceoutGridSize_);
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
bool NFmiVisualizationSpaceoutSettings::checkIsOptimizationsUsed(NFmiFastQueryInfo& fastInfo, const NFmiArea& mapArea, NFmiGrid& optimizedGridOut) const
{
    if(useGlobalVisualizationSpaceoutFactorOptimization() && fastInfo.IsGrid())
    {
        std::unique_ptr<NFmiArea> normalizedAreaPtr(mapArea.Clone());
        // Käytetyssä optimoidussa gridissä pitää olla peruskartta-aluen suhteen 0,0 - 1,1 alue
        normalizedAreaPtr->SetXYArea(NFmiRect(0, 0, 1, 1));
        auto baseGridSize = calcAreaGridSize(*normalizedAreaPtr);
        auto dataOverMapWorldXyBoundingBox = calcInfoAreaOverMapAreaWorldXyBoundingBox(fastInfo, *normalizedAreaPtr);
        auto approximationDataGridSizeOverMapBoundingBox = ::calcApproximationDataGridSizeOverMapBoundingBox(fastInfo, dataOverMapWorldXyBoundingBox);
        auto baseGridSizeOverMapBoundingBox = ::calcBaseGridSizeOverMapBoundingBox(baseGridSize, normalizedAreaPtr->WorldRect(), dataOverMapWorldXyBoundingBox);
        if(::shouldOptimizedGridBeUsed(baseGridSizeOverMapBoundingBox, approximationDataGridSizeOverMapBoundingBox))
        {
            NFmiPoint bottomLeftLatlon = normalizedAreaPtr->WorldXYToLatLon(dataOverMapWorldXyBoundingBox.BottomLeft());
            NFmiPoint topRightLatlon = normalizedAreaPtr->WorldXYToLatLon(dataOverMapWorldXyBoundingBox.TopRight());
            auto optimizedArea = normalizedAreaPtr->CreateNewArea(bottomLeftLatlon, topRightLatlon);
            optimizedGridOut = NFmiGrid(optimizedArea, boost::math::iround(baseGridSizeOverMapBoundingBox.X()), boost::math::iround(baseGridSizeOverMapBoundingBox.Y()));
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

NFmiPoint NFmiVisualizationSpaceoutSettings::getCheckedPossibleOptimizedGridSize(const NFmiPoint& suggestedGridSize, NFmiArea& mapArea) const
{
    if(useGlobalVisualizationSpaceoutFactorOptimization())
    {
        auto baseGridSize = calcAreaGridSize(mapArea);
        return ::getSmallerAreaPoint(suggestedGridSize, baseGridSize);
    }
    return suggestedGridSize;
}

// Lasketaan perus hilakoko nykyisillä harvennus asetuksilla ja katsotaan minkälainen
// harvennettu hila saadaan annetun area:n avulla. Tässä otetaan huomioon alueen WorldXY rectin leveys/korkeus 
// suhde, niin että metreissä leveämpi kantti saa maksimi hilakoon ja kapeampi saa jakosuhteessa vähemmän hilapisteita.
NFmiPoint NFmiVisualizationSpaceoutSettings::calcAreaGridSize(NFmiArea& area) const
{
    auto baseGridSize = calcBaseOptimizedGridSize(globalVisualizationSpaceoutFactor());
    auto areaWidthPerHeightFactor = area.WorldRect().Width() / area.WorldRect().Height();
    double gridsizeX = std::round(baseGridSize);
    double gridsizeY = std::round(baseGridSize);
    if(areaWidthPerHeightFactor >= 1)
        gridsizeY = std::round(gridsizeY / areaWidthPerHeightFactor);
    else
        gridsizeX = std::round(gridsizeX * areaWidthPerHeightFactor);
    return NFmiPoint(gridsizeX, gridsizeY);
}

static NFmiRect makeBoundingBoxFromEdgePoints(const std::set<double>& leftValues, const std::set<double>& rightValues, const std::set<double>& bottomValues, const std::set<double>& topValues)
{
    double leftWorldXy = *leftValues.rbegin(); // rbegin = suurin arvo lefteistä
    double rightWorldXy = *rightValues.begin(); // begin = pienin arvo righteista
    double bottomWorldXy = *bottomValues.rbegin(); // rbegin = suurin arvo bottomeista
    double topWorldXy = *topValues.begin(); // begin = pienin arvo topeista

    return NFmiRect(leftWorldXy, topWorldXy, rightWorldXy, bottomWorldXy);
}

static bool doBoundingBoxWithSameKindAreas(NFmiFastQueryInfo& fastInfo, NFmiArea& mapArea, NFmiRect &worldXyBoundingBox)
{
    const auto& worldXyRect = fastInfo.Area()->WorldRect();
    // Jos area-projektiot ovat saman tyylisiä, on boundingbox helppo laskea kahden pisteen avulla
    if(NFmiQueryDataUtil::AreAreasSameKind(&mapArea, fastInfo.Area()))
    {
        auto bottomLeftWorldXyPoint = mapArea.LatLonToWorldXY(fastInfo.Area()->BottomLeftLatLon());
        auto topRightWorldXyPoint = mapArea.LatLonToWorldXY(fastInfo.Area()->TopRightLatLon());

        std::set<double> leftValues{ worldXyRect.Left(), bottomLeftWorldXyPoint.X() };
        std::set<double> rightValues{ worldXyRect.Right(), topRightWorldXyPoint.X() };
        std::set<double> bottomValues{ worldXyRect.Bottom(), bottomLeftWorldXyPoint.Y() };
        std::set<double> topValues{ worldXyRect.Top(), topRightWorldXyPoint.Y() };

        worldXyBoundingBox = ::makeBoundingBoxFromEdgePoints(leftValues, rightValues, bottomValues, topValues);
        return true;
    }
    return false;
}

static unsigned long calcEdgePointSteppingCount(unsigned long gridSize)
{
    unsigned long step = boost::math::iround(gridSize / 30.);
    if(step < 1)
        step = 1;
    return step;
}

// Käy hiladatan reunapisteet läpi ja katsoo miten data osuu mapArea:n WorldXy-arean avulla laskettuun boundinboxiin.
// Boundingbox pidetään kuitenkin rajattuna mapArea:n sisälle.
NFmiRect NFmiVisualizationSpaceoutSettings::calcInfoAreaOverMapAreaWorldXyBoundingBox(NFmiFastQueryInfo& fastInfo, NFmiArea& mapArea) const
{
    NFmiRect worldXyBoundingBox;
    if(::doBoundingBoxWithSameKindAreas(fastInfo, mapArea, worldXyBoundingBox))
        return worldXyBoundingBox;

    const auto& worldXyRect = fastInfo.Area()->WorldRect();

    // Mennään läpi ensin kulmapisteet, jos ne peittävä jo kartta-alueen, ei tarvitse tutkia pidemmälle
    auto bottomLeftWorldXyPoint = mapArea.LatLonToWorldXY(fastInfo.Area()->BottomLeftLatLon());
    auto bottomRightWorldXyPoint = mapArea.LatLonToWorldXY(fastInfo.Area()->BottomRightLatLon());
    auto topLeftWorldXyPoint = mapArea.LatLonToWorldXY(fastInfo.Area()->TopLeftLatLon());
    auto topRightWorldXyPoint = mapArea.LatLonToWorldXY(fastInfo.Area()->TopRightLatLon());
    std::set<double> leftValues{ worldXyRect.Left(), bottomLeftWorldXyPoint.X(), topLeftWorldXyPoint.X() };
    std::set<double> rightValues{ worldXyRect.Right(), bottomRightWorldXyPoint.X(), topRightWorldXyPoint.X() };
    std::set<double> bottomValues{ worldXyRect.Bottom(), bottomLeftWorldXyPoint.Y(), bottomRightWorldXyPoint.Y() };
    std::set<double> topValues{ worldXyRect.Top(), topLeftWorldXyPoint.Y(), topRightWorldXyPoint.Y() };

    worldXyBoundingBox = ::makeBoundingBoxFromEdgePoints(leftValues, rightValues, bottomValues, topValues);
    if(worldXyRect == worldXyBoundingBox)
        return worldXyBoundingBox;

    // Muuten aloitetaan iteroimaan hilan reunapisteitä läpi harvennetusti (ei viitsi turhaan tutkia 1000x1000 hilan jokaista reunapistettä)
    auto gridSizeX = fastInfo.GridXNumber();
    auto gridSizeY = fastInfo.GridYNumber();
    unsigned long xStep = ::calcEdgePointSteppingCount(gridSizeX);
    // Käydään ensin ylä- ja alarivit
    for(unsigned long xIndex = xStep; xIndex < gridSizeX - 1; xIndex += xStep)
    {
        unsigned long latlonIndexBottom = xIndex;
        auto worldXyPointBottom = mapArea.LatLonToWorldXY(fastInfo.LatLon(latlonIndexBottom));
        bottomValues.insert(worldXyPointBottom.Y());
        leftValues.insert(worldXyPointBottom.X());
        rightValues.insert(worldXyPointBottom.X());

        unsigned long latlonIndexTop = gridSizeX * (gridSizeY - 1) + xIndex;
        auto worldXyPointTop = mapArea.LatLonToWorldXY(fastInfo.LatLon(latlonIndexTop));
        topValues.insert(worldXyPointTop.Y());
        leftValues.insert(worldXyPointTop.X());
        rightValues.insert(worldXyPointTop.X());
    }

    unsigned long yStep = ::calcEdgePointSteppingCount(gridSizeY);
    // Käydään reunimmaiset pystyrivit
    for(unsigned long yIndex = yStep; yIndex < gridSizeY - 1; yIndex += yStep)
    {
        unsigned long latlonIndexLeft = yIndex * gridSizeX;
        auto worldXyPointLeft = mapArea.LatLonToWorldXY(fastInfo.LatLon(latlonIndexLeft));
        bottomValues.insert(worldXyPointLeft.Y());
        topValues.insert(worldXyPointLeft.Y());
        leftValues.insert(worldXyPointLeft.X());

        unsigned long latlonIndexRight = (yIndex + 1) * gridSizeX - 1;
        auto worldXyPointRight = mapArea.LatLonToWorldXY(fastInfo.LatLon(latlonIndexRight));
        bottomValues.insert(worldXyPointRight.Y());
        topValues.insert(worldXyPointRight.Y());
        leftValues.insert(worldXyPointRight.X());
    }

    return ::makeBoundingBoxFromEdgePoints(leftValues, rightValues, bottomValues, topValues);
}
