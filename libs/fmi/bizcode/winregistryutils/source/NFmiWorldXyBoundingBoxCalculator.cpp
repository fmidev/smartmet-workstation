#include "NFmiWorldXyBoundingBoxCalculator.h"
#include "NFmiFastQueryInfo.h"
#include "NFmiArea.h"
#include "NFmiQueryDataUtil.h"
#include "MathHelper.h"

namespace
{
    NFmiRect makeBoundingBoxFromEdgePoints(const std::set<double>& leftValues, const std::set<double>& rightValues, const std::set<double>& bottomValues, const std::set<double>& topValues)
    {
        double leftWorldXy = *leftValues.rbegin(); // rbegin = suurin arvo lefteist‰
        double rightWorldXy = *rightValues.begin(); // begin = pienin arvo righteista
        double bottomWorldXy = *bottomValues.rbegin(); // rbegin = suurin arvo bottomeista
        double topWorldXy = *topValues.begin(); // begin = pienin arvo topeista

        return NFmiRect(leftWorldXy, topWorldXy, rightWorldXy, bottomWorldXy);
    }

    unsigned long calcEdgePointSteppingCount(unsigned long gridSize)
    {
        unsigned long step = boost::math::iround(gridSize / 30.);
        if(step < 1)
            step = 1;
        return step;
    }
}


NFmiWorldXyBoundingBoxCalculator::NFmiWorldXyBoundingBoxCalculator() = default;

void NFmiWorldXyBoundingBoxCalculator::clear()
{
    *this = NFmiWorldXyBoundingBoxCalculator();
}

NFmiRect NFmiWorldXyBoundingBoxCalculator::calcGridDataAreaOverMapAreaWorldXyBoundingBox(NFmiFastQueryInfo & fastInfo, NFmiArea & mapArea)
{
    clear();
    mapWorldXyRect_ = mapArea.WorldRect();
    NFmiRect worldXyBoundingBox;
    if(doBoundingBoxWithSameKindAreas(fastInfo, mapArea, worldXyBoundingBox))
        return worldXyBoundingBox;
    if(doBoundingBoxWithDataCornerPoints(fastInfo, mapArea, worldXyBoundingBox))
        return worldXyBoundingBox;
    return doBoundingBoxWithDataEdgePoints(fastInfo, mapArea);
}

bool NFmiWorldXyBoundingBoxCalculator::doBoundingBoxWithSameKindAreas(NFmiFastQueryInfo& fastInfo, NFmiArea& mapArea, NFmiRect& worldXyBoundingBox)
{
    // Jos area-projektiot ovat saman tyylisi‰, on boundingbox helppo laskea kahden pisteen avulla
    if(NFmiQueryDataUtil::AreAreasSameKind(&mapArea, fastInfo.Area()))
    {
        auto bottomLeftWorldXyPoint = mapArea.LatLonToWorldXY(fastInfo.Area()->BottomLeftLatLon());
        auto topRightWorldXyPoint = mapArea.LatLonToWorldXY(fastInfo.Area()->TopRightLatLon());

        std::set<double> leftValues{ mapWorldXyRect_.Left(), bottomLeftWorldXyPoint.X() };
        std::set<double> rightValues{ mapWorldXyRect_.Right(), topRightWorldXyPoint.X() };
        // Huom! NFmiRect:in Bottom ja Top k‰site on k‰‰nteinen, kuin area/grid maailmassa
        std::set<double> bottomValues{ mapWorldXyRect_.Top(), bottomLeftWorldXyPoint.Y() };
        std::set<double> topValues{ mapWorldXyRect_.Bottom(), topRightWorldXyPoint.Y() };

        worldXyBoundingBox = ::makeBoundingBoxFromEdgePoints(leftValues, rightValues, bottomValues, topValues);
        return true;
    }
    return false;
}

bool NFmiWorldXyBoundingBoxCalculator::doBoundingBoxWithDataCornerPoints(NFmiFastQueryInfo& fastInfo, NFmiArea& mapArea, NFmiRect& worldXyBoundingBox)
{
    // Kokeillaan jos pelkill‰ kulmapisteiden avulla saadaan koko kartta-alueen peitt‰v‰ bounding-box
    auto bottomLeftWorldXyPoint = mapArea.LatLonToWorldXY(fastInfo.Area()->BottomLeftLatLon());
    auto bottomRightWorldXyPoint = mapArea.LatLonToWorldXY(fastInfo.Area()->BottomRightLatLon());
    auto topLeftWorldXyPoint = mapArea.LatLonToWorldXY(fastInfo.Area()->TopLeftLatLon());
    auto topRightWorldXyPoint = mapArea.LatLonToWorldXY(fastInfo.Area()->TopRightLatLon());
    std::set<double> leftValues{ mapWorldXyRect_.Left(), bottomLeftWorldXyPoint.X(), topLeftWorldXyPoint.X() };
    std::set<double> rightValues{ mapWorldXyRect_.Right(), bottomRightWorldXyPoint.X(), topRightWorldXyPoint.X() };
    // Huom! NFmiRect:in Bottom ja Top k‰site on k‰‰nteinen, kuin area/grid maailmassa
    std::set<double> bottomValues{ mapWorldXyRect_.Top(), bottomLeftWorldXyPoint.Y(), bottomRightWorldXyPoint.Y() };
    std::set<double> topValues{ mapWorldXyRect_.Bottom(), topLeftWorldXyPoint.Y(), topRightWorldXyPoint.Y() };

    worldXyBoundingBox = ::makeBoundingBoxFromEdgePoints(leftValues, rightValues, bottomValues, topValues);
    return mapWorldXyRect_ == worldXyBoundingBox;
}

void NFmiWorldXyBoundingBoxCalculator::checkXyPoint(const NFmiPoint& worlXyPoint)
{
    auto x = worlXyPoint.X();
    // Left edge check
    if(x < boundingLeft_)
    {
        if(x < mapWorldXyRect_.Left())
            boundingLeft_ = mapWorldXyRect_.Left();
        else
            boundingLeft_ = x;
    }
    // Right edge check
    if(x > boundingRight_)
    {
        if(x > mapWorldXyRect_.Right())
            boundingRight_ = mapWorldXyRect_.Right();
        else
            boundingRight_ = x;
    }

    auto y = worlXyPoint.Y();
    // Huom! y-suunnassa taas k‰‰ntyneet suunnat
    // 
    // bottom edge check
    if(y < boundingBottom_)
    {
        if(y < mapWorldXyRect_.Top())
            boundingBottom_ = mapWorldXyRect_.Top();
        else
            boundingBottom_ = y;
    }
    // top edge check
    if(y > boundingTop_)
    {
        if(y > mapWorldXyRect_.Bottom())
            boundingTop_ = mapWorldXyRect_.Bottom();
        else
            boundingTop_ = y;
    }
}

NFmiRect NFmiWorldXyBoundingBoxCalculator::doBoundingBoxWithDataEdgePoints(NFmiFastQueryInfo& fastInfo, NFmiArea& mapArea)
{
    // Aloitetaan iteroimaan hilan reunapisteit‰ l‰pi harvennetusti (ei viitsi turhaan tutkia 1000x1000 hilan jokaista reunapistett‰)
    auto gridSizeX = fastInfo.GridXNumber();
    auto gridSizeY = fastInfo.GridYNumber();
    unsigned long xStep = ::calcEdgePointSteppingCount(gridSizeX);
    // K‰yd‰‰n ensin yl‰ ja ala vaakarivit
    for(unsigned long xIndex = 0; xIndex < gridSizeX - 1; xIndex += xStep)
    {
        unsigned long latlonIndexBottom = xIndex;
        auto worldXyPointBottom = mapArea.LatLonToWorldXY(fastInfo.LatLon(latlonIndexBottom));
        checkXyPoint(worldXyPointBottom);

        unsigned long latlonIndexTop = gridSizeX * (gridSizeY - 1) + xIndex;
        auto worldXyPointTop = mapArea.LatLonToWorldXY(fastInfo.LatLon(latlonIndexTop));
        checkXyPoint(worldXyPointTop);
    }

    unsigned long yStep = ::calcEdgePointSteppingCount(gridSizeY);
    // K‰yd‰‰n sitten vasemman ja oikean puoleiset pystyrivit
    for(unsigned long yIndex = 0; yIndex < gridSizeY - 1; yIndex += yStep)
    {
        unsigned long latlonIndexLeft = yIndex * gridSizeX;
        auto worldXyPointLeft = mapArea.LatLonToWorldXY(fastInfo.LatLon(latlonIndexLeft));
        checkXyPoint(worldXyPointLeft);

        unsigned long latlonIndexRight = (yIndex + 1) * gridSizeX - 1;
        auto worldXyPointRight = mapArea.LatLonToWorldXY(fastInfo.LatLon(latlonIndexRight));
        checkXyPoint(worldXyPointRight);
    }
    // Varmuuden vuoksi viel‰ lis‰t‰‰n top-right ja bottom-right pisteet, koska k‰ytetyt stepit saattoivat skipata kyseiset pisteet
    auto bottomRightWorldXyPoint = mapArea.LatLonToWorldXY(fastInfo.Area()->BottomRightLatLon());
    checkXyPoint(bottomRightWorldXyPoint);
    auto topRightWorldXyPoint = mapArea.LatLonToWorldXY(fastInfo.Area()->TopRightLatLon());
    checkXyPoint(topRightWorldXyPoint);

    return NFmiRect(boundingLeft_, boundingBottom_, boundingRight_, boundingTop_);
}

NFmiRect NFmiWorldXyBoundingBoxCalculator::calcDataWorldXyBoundingBoxOverMapArea(NFmiFastQueryInfo& fastInfo, NFmiArea& mapArea)
{
    NFmiWorldXyBoundingBoxCalculator boundingBoxCalculator;
    return boundingBoxCalculator.calcGridDataAreaOverMapAreaWorldXyBoundingBox(fastInfo, mapArea);
}
