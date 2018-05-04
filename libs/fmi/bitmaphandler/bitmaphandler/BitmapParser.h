#pragma once
#include "NFmiSatelliteImageCacheHelpers.h"

namespace BitmapHandler
{
    class BitmapParser
    {
    public:
        virtual NFmiImageHolder parse(const std::string &bitmapAsString) = 0;
    };
}
