#include "stdafx.h"
#include "wmssupport/LegendBuffer.h"
#include <gdiplus.h>

namespace Wms
{
    LegendBuffer::LegendBuffer(const NFmiImageHolder &legend)
        :legend_{ legend }
        , width{ legend->mImage->GetWidth() }
        , height{ legend->mImage->GetHeight() }
    {
    }

    unsigned int LegendBuffer::legendWidthRelativeToOrientation() const
    {
        return orientation == Orientation::Horizontal ? width : height;
    }

    unsigned int LegendBuffer::legendHeightRelativeToOrientation() const
    {
        return orientation == Orientation::Horizontal ? height : width;
    }

    NFmiImageHolder LegendBuffer::get()
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
}
