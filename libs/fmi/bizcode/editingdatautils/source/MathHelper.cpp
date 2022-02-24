#include "MathHelper.h"

namespace MathHelper
{
	// suoran kaksi pistettä on annettu ja x:n arvo, laske y:n arvo
	double InterpolateWithTwoPoints(double x, double x1, double x2, double y1, double y2, double minY, double maxY)
	{
		double k = (y2-y1)/(x2-x1);
		double b = (x1*y2 - y1*x2)/(x1-x2);
		double y = k*x + b;
		if(minY != kFloatMissing)
			y = std::max(y, minY);
		if(maxY != kFloatMissing)
			y = std::min(y, maxY);
		return y;
	}

} // namespace MathHelper
