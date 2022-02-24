#pragma once

#include "NFmiGlobals.h"

namespace MathHelper
{
	double InterpolateWithTwoPoints(double x, double x1, double x2, double y1, double y2, double minY = kFloatMissing, double maxY = kFloatMissing);
}

