#pragma once
#include "source/common/CommonAliases.h"

class NFmiPoint;

namespace SmartMetDataUtilities {
    namespace PointsUtil
    {
        PointPair verticalNeighbours(const NFmiPoint &point);
        PointPair horizontalNeighbours(const NFmiPoint &point);
        bool pointsAreEqual(NFmiPoint point1, NFmiPoint point2, double epsilon = 0.001);
    }
}
