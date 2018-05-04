#include "source/common/CommonAliases.h"
#include "source/newbaseUtil/PointsUtil.h"

#include "NFmiPoint.h"

#include <utility>

using namespace std;

namespace SmartMetDataUtilities {
    namespace PointsUtil
    {
        namespace
        {
            enum class Direction
            {
                LEFT,
                RIGHT,
                UP,
                DOWN
            };

            NFmiPoint getNeighbourInDirection(const NFmiPoint &point, const Direction &direction)
            {
                auto x = point.X();
                auto y = point.Y();
                switch(direction)
                {
                case Direction::DOWN:
                    return NFmiPoint(x, y - 1.0);
                case Direction::UP:
                    return NFmiPoint(x, y + 1.0);
                case Direction::LEFT:
                    return NFmiPoint(x - 1.0, y);
                case Direction::RIGHT:
                    return NFmiPoint(x + 1.0, y);
                default:
                    return NFmiPoint(0, 0);
                }
            }
        }

        PointPair verticalNeighbours(const NFmiPoint &point)
        {
            return PointPair(
                getNeighbourInDirection(point, Direction::UP),
                getNeighbourInDirection(point, Direction::DOWN)
            );
        }

        PointPair horizontalNeighbours(const NFmiPoint &point)
        {
            return PointPair(
                getNeighbourInDirection(point, Direction::LEFT),
                getNeighbourInDirection(point, Direction::RIGHT)
            );
        }

        bool pointsAreEqual(NFmiPoint point1, NFmiPoint point2, double epsilon)
        {
            return abs(point1.X() - point2.X()) < epsilon
            && abs(point1.Y() - point2.Y()) <epsilon;
        }
    }
}