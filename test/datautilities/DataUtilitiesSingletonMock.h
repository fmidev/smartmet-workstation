#pragma once
#include "gmock/gmock.h"

#include "datautilities/../source/dataUtil/DataUtilitiesSingleton.h"

#include "NFmiGrid.h"
#include "NFmiDataMatrix.h"
#include "NFmiPoint.h"
#include "NFmiFastQueryInfo.h"
#include "NFmiArea.h"

#include <memory>

class DataUtilitiesSingletonMock
{
public:
    static DataUtilitiesSingletonMock& instance()
    {
        static DataUtilitiesSingletonMock singleton;
        return singleton;
    }

    MOCK_METHOD3(initializeDrawingData, void(const NFmiFastQueryInfo& data, const NFmiArea& mapArea, const SmartMetDataUtilities::MapViewId& keys));
    MOCK_CONST_METHOD1(isModifiedDataDrawingPossible, bool(const SmartMetDataUtilities::MapViewId& keys));
    MOCK_CONST_METHOD1(isThereAnythingToDraw, bool(const SmartMetDataUtilities::MapViewId& keys));
    MOCK_CONST_METHOD0(getCroppedArea, std::shared_ptr<NFmiArea>());
    MOCK_CONST_METHOD0(getInterpolatedData, std::shared_ptr<NFmiDataMatrix<NFmiPoint>>());
    MOCK_CONST_METHOD1(getRootGrid, std::shared_ptr<NFmiGrid>(const SmartMetDataUtilities::MapViewId& keys));
};