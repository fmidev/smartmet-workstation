#include "ToolmasterHatchingUtils.h"

IntPoint::IntPoint() = default;

IntPoint::IntPoint(int x, int y)
:x(x)
,y(y)
{}


FloatPoint::FloatPoint() = default;

FloatPoint::FloatPoint(float x, float y)
    :x(x)
    , y(y)
{}

BoundingBox::BoundingBox() = default;

TMWorldLimits::TMWorldLimits() = default;

float TMWorldLimits::width() const
{
    return x_max - x_min;
}

float TMWorldLimits::height() const
{
    return y_max - y_min;
}

