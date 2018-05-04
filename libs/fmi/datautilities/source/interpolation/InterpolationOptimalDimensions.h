#pragma once
#include "source/newbaseUtil/SampleAverageDistances.h"

#include "NFmiGrid.h"
#include "NFmiArea.h"

namespace SmartMetDataUtilities
{
    class InterpolationOptimalDimensions
    {
        SampleAverageDistances sampleAverages_;
        const NFmiArea& area_;
    public:
        InterpolationOptimalDimensions(NFmiGrid& grid, const NFmiArea& area);
        ~InterpolationOptimalDimensions() = default;

        unsigned long getWidth();
        unsigned long getHeight();
    };
}
