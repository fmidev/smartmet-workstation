#include "source/newbaseUtil/Intersection.h"
#include "source/newbaseUtil/GridUtil.h"
#include "source/common/Borders.h"
#include "source/newbaseUtil/AreaUtil.h"

#include "NFmiPoint.h"

#include <functional>

namespace SmartMetDataUtilities
{
    namespace
    {
        void convertBordersFromAtlanticToPacificOrViceVersaIfNeeded(Borders& latLonBorders, const NFmiArea& gridArea)
        {
            if(AreaUtil::isPacific(gridArea) == Borders::isPacific(latLonBorders))
            {
            }
            else if(Borders::isPacific(latLonBorders))
            {
                latLonBorders.convertToAtlantic();
            }
            else
            {
                latLonBorders.convertToPacific();
            }
        }
    }

    Intersection::Intersection(const NFmiArea &area, NFmiGrid &grid)
        : area_(area)
        , grid_(grid)
        , right_(GridUtil::rigthStepperFunctionFor(grid))
        , left_(GridUtil::leftStepperFunctionFor(grid))
        , up_(GridUtil::upStepperFunctionFor(grid))
        , down_(GridUtil::downStepperFunctionFor(grid))
    {
    }

    Borders Intersection::getBorders()
    {
        if(intersectionBorders_.isUnmodified())
        {
            calculate();
        }
        return intersectionBorders_;
    }

    void Intersection::calculate()
    {
        if(GridUtil::gridFitsInsideArea(grid_, area_))
        {
            loopGrid();
        }
        else
        {
            loopGridWithLimits();
        }
    };

    void Intersection::loopGridWithLimits()
    {
        auto areaBorders = AreaUtil::latLonBordersForRectangleAroundArea(area_);
        convertBordersFromAtlanticToPacificOrViceVersaIfNeeded(areaBorders, *grid_.Area());
        GridUtil::setGridToTopLeftCornerJustOutsideBorders(grid_, areaBorders);
        loopGridWithGivenStepper([&areaBorders, &right = right_](auto latLon) { return right() && latLon.X() <= areaBorders.rightBorder; });
        loopGridWithGivenStepper([&areaBorders, &down = down_](auto latLon) { return down() && latLon.Y() >= areaBorders.bottomBorder; });
        loopGridWithGivenStepper([&areaBorders, &left = left_](auto latLon) { return left() && latLon.X() >= areaBorders.leftBorder; });
        loopGridWithGivenStepper([&areaBorders, &up = up_](auto latLon) { return up() && latLon.Y() <= areaBorders.topBorder; });
    }

    void Intersection::loopGrid()
    {
        GridUtil::setGridToTopLeftCorner(grid_);
        loopGridWithGivenStepper([&right = right_](auto latLon) { return right(); });
        loopGridWithGivenStepper([&down = down_](auto latLon) { return down(); });
        loopGridWithGivenStepper([&left = left_](auto latLon) { return left(); });
        loopGridWithGivenStepper([&up = up_](auto latLon) { return up(); });
    }

    void Intersection::loopGridWithGivenStepper(const StepperWithPossibilityToDoChecking& stepperWithPossibleCheck)
    {
        NFmiPoint latLon;
        do
        {
            latLon = grid_.LatLon();
            auto relativePoint = area_.ToXY(latLon);
            intersectionBorders_.updateBorderValuesWith(relativePoint);
        } while(stepperWithPossibleCheck(latLon));
    }

    bool Intersection::gridAndMapAreasIntersect(NFmiGrid& grid, const NFmiArea& mapArea)
    {
        auto& gridArea = *grid.Area();
        return AreaUtil::areasIntersect(gridArea, mapArea);
    }
}