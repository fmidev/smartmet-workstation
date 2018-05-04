#pragma once
#include "datautilities/../source/newbaseUtil/Intersection.h"
#include "datautilities/../source/common/Borders.h"

#include "NFmiFastQueryInfo.h"
#include "NFmiArea.h"
#include "NFmiGrid.h"

namespace ReferenceAreaCreator
{
    namespace
    {
        decltype(auto) loopGridAndCalculateIntersectionBordersWithArea(NFmiGrid& grid, const NFmiArea &area)
        {
            SmartMetDataUtilities::Borders borders;
            grid.First();
            do
            {
                auto latLon = grid.LatLon();
                if(!area.IsInside(latLon))
                {
                    continue;
                }
                auto relativePoint = area.ToXY(latLon);
                borders.updateBorderValuesWith(relativePoint);
            } while(grid.Next());
            return borders;
        }

        decltype(auto) loopGridAndCalculateBordersRelativeToArea(NFmiGrid& grid, const NFmiArea &area)
        {
            SmartMetDataUtilities::Borders borders;
            grid.First();
            do
            {
                auto latLon = grid.LatLon();
                auto relativePoint = area.ToXY(latLon);
                borders.updateBorderValuesWith(relativePoint);
            } while(grid.Next());
            return borders;
        }

        decltype(auto) getSurroundingBordersForInfoWithArea(NFmiFastQueryInfo& info, const NFmiArea& area)
        {
            auto grid = *const_cast<NFmiGrid*>(info.Grid());

            if(area.IsInside(*grid.Area()))
            {
                return loopGridAndCalculateBordersRelativeToArea(grid, area);
            }
            else
            {
                return loopGridAndCalculateIntersectionBordersWithArea(grid, area);
            }
        }
    }
    decltype(auto) getReferenceAreaWith(NFmiFastQueryInfo& info, const NFmiArea& area)
    {
        auto borders = getSurroundingBordersForInfoWithArea(info, area);
        auto bottomLeftLatLon = area.ToLatLon(NFmiPoint(borders.leftBorder, borders.topBorder));
        auto topRightLatLon = area.ToLatLon(NFmiPoint(borders.rightBorder, borders.bottomBorder));

        auto referenceArea = area.CreateNewArea(bottomLeftLatLon, topRightLatLon);
        referenceArea->SetXYArea(NFmiRect(0, 0, 1, 1));
        return referenceArea;
    }
}
