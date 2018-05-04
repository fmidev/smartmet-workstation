#pragma once
#include "NFmiPoint.h"
#include "gmock/gmock.h"
namespace SmartMetDataUtilitiesTest {
    class GridMock {
    public:
        MOCK_CONST_METHOD0(XNumber, unsigned long());
        MOCK_CONST_METHOD0(YNumber, unsigned long());
        MOCK_CONST_METHOD0(LatLon, const NFmiPoint());
        MOCK_METHOD2(GridPoint, bool(double x, double y));
        MOCK_CONST_METHOD1(IsInside, bool(const NFmiPoint & theLatLon));
        MOCK_CONST_METHOD1(GridToLatLon, const NFmiPoint(const NFmiPoint & theGridPoint));
        MOCK_METHOD0(Right, bool());
        MOCK_METHOD0(Up, bool());
    };
}