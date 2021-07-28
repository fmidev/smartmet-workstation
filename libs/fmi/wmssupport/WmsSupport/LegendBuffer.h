#pragma once

#include "NFmiSatelliteImageCacheHelpers.h"

namespace Wms
{
    enum class Orientation
    {
        Horizontal,
        Vertical
    };

    class LegendBuffer
    {
        NFmiImageHolder legend_;
    public:
        unsigned int width;
        unsigned int height;
        Orientation orientation;

        LegendBuffer(const NFmiImageHolder &legend);

        unsigned int legendWidthRelativeToOrientation() const;
        unsigned int legendHeightRelativeToOrientation() const;
        NFmiImageHolder get();
    };
}
