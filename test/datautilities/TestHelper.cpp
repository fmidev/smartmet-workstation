#include "TestHelper.h"
#include "datautilities/DataUtilitiesAdapter.h"
#include "datautilities/../source/rootData/RootData.h"
#include "datautilities/../source/interpolatedData/InterpolatedData.h"

#include "NFmiArea.h"
#include "NFmiPoint.h"
#include "NFmiGrid.h"
#include "NFmiQueryInfo.h"

#include <memory>
#include <tuple>

using namespace SmartMetDataUtilities;
using namespace testing;
using namespace std;

namespace TestHelper
{
    bool rootDatasAreEqual(const RootData& data1, const RootData& data2)
    {
        return data1.getGrid()->XNumber() == data2.getGrid()->XNumber()
            && data1.getGrid()->YNumber() == data2.getGrid()->YNumber()
            && data1.getMatrix()->NY() == data2.getMatrix()->NY()
            && data1.getMatrix()->NX() == data2.getMatrix()->NX();
    }

    bool interpolatedDatasAreEqual(const InterpolatedData& data1, const InterpolatedData& data2)
    {
        return data1.getArea()->AreaStr() == data2.getArea()->AreaStr()
            && data1.getMatrix()->NY() == data2.getMatrix()->NY()
            && data1.getMatrix()->NX() == data2.getMatrix()->NX();
    }

    void checkThatAreaIsNormalized(const NFmiArea& area)
    {
        auto croppedAreaXY = area.XYArea();
        EXPECT_THAT(croppedAreaXY.Left(), Eq(0));
        EXPECT_THAT(croppedAreaXY.Right(), Eq(1));
        EXPECT_THAT(croppedAreaXY.Top(), Eq(0));
        EXPECT_THAT(croppedAreaXY.Bottom(), Eq(1));
    }

    DataGridAreaTuple getInterpolatedDataGridAreaTupleWith(NFmiArea& area, size_t x, size_t y)
    {
        return DataGridAreaTuple(make_shared<NFmiDataMatrix<NFmiPoint>>(x,y), make_shared<NFmiGrid>(&area, x, y), shared_ptr<NFmiArea>(area.CreateNewArea(area.BottomLeftLatLon(), area.TopRightLatLon())));
    }
}
