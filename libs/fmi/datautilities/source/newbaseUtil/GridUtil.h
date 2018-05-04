#pragma once
#include "source/common/Borders.h"

#include "NFmiLocation.h"
#include "NFmiArea.h"

namespace SmartMetDataUtilities {
    namespace GridUtil
    {
        namespace
        {
            template<typename Grid>
            void setGridToTopLeftPointOutsideBorders(Grid& grid, const Borders& latLonBorders)
            {
                setGridToLatLonPoint(grid, latLonBorders.topLeft());
                grid.Left();
                grid.Up();
            }
        }

        template<typename Grid>
        inline bool setGridToLatLonPoint(Grid& grid, const NFmiPoint &latLonPoint)
        {
            auto gridPoint = grid.LatLonToGrid(latLonPoint);
            return setGridToPoint(grid, gridPoint);
        }

        template<typename Grid>
        inline bool setGridToPoint(Grid& grid, const NFmiPoint &gridPoint)
        {
            return grid.GridPoint(gridPoint.X(), gridPoint.Y());
        }

        template<typename Grid>
        inline bool setGridToTopLeftCorner(Grid& grid)
        {
            auto topLeft = NFmiPoint(0, grid.YNumber() - 1);
            return GridUtil::setGridToPoint(grid, topLeft);
        }

        template<typename Grid>
        inline void setGridToTopLeftCornerJustOutsideBorders(Grid& grid, const Borders& latLonBorders)
        {
            setGridToTopLeftCorner(grid);
            setGridToTopLeftPointOutsideBorders(grid, latLonBorders);
        }

        template<typename Grid>
        inline double surfaceDistanceOfPoints(const Grid &grid, const NFmiPoint &firstPoint, const NFmiPoint &secondPoint)
        {
            auto firstPointOnMap = mapPointToSurface(grid, firstPoint);
            auto secondPointOnMap = mapPointToSurface(grid, secondPoint);
            return firstPointOnMap.Distance(secondPointOnMap);
        }

        template<typename Grid>
        inline NFmiLocation mapPointToSurface(const Grid &grid, const NFmiPoint &point)
        {
            return NFmiLocation(grid.GridToLatLon(point));
        }

        template <typename Grid>
        inline decltype(auto) rigthStepperFunctionFor(Grid& grid)
        {
            return [&grid](unsigned long numberOfSteps = 1)
            {
                return grid.Right(numberOfSteps);
            };
        }

        template <typename Grid>
        inline decltype(auto) upStepperFunctionFor(Grid& grid)
        {
            return [&grid](unsigned long numberOfSteps = 1)
            {
                return grid.Up(numberOfSteps);
            };
        }

        template <typename Grid>
        inline decltype(auto) leftStepperFunctionFor(Grid& grid)
        {
            return [&grid](unsigned long numberOfSteps = 1)
            {
                return grid.Left(numberOfSteps);
            };
        }

        template <typename Grid>
        inline decltype(auto) downStepperFunctionFor(Grid& grid)
        {
            return [&grid](unsigned long numberOfSteps = 1)
            {
                return grid.Down(numberOfSteps);
            };
        }

        template <typename Grid>
        inline bool gridFitsInsideArea(const Grid& grid, const NFmiArea& area)
        {
            auto gridArea = grid.Area();
            return area.IsInside(*gridArea);
        }

        template <typename Grid>
        inline std::string resolutionOf(const Grid& grid)
        {
            return std::to_string(grid.XNumber()) + "x" + std::to_string(grid.YNumber());
        }
    }
}
