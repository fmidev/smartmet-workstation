#pragma once

#include <memory>
#include <string>

class NFmiArea;
class NFmiGrid;

namespace SmartMetDataUtilities
{
    class Borders;
}

namespace SmartMetDataUtilities
{
    namespace AreaUtil
    {
        Borders latLonBordersForRectangleAroundArea(const NFmiArea& area);
        void normalizeArea(NFmiArea& area);
        std::unique_ptr<NFmiArea> calculateCroppedArea(NFmiGrid& grid, const NFmiArea &mapArea);
        double areaInSquareMetersFor(const NFmiArea& area);
        std::unique_ptr<NFmiArea> createCopyOf(const NFmiArea& area);
        bool isPacific(const NFmiArea& area);
        bool areasIntersect(const NFmiArea& area1, const NFmiArea& area2, size_t resolution = 100);
        std::string makeAreaString(const NFmiArea& area);

        double MaxLongitude(const NFmiArea& area, size_t resolution = 100);
        double MaxLatitude(const NFmiArea& area, size_t resolution = 100);
        double MinLongitude(const NFmiArea& area, size_t resolution = 100);
        double MinLatitude(const NFmiArea& area, size_t resolution = 100);

        template <typename Lambda>
        inline void horizontalForEach(double beginRelativeX, double endRelativeX, double relativeY, size_t resolution, Lambda&& lambda)
        {
            auto increment = (endRelativeX - beginRelativeX) / resolution;
            for(int i = 0; i < resolution - 1; ++i, beginRelativeX += increment)
            {
                lambda(NFmiPoint(beginRelativeX, relativeY));
            }
            lambda(NFmiPoint(endRelativeX, relativeY));
        }

        template <typename Lambda>
        inline void verticalForEach(double beginRelativeY, double endRelativeY, double relativeX, size_t resolution, Lambda&& lambda)
        {
            auto increment = (endRelativeY - beginRelativeY) / resolution;
            for(int j = 0; j < resolution - 1; ++j, beginRelativeY += increment)
            {
                lambda(NFmiPoint(relativeX, beginRelativeY));
            }
            lambda(NFmiPoint(relativeX, endRelativeY));
        }
    }
}
