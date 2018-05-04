#pragma once
#include "source/common/CommonAliases.h"

#include "NFmiGrid.h"

#include <vector>

namespace SmartMetDataUtilities
{
    class SampleAverageDistances
    {
        NFmiGrid& grid_;
        SamplePoints sample_;
        std::vector<double> horizontalDistances_;
        std::vector<double> verticalDistances_;
    public:
        SampleAverageDistances(NFmiGrid& grid);

        double getHorizontalAverage();
        double getVerticalAverage();
    private:
        void calculateAverageDistancesFor();
    };
}