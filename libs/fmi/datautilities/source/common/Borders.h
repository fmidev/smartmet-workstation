#pragma once

#include "NFmiPoint.h"

#include <functional>
#include <limits>
#include <cmath>

// SmartMet includes windows headers which define legacy macros named min and max
#ifdef max
#undef max
#endif

#ifdef min
#undef min
#endif

namespace SmartMetDataUtilities
{
    class Borders
    {
    public:
        double leftBorder = std::numeric_limits<double>::max();
        double rightBorder = -std::numeric_limits<double>::max();
        double bottomBorder = std::numeric_limits<double>::max();
        double topBorder = -std::numeric_limits<double>::max();

        void updateBorderValuesWith(const NFmiPoint &point);
        NFmiPoint topLeft() const;
        NFmiPoint topRight() const;
        NFmiPoint bottomLeft() const;
        NFmiPoint bottomRight() const;

        void convertToPacific();
        void convertToAtlantic();

        bool isUnmodified() const;

        static bool isPacific(const Borders& latLonBorders);
    };
}
