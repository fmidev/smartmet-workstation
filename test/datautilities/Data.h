#pragma once

#include "datautilities/../source/rootData/RootData.h"
#include "datautilities/../source/interpolatedData/InterpolatedData.h"
#include "datautilities/DataUtilitiesAdapter.h"

#include "FakeDataCreator.h"
#include "TestHelper.h"

#include "NFmiAreaFactory.h"

#include <string>
#include <tuple>
#include "source/dataUtil/MapViewId.h"

namespace Data
{
    // resolution to use for test data
    static const unsigned long default_x = 220;
    static const unsigned long default_y = 300;

    static const std::string atlanticLatLonStr = "latlon:-180,-90,180,90";
    static const std::string smallAtlanticLatLonStr = "latlon:-105.0,-3.5,-31.5,40.0";
    static const std::string pacificLatLonStr = "latlon:0,-90,360,90";
    static const std::string smallPacificLatLonStr = "latlon:150.0,-15.5,220.0,20.5";
    static const std::string euroStereoStr = "stereographic,10,90,60:-19.22,25,79.7,57";
    static const std::string nordicStereoStr = "stereographic,20,90,60:6,51.3,49,70.2";
    static const std::string smallEuroStereoStr = "stereographic,10,90,60:0,0,20,30";
    static const std::string smallNordicStereoStr = "stereographic,20,90,60:30.1,50.1,30.2,50.2";

    static NFmiAreaFactory::return_type atlanticLatLonArea = NFmiAreaFactory::Create(atlanticLatLonStr);
    static NFmiAreaFactory::return_type smallAtlanticLatLonArea = NFmiAreaFactory::Create(smallAtlanticLatLonStr);
    static NFmiAreaFactory::return_type pacificLatLonArea = NFmiAreaFactory::Create(pacificLatLonStr);
    static NFmiAreaFactory::return_type smallPacificLatLonArea = NFmiAreaFactory::Create(smallPacificLatLonStr);
    static NFmiAreaFactory::return_type euroStereoArea = NFmiAreaFactory::Create(euroStereoStr);
    static NFmiAreaFactory::return_type nordicStereoArea = NFmiAreaFactory::Create(nordicStereoStr);
    static NFmiAreaFactory::return_type smallEuroStereoArea = NFmiAreaFactory::Create(smallEuroStereoStr);
    static NFmiAreaFactory::return_type smallNordicStereoArea = NFmiAreaFactory::Create(smallNordicStereoStr);

    static NFmiFastQueryInfo nordicStereoInfo = FakeDataCreator::createFastQueryInfoFrom(nordicStereoStr, default_x, default_y);
    static NFmiFastQueryInfo euroStereoInfo = FakeDataCreator::createFastQueryInfoFrom(euroStereoStr, default_x, default_y);
    static NFmiFastQueryInfo atlanticLatLonInfo = FakeDataCreator::createFastQueryInfoFrom(atlanticLatLonStr, default_x, default_y);
    static NFmiFastQueryInfo smallAtlanticLatlonInfo = FakeDataCreator::createFastQueryInfoFrom(smallAtlanticLatLonStr, default_x, default_y);
    static NFmiFastQueryInfo pacificLatLonInfo = FakeDataCreator::createFastQueryInfoFrom(pacificLatLonStr, default_x, default_y);
    static NFmiFastQueryInfo smallPacificLatLonInfo = FakeDataCreator::createFastQueryInfoFrom(smallPacificLatLonStr, default_x, default_y);

    // Keys for adapter
    static const unsigned int mapWiewIndex1 = 0;
    static const std::string fileFilter1 = "testi";
    static unsigned int mapWiewIndex2 = 3;
    static const std::string fileFilter2 = "testi2";
    static const SmartMetDataUtilities::MapViewId expectedKeys1(mapWiewIndex1, fileFilter1);
    static const SmartMetDataUtilities::MapViewId expectedKeys2(mapWiewIndex2, fileFilter2);

    namespace
    {
        auto matrixGridArea1 = TestHelper::getInterpolatedDataGridAreaTupleWith(*euroStereoArea, 10, 20);
        auto matrixGridArea2 = TestHelper::getInterpolatedDataGridAreaTupleWith(*nordicStereoArea, 20, 10);
        auto matrixGridArea4 = TestHelper::getInterpolatedDataGridAreaTupleWith(*nordicStereoArea, 11, 12);
        auto matrixGridArea3 = TestHelper::getInterpolatedDataGridAreaTupleWith(*euroStereoArea, 14, 17);
        auto matrixGridArea5 = TestHelper::getInterpolatedDataGridAreaTupleWith(*atlanticLatLonArea, 22, 37);
    }

    static auto rootData1 = SmartMetDataUtilities::RootData(std::get<0>(matrixGridArea1), std::get<1>(matrixGridArea1), std::get<2>(matrixGridArea1));
    static auto rootData2 = SmartMetDataUtilities::RootData(std::get<0>(matrixGridArea2), std::get<1>(matrixGridArea2), std::get<2>(matrixGridArea2));
    static auto rootData3 = SmartMetDataUtilities::RootData(std::get<0>(matrixGridArea3), std::get<1>(matrixGridArea3), std::get<2>(matrixGridArea3));
    static auto rootData4 = SmartMetDataUtilities::RootData(std::get<0>(matrixGridArea4), std::get<1>(matrixGridArea4), std::get<2>(matrixGridArea4));
    static auto rootData5 = SmartMetDataUtilities::RootData(std::get<0>(matrixGridArea5), std::get<1>(matrixGridArea5), std::get<2>(matrixGridArea5));

    static auto interpolatedData1 = SmartMetDataUtilities::InterpolatedData(std::get<0>(matrixGridArea1), std::get<2>(matrixGridArea1));
    static auto interpolatedData2 = SmartMetDataUtilities::InterpolatedData(std::get<0>(matrixGridArea2), std::get<2>(matrixGridArea2));
    static auto interpolatedData3 = SmartMetDataUtilities::InterpolatedData(std::get<0>(matrixGridArea3), std::get<2>(matrixGridArea3));
    static auto interpolatedData4 = SmartMetDataUtilities::InterpolatedData(std::get<0>(matrixGridArea4), std::get<2>(matrixGridArea4));
    static auto interpolatedData5 = SmartMetDataUtilities::InterpolatedData(std::get<0>(matrixGridArea5), std::get<2>(matrixGridArea5));
}