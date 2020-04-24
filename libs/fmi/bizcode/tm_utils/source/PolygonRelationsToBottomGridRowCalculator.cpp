#include "PolygonRelationsToBottomGridRowCalculator.h"
#include "NFmiDataModifierMin.h"
#include "NFmiDataModifierMax.h"

void PolygonRelationsToBottomGridRowCalculator::addCoordinatesX(const std::vector<float>& baseCoordinates, size_t startIndex, size_t polygonSize)
{
    auto startIter = baseCoordinates.begin() + startIndex;
    coordinatesX_ = std::vector<float>(startIter, startIter + polygonSize);
}

void PolygonRelationsToBottomGridRowCalculator::addCoordinatesY(const std::vector<float>& baseCoordinates, size_t startIndex, size_t polygonSize)
{
    auto startIter = baseCoordinates.begin() + startIndex;
    coordinatesY_ = std::vector<float>(startIter, startIter + polygonSize);
}

void PolygonRelationsToBottomGridRowCalculator::setBottomRowCoordinateY(float bottomRowCoordinateY)
{
    bottomRowCoordinateY_ = bottomRowCoordinateY;
}

PolygonsBottomEdgeTouching PolygonRelationsToBottomGridRowCalculator::getResult()
{
    doCalculations();
    if(calculationOk_)
        return polygonsBottomEdgeTouching_;
    else
        throw std::runtime_error("Unknown error in PolygonRelationsToBottomGridRowCalculator");
}

void PolygonRelationsToBottomGridRowCalculator::doCalculations()
{
    if(!calculationOk_)
    {
        if(bottomRowCoordinateY_ == kFloatMissing)
            throw std::runtime_error("Error in PolygonRelationsToBottomGridRowCalculator, bottom-row-coordinate was not set");
        if(coordinatesX_.empty() || coordinatesX_.size() != coordinatesY_.size())
            throw std::runtime_error("Error in PolygonRelationsToBottomGridRowCalculator, illegal hatch polygon coordinate setup");

        NFmiDataModifierMin minX;
        NFmiDataModifierMax maxX;
        NFmiDataModifierMin minBottomRowTouchingX;
        NFmiDataModifierMax maxBottomRowTouchingX;
        for(size_t coordinateIndex = 0; coordinateIndex < coordinatesX_.size(); coordinateIndex++)
        {
            auto coordinateX = coordinatesX_[coordinateIndex];
            minX.Calculate(coordinateX);
            maxX.Calculate(coordinateX);
            auto coordinateY = coordinatesY_[coordinateIndex];
            if(coordinateY == bottomRowCoordinateY_)
            {
                minBottomRowTouchingX.Calculate(coordinateX);
                maxBottomRowTouchingX.Calculate(coordinateX);
            }
        }
        polygonsBottomEdgeTouching_ = calcBottomTouchingIndex(static_cast<float>(minX.FloatValue()), static_cast<float>(maxX.FloatValue()), static_cast<float>(minBottomRowTouchingX.FloatValue()), static_cast<float>(maxBottomRowTouchingX.FloatValue()));
        calculationOk_ = true;
    }
}

PolygonsBottomEdgeTouching PolygonRelationsToBottomGridRowCalculator::calcBottomTouchingIndex(float minTotalX, float maxTotalX, float minBottomRowTouchingX, float maxBottomRowTouchingX)
{
    if(minTotalX == kFloatMissing ||  maxTotalX == kFloatMissing)
        throw std::runtime_error("Error in PolygonRelationsToBottomGridRowCalculator, calculated min/max x-coordinates were illegal");

    if(minBottomRowTouchingX == kFloatMissing || maxBottomRowTouchingX == kFloatMissing)
        return PolygonsBottomEdgeTouching::AlwaysNotTouching;
    if(minBottomRowTouchingX == minTotalX && maxBottomRowTouchingX == maxTotalX)
        return PolygonsBottomEdgeTouching::AlwaysTouching;
    auto totalDiff = maxTotalX - minTotalX;
    auto touchingDiff = maxBottomRowTouchingX - minBottomRowTouchingX;
    if(touchingDiff / totalDiff > .9)
        return PolygonsBottomEdgeTouching::AlmostAlwaysTouching;
    if(minBottomRowTouchingX > minTotalX)
        return PolygonsBottomEdgeTouching::FirstNotTouching;
    else
        return PolygonsBottomEdgeTouching::FirstTouching;
}

