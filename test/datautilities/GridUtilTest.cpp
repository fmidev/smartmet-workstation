#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "datautilities/../source/newbaseUtil/GridUtil.h"
#include "GridMock.h"

using namespace testing;
using namespace std;
using namespace SmartMetDataUtilities;
using namespace SmartMetDataUtilitiesTest;

class GridUtilTest : public Test
{
public:
    GridMock gridMock;
    NFmiPoint point1 = NFmiPoint(10, 98);
    NFmiPoint point2 = NFmiPoint(14, 42);
};

TEST_F(GridUtilTest, SetGridToPointCallsCorrectlyGridsGridPointMethod)
{
    NFmiPoint point(10, 98);
    EXPECT_CALL(gridMock, GridPoint(point.X(), point.Y()));

    GridUtil::setGridToPoint(gridMock, point);
}

TEST_F(GridUtilTest, SurfaceDistanceOfGridPointReturnsRightResult)
{
    auto correctSurfaceDistance = NFmiLocation(point1).Distance(NFmiLocation(point2));

    EXPECT_CALL(gridMock, GridToLatLon(point1))
        .WillOnce(Return(point1));
    EXPECT_CALL(gridMock, GridToLatLon(point2))
        .WillOnce(Return(point2));

    EXPECT_THAT(GridUtil::surfaceDistanceOfPoints(gridMock, point1, point2), DoubleEq(correctSurfaceDistance));
}

TEST_F(GridUtilTest, PointToSurfaceReturnsCorrectNFmiLocation)
{
    EXPECT_CALL(gridMock, GridToLatLon(point1))
        .WillOnce(Return(point1));

    EXPECT_THAT(GridUtil::mapPointToSurface(gridMock, point1), Eq(NFmiLocation(point1)));
}
