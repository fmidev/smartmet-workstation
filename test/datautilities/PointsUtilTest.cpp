#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "datautilities/../source/newbaseUtil/PointsUtil.h"
#include "NFmiPoint.h"

#include <algorithm>

using namespace testing;
using namespace std;
using namespace SmartMetDataUtilities;

class PointsUtilTest : public Test
{
public:
    NFmiPoint start = NFmiPoint(0, 0);
    NFmiPoint rightNeighbour = NFmiPoint(1, 0);
    NFmiPoint leftNeighbour = NFmiPoint(-1, 0);
    NFmiPoint upperNeighbour = NFmiPoint(0, 1);
    NFmiPoint lowerNeighbour = NFmiPoint(0, -1);
};

TEST_F(PointsUtilTest, VerticalNeighboursContainsVerticalNeighbours)
{
    auto vertical = PointsUtil::verticalNeighbours(start);

    EXPECT_THAT(vertical.first, Eq(upperNeighbour));
    EXPECT_THAT(vertical.second, Eq(lowerNeighbour));
}

TEST_F(PointsUtilTest, HorizontalNeighboursContainsHorizontalNeighbours)
{
    auto horizontal = PointsUtil::horizontalNeighbours(start);

    EXPECT_THAT(horizontal.first, Eq(leftNeighbour));
    EXPECT_THAT(horizontal.second, Eq(rightNeighbour));
}