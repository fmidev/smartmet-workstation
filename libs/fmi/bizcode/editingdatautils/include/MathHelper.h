#pragma once

#include "NFmiGlobals.h"
#include <numeric>

namespace MathHelper
{
	double InterpolateWithTwoPoints(double x, double x1, double x2, double y1, double y2, double minY = kFloatMissing, double maxY = kFloatMissing);

	// Onko annetut luvut tarpeeksi lähellä toisiaan float/double lukujen tarkkuuden rajoissa
	template<typename T>
	bool Approximately(T value1, T value2, T usedEpsilon = std::numeric_limits<T>::epsilon() * 3)
	{
		return std::fabs(value1 - value2) <= usedEpsilon;
	}
}

