#include "source/newbaseUtil/AreaUtil.h"
#include "source/newbaseUtil/Intersection.h"
#include "source/common/Borders.h"
#include "source/newbaseUtil/PointsUtil.h"

#include "NFmiArea.h"
#include "NFmiPoint.h"
#include "NFmiRect.h"

#include <memory>
#include <utility>
#include <limits>
#include <cmath>

using namespace std;

namespace SmartMetDataUtilities {

    namespace AreaUtil
    {
        using TopLeftAndBottomRight = pair<NFmiPoint, NFmiPoint>;
        namespace
        {
            struct MinMax
            {
                double Min;
                double Max;
            };

            MinMax initMinMax()
            {
                MinMax minMaxLongitude;
                minMaxLongitude.Min = std::numeric_limits<double>::max();
                minMaxLongitude.Max = -std::numeric_limits<double>::max();
                return minMaxLongitude;
            }

            void updateMinMaxWith(MinMax& minMax, double value)
            {
                minMax.Min = std::min(minMax.Min, value);
                minMax.Max = std::max(minMax.Max, value);
            }

            MinMax MinMaxForLongitude(const NFmiArea& area, size_t resolution)
            {
                MinMax minMaxLongitude = initMinMax();
                verticalForEach(area.Top(), area.Bottom(), area.Left(), resolution, [&](auto&& leftRelativePoint)
                {
                    auto rightLatLon = area.ToLatLon(NFmiPoint(area.Right(), leftRelativePoint.Y()));
                    auto leftLatLon = area.ToLatLon(leftRelativePoint);
                    updateMinMaxWith(minMaxLongitude, leftLatLon.X());
                    updateMinMaxWith(minMaxLongitude, rightLatLon.X());
                });
                return minMaxLongitude;
            }

            MinMax MinMaxForLatitude(const NFmiArea& area, size_t resolution)
            {
                MinMax minMaxLatitude = initMinMax();
                horizontalForEach(area.Left(), area.Right(), area.Top(), resolution, [&](auto&& topRelativePoint)
                {
                    auto bottomLatLon = area.ToLatLon(NFmiPoint(topRelativePoint.X(), area.Bottom()));
                    auto topLatLon = area.ToLatLon(topRelativePoint);
                    updateMinMaxWith(minMaxLatitude, topLatLon.Y());
                    updateMinMaxWith(minMaxLatitude, bottomLatLon.Y());
                });
                return minMaxLatitude;
            }

            Borders bordersOfIntersectionFor(const NFmiArea &area, NFmiGrid &grid)
            {
                Intersection intersection(area, grid);
                return intersection.getBorders();
            }

            TopLeftAndBottomRight findPointsRelativeToAreaFromGridThatDefineRectangleAroundArea(const NFmiArea &area, NFmiGrid &grid)
            {
                auto borders = bordersOfIntersectionFor(area, grid);
                return TopLeftAndBottomRight(borders.topLeft(), borders.bottomRight());
            }

            TopLeftAndBottomRight xyPairToLatLonPair(const NFmiArea &area, const TopLeftAndBottomRight& topLeftAndBottomRight)
            {
                auto topLeft = area.ToLatLon(topLeftAndBottomRight.first);
                auto bottomRight = area.ToLatLon(topLeftAndBottomRight.second);
                return TopLeftAndBottomRight(topLeft, bottomRight);
            }

            unique_ptr<NFmiArea> cropAreaToGrid(const NFmiArea &area, NFmiGrid &grid)
            {
                TopLeftAndBottomRight xy = findPointsRelativeToAreaFromGridThatDefineRectangleAroundArea(area, grid);
                TopLeftAndBottomRight latLon = xyPairToLatLonPair(area, xy);
                return unique_ptr<NFmiArea>(area.CreateNewArea(latLon.first, latLon.second));
            }

            bool area2ContainsAtleastOnePointFromArea1Borders(const NFmiArea& area1, const NFmiArea& area2, size_t resolution)
            {
                auto foundPointFromArea1ThatIsInsideArea2 = false;
                auto isInsideArea2 = [&](auto relativePoint)
                {
                    auto latlon = area1.ToLatLon(relativePoint);
                    if(area2.IsInside(latlon))
                    {
                        foundPointFromArea1ThatIsInsideArea2 = true;
                    }
                };
                verticalForEach(area1.Top(), area1.Bottom(), area1.Left(), resolution, isInsideArea2);
                verticalForEach(area1.Top(), area1.Bottom(), area1.Right(), resolution, isInsideArea2);
                horizontalForEach(area1.Left(), area1.Right(), area1.Top(), resolution, isInsideArea2);
                horizontalForEach(area1.Left(), area1.Right(), area1.Bottom(), resolution, isInsideArea2);
                return foundPointFromArea1ThatIsInsideArea2;
            }
        }

        // SmartMet demands this normalization to position any data on map view
        void normalizeArea(NFmiArea& area)
        {
            area.SetXYArea(NFmiRect(0, 0, 1, 1));
        }

        unique_ptr<NFmiArea> calculateCroppedArea(NFmiGrid& grid, const NFmiArea &mapArea)
        {
            auto croppedArea = cropAreaToGrid(mapArea, grid);
            normalizeArea(*croppedArea);
            return croppedArea;
        }

        Borders latLonBordersForRectangleAroundArea(const NFmiArea& area)
        {
            Borders borders;
            borders.leftBorder = MinLongitude(area);
            borders.rightBorder = MaxLongitude(area);
            borders.topBorder = MaxLatitude(area);
            borders.bottomBorder = MinLatitude(area);
            return borders;
        }

        double areaInSquareMetersFor(const NFmiArea& area)
        {
            return area.WorldXYHeight() * area.WorldXYWidth();
        }

        unique_ptr<NFmiArea> createCopyOf(const NFmiArea& area)
        {
            return unique_ptr<NFmiArea>(area.CreateNewArea(area.BottomLeftLatLon(), area.TopRightLatLon()));
        }

        bool isPacific(const NFmiArea& area)
        {
            return NFmiArea::IsPacificView_legacy(area.BottomLeftLatLon(), area.TopRightLatLon());
        }

        bool areasIntersect(const NFmiArea& area1, const NFmiArea& area2, size_t resolution)
        {
            if(area1.IsInside(area2) || area2.IsInside(area1))
            {
                return true;
            }
            return area2ContainsAtleastOnePointFromArea1Borders(area1, area2, resolution);
        }

        string makeAreaString(const NFmiArea& area)
        {
            // Newbase with wgs84 support doesn't support old style area strings anymore, 
            // we build here substitute string with PROJ library's Proj-string and area's corner points.
            auto areaStr = area.ProjStr();
            areaStr += ":";
            auto bottomLeftLatlon = area.BottomLeftLatLon();
            areaStr += to_string(bottomLeftLatlon.X());
            areaStr += ",";
            areaStr += to_string(bottomLeftLatlon.Y());
            auto topRightLatlon = area.TopRightLatLon();
            areaStr += ",";
            areaStr += to_string(topRightLatlon.X());
            areaStr += ",";
            areaStr += to_string(topRightLatlon.Y());

            return areaStr;
        }

        double MaxLongitude(const NFmiArea& area, size_t resolution)
        {
            auto minMaxLongitude = MinMaxForLongitude(area, resolution);
            return minMaxLongitude.Max;
        }
        double MaxLatitude(const NFmiArea& area, size_t resolution)
        {
            auto minMaxLatitude = MinMaxForLatitude(area, resolution);
            return minMaxLatitude.Max;
        }
        double MinLongitude(const NFmiArea& area, size_t resolution)
        {
            auto minMaxLongitude = MinMaxForLongitude(area, resolution);
            return minMaxLongitude.Min;
        }
        double MinLatitude(const NFmiArea& area, size_t resolution)
        {
            auto minMaxLatitude = MinMaxForLatitude(area, resolution);
            return minMaxLatitude.Min;
        }
    }
}