#pragma once
#include "datautilities/DataUtilitiesAdapter.h"

#include "gmock/gmock.h"
#include "gmock/gmock-generated-matchers.h"
#include "gtest/gtest.h"

#include <tuple>
#include <memory>

namespace SmartMetDataUtilities
{
    class RootData;
    class InterpolatedData;
}

class NFmiArea;
class NFmiPoint;
class NFmiGrid;
class NFmiQueryInfo;

template<typename A>
class NFmiDataMatrix;

namespace TestHelper
{
    using DataGridAreaTuple = std::tuple<std::shared_ptr<NFmiDataMatrix<NFmiPoint>>, std::shared_ptr<NFmiGrid>, std::shared_ptr<NFmiArea>>;

    bool rootDatasAreEqual(const SmartMetDataUtilities::RootData& data1, const SmartMetDataUtilities::RootData& data2);
    
    bool interpolatedDatasAreEqual(const SmartMetDataUtilities::InterpolatedData& data1, const SmartMetDataUtilities::InterpolatedData& data2);

    void checkThatAreaIsNormalized(const NFmiArea& area);

    DataGridAreaTuple getInterpolatedDataGridAreaTupleWith(NFmiArea& area, size_t x, size_t y);

    MATCHER_P(KeyPairsAreEqual, expected, "") { return (arg.getMapViewIndex() == expected.getMapViewIndex() && arg.getFileFilter() == expected.getFileFilter()); };

    template<typename A, typename B>
    bool typesMatch(A a, B b)
    {
        return std::is_same<A, B>::value;
    }
};

