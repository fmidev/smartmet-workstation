#include "source/interpolation/InterpolationOptimalDimensions.h"

namespace SmartMetDataUtilities
{
    InterpolationOptimalDimensions::InterpolationOptimalDimensions(NFmiGrid& grid, const NFmiArea& area)
        :sampleAverages_(grid)
        , area_(area)
    {
    }

    unsigned long InterpolationOptimalDimensions::getWidth()
    {
        return static_cast<unsigned long>(area_.WorldXYWidth() / sampleAverages_.getHorizontalAverage());
    }

    unsigned long InterpolationOptimalDimensions::getHeight()
    {
        return static_cast<unsigned long>(area_.WorldXYHeight() / sampleAverages_.getVerticalAverage());
    }
}