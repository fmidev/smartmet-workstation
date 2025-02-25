#include "source/newbaseUtil/AreaUtil.h"
#include "source/newbaseUtil/Intersection.h"
#include "source/common/Borders.h"
#include "source/newbaseUtil/PointsUtil.h"

#include "NFmiArea.h"
#include "NFmiPoint.h"
#include "NFmiRect.h"
#include "NFmiAzimuthalArea.h"

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

            bool isWorldAreaGrid(NFmiGrid& grid)
            {
                auto area = grid.Area();
                if(area && area->ClassId() == kNFmiLatLonArea)
                {
                    const auto atlanticWorldBottomLeft = NFmiPoint(-180, -90);
                    const auto atlanticWorldTopRight = NFmiPoint(180, 90);
                    const auto pacificWorldBottomLeft = NFmiPoint(0, -90);
                    const auto pacificWorldTopRight = NFmiPoint(360, 90);
                    auto bl = area->BottomLeftLatLon();
                    auto tr = area->TopRightLatLon();
                    auto oneGridPointWidth = (tr.X() - bl.X()) / (grid.OriginalXNumber() - 1);
                    oneGridPointWidth *= 1.01; // make it little wider so that comparison wont have problems with double accuracy
                    if(bl == atlanticWorldBottomLeft)
                    {
                        if(tr == atlanticWorldTopRight)
                            return true;
                        auto missingRightSideWidth = atlanticWorldTopRight.X() - tr.X();
                        if(oneGridPointWidth > missingRightSideWidth)
                            return true;
                    }

                    if(bl == pacificWorldBottomLeft)
                    {
                        if(tr == pacificWorldTopRight)
                            return true;
                        auto missingRightSideWidth = pacificWorldTopRight.X() - tr.X();
                        if(oneGridPointWidth > missingRightSideWidth)
                            return true;
                    }
                }
                return false;
            }

            unique_ptr<NFmiArea> cropAreaToGrid(const NFmiArea &area, NFmiGrid &grid)
            {
                if(isWorldAreaGrid(grid))
                {
                    // If world area data, original map-area is always good
                    return unique_ptr<NFmiArea>(area.Clone());
                }
                else
                {
                    TopLeftAndBottomRight xy = findPointsRelativeToAreaFromGridThatDefineRectangleAroundArea(area, grid);
                    TopLeftAndBottomRight latLon = xyPairToLatLonPair(area, xy);
                    return unique_ptr<NFmiArea>(area.CreateNewArea(latLon.first, latLon.second));
                }
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

        string firstPartOfAreaStrFor(const NFmiArea& area)
        {
            auto areaStr = area.AreaStr();
            auto split = NFmiStringTools::Split(areaStr, ":");
            return split.front();
        }

        bool isPacific(const NFmiArea& area)
        {
            return NFmiArea::IsPacificView(area.BottomLeftLatLon(), area.TopRightLatLon());
        }

        bool areasIntersect(const NFmiArea& area1, const NFmiArea& area2, size_t resolution)
        {
            if(area1.IsInside(area2) || area2.IsInside(area1))
            {
                return true;
            }
            return area2ContainsAtleastOnePointFromArea1Borders(area1, area2, resolution);
        }

        bool areAreasEqual(NFmiArea* area1, NFmiArea* area2, double epsilon)
        {
            if(area1->ClassId() == area2->ClassId())
            {
                if(PointsUtil::pointsAreEqual(area1->BottomLeftLatLon(), area2->BottomLeftLatLon(), epsilon)
                    && PointsUtil::pointsAreEqual(area1->TopRightLatLon(), area2->TopRightLatLon(), epsilon))
                {
                    if(area1->ClassId() == kNFmiStereographicArea || area1->ClassId() == kNFmiEquiDistArea || area1->ClassId() == kNFmiGnomonicArea)
                    {
                        if(static_cast<const NFmiAzimuthalArea*>(area1)->Orientation() == static_cast<const NFmiAzimuthalArea*>(area2)->Orientation())
                            return true;
                    }
                }
            }
            return false;
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