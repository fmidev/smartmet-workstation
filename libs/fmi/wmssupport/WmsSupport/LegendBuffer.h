#pragma once

#include "NFmiSatelliteImageCacheHelpers.h"

#include <gdiplus.h>

#include <memory>

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

        LegendBuffer(NFmiImageHolder legend)
            :legend_{ legend }
            , width{ legend->mImage->GetWidth() }
            , height{ legend->mImage->GetHeight() }
        {
        }

        unsigned int legendWidthRelativeToOrientation() const
        {
            return orientation == Orientation::Horizontal ? width : height;
        }

        unsigned int legendHeightRelativeToOrientation() const
        {
            return orientation == Orientation::Horizontal ? height : width;
        }

        NFmiImageHolder get()
        {
            auto scaledLegend = std::make_shared<Gdiplus::Bitmap>(width, height);
            Gdiplus::Graphics graphics(scaledLegend.get());

            auto horizontalScalingFactor = ((double)width) / (double)legend_->mImage->GetWidth();
            auto verticalScalingFactor = ((double)height) / (double)legend_->mImage->GetHeight();

            graphics.ScaleTransform(static_cast<Gdiplus::REAL>(horizontalScalingFactor), static_cast<Gdiplus::REAL>(verticalScalingFactor));
            graphics.DrawImage(legend_->mImage.get(), 0, 0);
            legend_->mImage = scaledLegend;
            return legend_;
        }
    };
}