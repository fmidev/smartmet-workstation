#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "gmock/gmock-actions.h"

#include "datautilities/DataUtilitiesAdapter.h"
#include "datautilities/../source/error/Error.h"
#include "datautilities/../source/newbaseUtil/PointsUtil.h"
#include "datautilities/../source/newbaseUtil/AreaUtil.h"

#include "ReferenceAreaCreator.h"
#include "DataUtilitiesSingletonMock.h"
#include "Data.h"
#include "TestHelper.h"

#include "NFmiFastQueryInfo.h"
#include "NFmiQueryDataUtil.h"

#include <chrono>

using namespace testing;
using namespace std;
using namespace SmartMetDataUtilities;
using namespace TestHelper;
using namespace Data;
using milliseconds = chrono::milliseconds;

class DataUtilitiesAdapterTest : public Test
{
public:
};

TEST_F(DataUtilitiesAdapterTest, takesToolmasterStatusMapWiewIndexAndFileFilterInConstructor)
{
    EXPECT_NO_THROW(DataUtilitiesAdapter<> adapter(Toolmaster::Available, 2, "file/at/somewhere"));
    EXPECT_NO_THROW(DataUtilitiesAdapter<> adapter(Toolmaster::Available, 0, "file-is-this-format.sqd"));
}

TEST_F(DataUtilitiesAdapterTest, toolmasterStatusMapWiewIndexAndFileFilterCanBeInjectedThroughSetters)
{
    DataUtilitiesAdapter<> adapter;
    EXPECT_NO_THROW(adapter.setToolmasterAvailability(Toolmaster::Available));
    EXPECT_NO_THROW(adapter.setMapViewIndex(2));
    EXPECT_NO_THROW(adapter.setFileFilter("default"));
}

TEST_F(DataUtilitiesAdapterTest, DataUtilitiesSingletonIsInjectedThroughTemplateArgument)
{
    EXPECT_NO_THROW(DataUtilitiesAdapter<DataUtilitiesSingletonMock>adapter(Toolmaster::Available, 0, "default"));
}

TEST_F(DataUtilitiesAdapterTest, InjectedDataUtilitiesSingletonDefaultsToDataUtilities)
{
    DataUtilitiesAdapter<> adapterImplicit(Toolmaster::Available, 0, "default");
    DataUtilitiesAdapter<DataUtilitiesSingleton> adapterExcplicit(Toolmaster::Available, 1, "default");

    EXPECT_TRUE(typesMatch(adapterImplicit, adapterExcplicit));
}

TEST_F(DataUtilitiesAdapterTest, forwardsIdAndFileFilterToDataUtilitiesInAllNecessaryCalls)
{
    DataUtilitiesAdapter<DataUtilitiesSingletonMock> adapterWithMock(Toolmaster::Available, mapWiewIndex1, fileFilter1);

    EXPECT_CALL(DataUtilitiesSingletonMock::instance(), initializeDrawingData(_, _, KeyPairsAreEqual(expectedKeys1)));
    EXPECT_CALL(DataUtilitiesSingletonMock::instance(), isModifiedDataDrawingPossible(KeyPairsAreEqual(expectedKeys1)))
        .WillOnce(Return(true));
    EXPECT_CALL(DataUtilitiesSingletonMock::instance(), isThereAnythingToDraw(KeyPairsAreEqual(expectedKeys1)))
        .WillOnce(Return(true));
    EXPECT_CALL(DataUtilitiesSingletonMock::instance(), getRootGrid(KeyPairsAreEqual(expectedKeys1)))
        .WillOnce(Return(nullptr));

    adapterWithMock.initializeDrawingData(atlanticLatLonInfo, *euroStereoArea);
    adapterWithMock.isModifiedDataDrawingPossible();
    adapterWithMock.isThereAnythingToDraw();
    adapterWithMock.getRootGrid();
}

TEST_F(DataUtilitiesAdapterTest, adaptersWithDifferentKeysForwardDifferentKeys)
{
    DataUtilitiesAdapter<DataUtilitiesSingletonMock> adapterWithMock1(Toolmaster::Available, mapWiewIndex1, fileFilter1);
    DataUtilitiesAdapter<DataUtilitiesSingletonMock> adapterWithMock2(Toolmaster::Available, mapWiewIndex2, fileFilter2);

    EXPECT_CALL(DataUtilitiesSingletonMock::instance(), initializeDrawingData(_, _, KeyPairsAreEqual(expectedKeys1)));
    adapterWithMock1.initializeDrawingData(atlanticLatLonInfo, *euroStereoArea);

    Mock::VerifyAndClearExpectations(&DataUtilitiesSingletonMock::instance());

    EXPECT_CALL(DataUtilitiesSingletonMock::instance(), initializeDrawingData(_, _, KeyPairsAreEqual(expectedKeys2)));
    adapterWithMock2.initializeDrawingData(atlanticLatLonInfo, *euroStereoArea);
}

class DataUtilitiesTest : public Test
{
public:
    DataUtilitiesAdapter<> adapter;

    DataUtilitiesTest()
        :adapter(Toolmaster::Available, 0, "default")
    {
    }

    ~DataUtilitiesTest()
    {
        adapter.clear();
    }
};

class DataUtilitiesIsModifiedDataDrawingPossible : public DataUtilitiesTest
{
public:
};

TEST_F(DataUtilitiesIsModifiedDataDrawingPossible, returnsFalseIfAreasProjectionsAreEqual)
{
    adapter.initializeDrawingData(nordicStereoInfo, *nordicStereoArea);
    EXPECT_THAT(adapter.isModifiedDataDrawingPossible(), Eq(false));

    adapter.initializeDrawingData(euroStereoInfo, *euroStereoArea);
    EXPECT_THAT(adapter.isModifiedDataDrawingPossible(), Eq(false));

    adapter.initializeDrawingData(atlanticLatLonInfo, *atlanticLatLonArea);
    EXPECT_THAT(adapter.isModifiedDataDrawingPossible(), Eq(false));

    adapter.initializeDrawingData(pacificLatLonInfo, *pacificLatLonArea);
    EXPECT_THAT(adapter.isModifiedDataDrawingPossible(), Eq(false));

    adapter.initializeDrawingData(pacificLatLonInfo, *smallPacificLatLonArea);
    EXPECT_THAT(adapter.isModifiedDataDrawingPossible(), Eq(false));
}

TEST_F(DataUtilitiesIsModifiedDataDrawingPossible, returnsFalseIfToolmasterIsNotAvailable)
{
    DataUtilitiesAdapter<> adapterWithoutToolMaster(Toolmaster::NotAvailable, 0, "default");
    adapterWithoutToolMaster.initializeDrawingData(atlanticLatLonInfo, *euroStereoArea);
    EXPECT_THAT(adapterWithoutToolMaster.isModifiedDataDrawingPossible(), Eq(false));
}

TEST_F(DataUtilitiesIsModifiedDataDrawingPossible, returnsFalseIfDataIsNotGrid)
{
    auto data = FakeDataCreator::createFastQueryInfoFrom(euroStereoStr, 0, 0);
    ASSERT_THAT(data.IsGrid(), Eq(false));

    adapter.initializeDrawingData(data, *atlanticLatLonArea);

    EXPECT_THAT(adapter.isModifiedDataDrawingPossible(), Eq(false));
}

TEST_F(DataUtilitiesIsModifiedDataDrawingPossible, returnsTrueIfAreasProjectionsAreNotEqual)
{
    adapter.initializeDrawingData(nordicStereoInfo, *euroStereoArea);
    EXPECT_THAT(adapter.isModifiedDataDrawingPossible(), Eq(true));
}

TEST_F(DataUtilitiesIsModifiedDataDrawingPossible, returnsTrueIfProjectionsAreEqualButButOneIsPacifcAndOtherAtlantic)
{
    adapter.initializeDrawingData(atlanticLatLonInfo, *pacificLatLonArea);
    EXPECT_THAT(adapter.isModifiedDataDrawingPossible(), Eq(true));
}

TEST_F(DataUtilitiesIsModifiedDataDrawingPossible, returnsTrueIfProjectionsAreEqualButCentralLongitudeDiffers)
{
    adapter.initializeDrawingData(atlanticLatLonInfo, *euroStereoArea);
    EXPECT_THAT(adapter.isModifiedDataDrawingPossible(), Eq(true));
}

class DataUtilitiesIsThereAnythingToDraw : public DataUtilitiesTest
{
public:
};

TEST_F(DataUtilitiesIsThereAnythingToDraw, returnsFalseIfDataAreaIsOutsideMapArea)
{
    auto& nordicInfoArea = *nordicStereoInfo.Area();
    ASSERT_THAT(AreaUtil::MaxLatitude(*smallEuroStereoArea), Lt(AreaUtil::MinLatitude(nordicInfoArea)));

    adapter.initializeDrawingData(nordicStereoInfo, *smallEuroStereoArea);

    EXPECT_THAT(adapter.isThereAnythingToDraw(), Eq(false));
}

TEST_F(DataUtilitiesIsThereAnythingToDraw, returnsFalseIfToolmasterNotAvailable)
{
    DataUtilitiesAdapter<> adapterWithoutToolmaster(Toolmaster::NotAvailable, 0, "default");
    adapterWithoutToolmaster.initializeDrawingData(nordicStereoInfo, *euroStereoArea);

    EXPECT_THAT(adapterWithoutToolmaster.isThereAnythingToDraw(), Eq(false));
}

TEST_F(DataUtilitiesIsThereAnythingToDraw, returnsFalseIfDataIsEmpty)
{
    auto areaSoSmallThatNotOneGridPointFitsInside = NFmiAreaFactory::Create("stereographic,20,90,60:30.1,50.1,30.1,50.1");
    adapter.initializeDrawingData(atlanticLatLonInfo, *areaSoSmallThatNotOneGridPointFitsInside);

    EXPECT_THAT(adapter.isThereAnythingToDraw(), Eq(false));
}

class DataUtilitiesRootCropping : public DataUtilitiesTest
{
public:
};

TEST_F(DataUtilitiesRootCropping, rootGridIsNullptrBeforeFirstCallToInitialize)
{
    ASSERT_THAT(adapter.getRootGrid(), Eq(nullptr));
}

TEST_F(DataUtilitiesRootCropping, firstInitializeDrawinDataCallCreatesRootGrid)
{
    ASSERT_THAT(adapter.getRootGrid(), Eq(nullptr));

    adapter.initializeDrawingData(nordicStereoInfo, *euroStereoArea);

    EXPECT_THAT(adapter.getRootGrid(), Not(Eq(nullptr)));
}

TEST_F(DataUtilitiesRootCropping, rootGridIsNotUpdatedifMapAreaIsInsideCurrentRootArea)
{
    adapter.initializeDrawingData(atlanticLatLonInfo, *euroStereoArea);
    auto rootGrid = adapter.getRootGrid();
    auto smallerArea = NFmiAreaFactory::Create("stereographic,10,90,60:10,35,60,52");
    EXPECT_THAT(rootGrid->Area()->IsInside(*smallerArea), Eq(true));

    adapter.initializeDrawingData(atlanticLatLonInfo, *smallerArea);
    EXPECT_THAT(NFmiQueryDataUtil::AreAreasEqual(rootGrid->Area(), adapter.getRootGrid()->Area()), Eq(true));
}

TEST_F(DataUtilitiesRootCropping, rootGridIsNotUpdatedifNewCroppedAreaIsSmaller)
{
    auto originalArea = NFmiAreaFactory::Create("stereographic,10,90,60:11,20,70,50");
    auto areaOutsideOriginalArea = NFmiAreaFactory::Create("stereographic,10,90,60:-10,20,20,50");
    auto croppedReferenceArea = ReferenceAreaCreator::getReferenceAreaWith(atlanticLatLonInfo, *areaOutsideOriginalArea);
    adapter.initializeDrawingData(atlanticLatLonInfo, *originalArea);
    auto rootGrid = adapter.getRootGrid();
    ASSERT_THAT(rootGrid->Area()->IsInside(*croppedReferenceArea), Eq(false));
    ASSERT_THAT(AreaUtil::areaInSquareMetersFor(*rootGrid->Area()), Gt(AreaUtil::areaInSquareMetersFor(*croppedReferenceArea)));

    adapter.initializeDrawingData(atlanticLatLonInfo, *areaOutsideOriginalArea);

    EXPECT_THAT(NFmiQueryDataUtil::AreAreasEqual(rootGrid->Area(), adapter.getRootGrid()->Area()), Eq(true));
}

TEST_F(DataUtilitiesRootCropping, rootGridIsUpdatedifNewCroppedAreaIsLargerThanRootArea)
{
    auto originalArea = NFmiAreaFactory::Create("stereographic,10,90,60:11,50,15,20");
    auto areaOutsideOriginalArea = NFmiAreaFactory::Create("stereographic,10,90,60:-25,50,10,20");
    auto croppedReferenceArea = ReferenceAreaCreator::getReferenceAreaWith(atlanticLatLonInfo, *areaOutsideOriginalArea);
    adapter.initializeDrawingData(atlanticLatLonInfo, *originalArea);
    auto rootGrid = adapter.getRootGrid();
    ASSERT_THAT(rootGrid->Area()->IsInside(*croppedReferenceArea), Eq(false));
    ASSERT_THAT(AreaUtil::areaInSquareMetersFor(*rootGrid->Area()), Lt(AreaUtil::areaInSquareMetersFor(*croppedReferenceArea)));

    adapter.initializeDrawingData(atlanticLatLonInfo, *areaOutsideOriginalArea);

    EXPECT_THAT(NFmiQueryDataUtil::AreAreasEqual(rootGrid->Area(), adapter.getRootGrid()->Area()), Eq(false));
}

TEST_F(DataUtilitiesRootCropping, rootGridIsNotUpdatedIfDataAndMapAreaDoNotIntersect)
{
    auto smallEuroArea = NFmiAreaFactory::Create("stereographic,10,90,60:0,0,20,30");
    auto nordicDatasArea = nordicStereoInfo.Area();
    ASSERT_THAT(AreaUtil::MaxLatitude(*smallEuroArea), Lt(AreaUtil::MinLatitude(*nordicDatasArea)));

    adapter.initializeDrawingData(nordicStereoInfo, *smallEuroArea);

    EXPECT_THAT(adapter.getRootGrid(), Eq(nullptr));
}

TEST_F(DataUtilitiesRootCropping, croppedAreaIsNormalized)
{
    adapter.initializeDrawingData(atlanticLatLonInfo, *euroStereoArea);
    auto rootGrid = adapter.getRootGrid();
    auto smallerArea = NFmiAreaFactory::Create("stereographic,10,90,60:10,35,60,52");
    EXPECT_THAT(rootGrid->Area()->IsInside(*smallerArea), Eq(true));

    adapter.initializeDrawingData(atlanticLatLonInfo, *smallerArea);

    checkThatAreaIsNormalized(*adapter.getCroppedArea());
}

TEST_F(DataUtilitiesRootCropping, rootAreaIsNormalized)
{
    adapter.initializeDrawingData(atlanticLatLonInfo, *euroStereoArea);
    auto rootGrid = adapter.getRootGrid();
    checkThatAreaIsNormalized(*rootGrid->Area());
}

TEST_F(DataUtilitiesRootCropping, croppedDatasAreaMatchesCroppedArea)
{
    adapter.initializeDrawingData(atlanticLatLonInfo, *euroStereoArea);
    auto rootGrid = adapter.getRootGrid();
    auto smallerArea = NFmiAreaFactory::Create("stereographic,10,90,60:10,35,40,52");
    EXPECT_THAT(rootGrid->Area()->IsInside(*smallerArea), Eq(true));
    adapter.initializeDrawingData(atlanticLatLonInfo, *smallerArea);
    auto croppedData = adapter.getInterpolatedData();
    decltype(auto) croppedArea = adapter.getCroppedArea();

    EXPECT_TRUE(PointsUtil::pointsAreEqual((*croppedData)[0][0], croppedArea->BottomLeftLatLon()));
    EXPECT_TRUE(PointsUtil::pointsAreEqual((*croppedData)[croppedData->NX() - 1][croppedData->NY() - 1], croppedArea->TopRightLatLon()));
}

TEST_F(DataUtilitiesRootCropping, dataIsCroppedAroundMapAreaByTwoPixelsMax)
{
    adapter.initializeDrawingData(atlanticLatLonInfo, *euroStereoArea);
    auto rootGrid = adapter.getRootGrid();
    auto smallerArea = NFmiAreaFactory::Create("stereographic,10,90,60:15,35,60,52");
    EXPECT_THAT(rootGrid->Area()->IsInside(*smallerArea), Eq(true));
    adapter.initializeDrawingData(atlanticLatLonInfo, *smallerArea);

    auto croppedData = adapter.getInterpolatedData();
    auto height = croppedData->NY();
    auto width = croppedData->NX();
    auto insideFactor = 2;
    for(size_t j = 0; j != height; ++j)
    {
        auto leftBorder = (*croppedData)[0][j];
        auto rightBorder = (*croppedData)[width - 1][j];
        ASSERT_THAT(smallerArea->IsInside(leftBorder), Eq(false));
        ASSERT_THAT(smallerArea->IsInside(rightBorder), Eq(false));
        if(j >= insideFactor && j <= height - 1 - insideFactor)
        {
            auto insideLeftBorder = (*croppedData)[insideFactor][j];
            auto insideRightBorder = (*croppedData)[width - 1 - insideFactor][j];
            ASSERT_THAT(smallerArea->IsInside(insideLeftBorder), Eq(true));
            ASSERT_THAT(smallerArea->IsInside(insideRightBorder), Eq(true));
        }
    }
    for(size_t i = 0; i != width; ++i)
    {
        auto bottomBorder = (*croppedData)[i][0];
        auto topBorder = (*croppedData)[i][height - 1];
        ASSERT_THAT(smallerArea->IsInside(bottomBorder), Eq(false));
        ASSERT_THAT(smallerArea->IsInside(topBorder), Eq(false));
        if(i >= insideFactor && i <= width - 1 - insideFactor)
        {
            auto insideBottomBorder = (*croppedData)[i][insideFactor];
            auto insideTopBorder = (*croppedData)[i][height - 1 - insideFactor];
            ASSERT_THAT(smallerArea->IsInside(insideBottomBorder), Eq(true));
            ASSERT_THAT(smallerArea->IsInside(insideTopBorder), Eq(true));
        }
    }
}

TEST_F(DataUtilitiesRootCropping, ifMapAreaFitsInsideAreaThatWasStoredWhenRootDataWasCreatedDoNotUpdateRootData)
{
    adapter.initializeDrawingData(nordicStereoInfo, *euroStereoArea);
    auto rootGridBefore = adapter.getRootGrid();
    auto areaStr = rootGridBefore->Area()->AreaStr();
    auto smallerArea = NFmiAreaFactory::Create("stereographic,10,90,60:-10,38,66,69");
    auto referenceArea = ReferenceAreaCreator::getReferenceAreaWith(nordicStereoInfo, *smallerArea);
    ASSERT_THAT(euroStereoArea->IsInside(*smallerArea), Eq(true));
    ASSERT_THAT(AreaUtil::areaInSquareMetersFor(*referenceArea), Ge(AreaUtil::areaInSquareMetersFor(*rootGridBefore->Area())));

    adapter.initializeDrawingData(nordicStereoInfo, *smallerArea);
    auto rootGridAfter = adapter.getRootGrid();

    EXPECT_THAT(rootGridBefore, Eq(rootGridAfter));
}

TEST_F(DataUtilitiesRootCropping, ifMapAreaFitsInsideAreaThatWasStoredWhenRootDataWasCreatedAndRootDatasAreaFitsInsideMapAreaInterpolateUsingRootDataWithoutCropping)
{
    adapter.initializeDrawingData(nordicStereoInfo, *euroStereoArea);
    auto rootGridBefore = adapter.getRootGrid();
    auto smallerArea = NFmiAreaFactory::Create("stereographic,10,90,60:-10,49,56.4136,67.7409");
    ASSERT_THAT(euroStereoArea->IsInside(*smallerArea), Eq(true));
    ASSERT_THAT(smallerArea->IsInside(*rootGridBefore->Area()), Eq(true));

    adapter.initializeDrawingData(nordicStereoInfo, *smallerArea);
    auto croppedArea = adapter.getCroppedArea();
    auto rootGridAfter = adapter.getRootGrid();

    EXPECT_THAT(AreaUtil::areAreasEqual(rootGridBefore->Area(), croppedArea.get()), Eq(true));
    EXPECT_THAT(rootGridBefore, Eq(rootGridAfter));
}

TEST_F(DataUtilitiesRootCropping, ifMapAreaFitsInsideAreaThatWasStoredWhenRootDataWasCreatedAndRootDatasAreaDoesNotFitInsideMapAreaInterpolateUsingRootDataWithCropping)
{
    adapter.initializeDrawingData(nordicStereoInfo, *euroStereoArea);
    auto rootGridBefore = adapter.getRootGrid();
    auto smallerArea = NFmiAreaFactory::Create("stereographic,10,90,60:-20,35,60,52");
    ASSERT_THAT(euroStereoArea->IsInside(*smallerArea), Eq(true));
    ASSERT_THAT(smallerArea->IsInside(*rootGridBefore->Area()), Eq(false));

    adapter.initializeDrawingData(nordicStereoInfo, *smallerArea);
    auto croppedArea = adapter.getCroppedArea();
    auto rootGridAfter = adapter.getRootGrid();

    EXPECT_THAT(rootGridBefore->Area()->IsInside(*croppedArea), Eq(true));
    EXPECT_THAT(rootGridBefore, Eq(rootGridAfter));
}

class DataUtilitiesCaching : public DataUtilitiesTest
{
public:
};

TEST_F(DataUtilitiesCaching, whenInitializeIsCalledWithExactlyTheSameParametersMoreThanOnceTheEarlierInterpolationIsUsed)
{
    adapter.initializeDrawingData(atlanticLatLonInfo, *euroStereoArea);
    auto firstData = adapter.getInterpolatedData();
    auto firstArea = adapter.getCroppedArea();

    adapter.initializeDrawingData(atlanticLatLonInfo, *euroStereoArea);
    auto secondData = adapter.getInterpolatedData();
    auto secondArea = adapter.getCroppedArea();

    EXPECT_THAT(firstData, Eq(secondData));
    EXPECT_THAT(firstArea, Eq(secondArea));
}

TEST_F(DataUtilitiesCaching, ifDatasGridSizeHasChangedOldInterpolationCantBeUsed)
{
    adapter.initializeDrawingData(atlanticLatLonInfo, *euroStereoArea);
    auto firstData = adapter.getInterpolatedData();
    auto firstArea = adapter.getCroppedArea();

    auto smallerGridWorldData = FakeDataCreator::createFastQueryInfoFrom(atlanticLatLonStr, default_x - 10, default_y - 10);
    adapter.initializeDrawingData(smallerGridWorldData, *euroStereoArea);
    auto secondData = adapter.getInterpolatedData();
    auto secondArea = adapter.getCroppedArea();

    EXPECT_THAT(firstData, Not(Eq(secondData)));
    EXPECT_THAT(firstArea, Not(Eq(secondArea)));
}

class DataUtilitiesRootCaching : public DataUtilitiesCaching
{
public:
};

TEST_F(DataUtilitiesRootCaching, ifDatasGridSizeHasChangedOldRootDataCantBeUsed)
{
    adapter.initializeDrawingData(atlanticLatLonInfo, *nordicStereoArea);
    auto rootGridBefore = adapter.getRootGrid();

    auto smallerGridWorldData = FakeDataCreator::createFastQueryInfoFrom(atlanticLatLonStr, default_x - 10, default_y - 10);
    adapter.initializeDrawingData(smallerGridWorldData, *smallNordicStereoArea);
    auto rootGridAfter = adapter.getRootGrid();

    EXPECT_THAT(rootGridAfter, Not(Eq(rootGridBefore)));
}

class DataUtilitiesAreaCropperLimits : public DataUtilitiesTest
{
public:
    milliseconds referenceDurationForEuroToNordicInterpolation = interpolationFromEuroToNordicReferenceDuration();
    milliseconds referenceDurationForNordicToEuroInterpolation = interpolationFromNordicToEuroReferenceDuration();

    chrono::milliseconds interpolationFromEuroToNordicReferenceDuration()
    {
        auto before = chrono::system_clock::now();
        adapter.initializeDrawingData(euroStereoInfo, *nordicStereoArea);
        auto after = chrono::system_clock::now();

        return chrono::duration_cast<chrono::milliseconds>(after - before);
    }

    chrono::milliseconds interpolationFromNordicToEuroReferenceDuration()
    {
        auto before = chrono::system_clock::now();
        adapter.initializeDrawingData(nordicStereoInfo, *euroStereoArea);
        auto after = chrono::system_clock::now();

        return chrono::duration_cast<chrono::milliseconds>(after - before);
    }

    auto durationOfInitializationFor(const NFmiFastQueryInfo& data, const NFmiArea& area)
    {
        auto before = chrono::system_clock::now();
        adapter.initializeDrawingData(data, area);
        auto after = chrono::system_clock::now();

        return chrono::duration_cast<chrono::milliseconds>(after - before);
    }
};

TEST_F(DataUtilitiesAreaCropperLimits, limitsAreChosenForLatlonToNordicConversionSoThatInterpolationIsFast)
{
    milliseconds maxDuration = referenceDurationForEuroToNordicInterpolation == 0ms
        ? 2ms
        : referenceDurationForEuroToNordicInterpolation * 2;

    auto durationMS = durationOfInitializationFor(atlanticLatLonInfo, *nordicStereoArea);
    EXPECT_THAT(durationMS.count(), Le(maxDuration.count()));
}

TEST_F(DataUtilitiesAreaCropperLimits, limitsAreChosenForLatlonToEuroConversionSoThatInterpolationIsFast)
{
    milliseconds maxDuration = referenceDurationForEuroToNordicInterpolation == 0ms
        ? 2ms
        : referenceDurationForEuroToNordicInterpolation * 2;

    auto durationMS = durationOfInitializationFor(atlanticLatLonInfo, *euroStereoArea);
    EXPECT_THAT(durationMS.count(), Le(maxDuration.count()));
}

TEST_F(DataUtilitiesAreaCropperLimits, limitsAreChosenForNordicToEuroConversionSoThatInterpolationIsFast)
{
    milliseconds maxDuration = referenceDurationForEuroToNordicInterpolation == 0ms
        ? 2ms
        : referenceDurationForEuroToNordicInterpolation * 5;

    auto durationMS = durationOfInitializationFor(nordicStereoInfo, *euroStereoArea);
    EXPECT_THAT(durationMS.count(), Le(maxDuration.count()));
}

TEST_F(DataUtilitiesAreaCropperLimits, limitsAreChosenForEuroToNordicConversionSoThatInterpolationIsFast)
{
    milliseconds maxDuration = referenceDurationForNordicToEuroInterpolation == 0ms
        ? 2ms
        : referenceDurationForNordicToEuroInterpolation * 2;

    auto durationMS = durationOfInitializationFor(euroStereoInfo, *nordicStereoArea);
    EXPECT_THAT(durationMS.count(), Le(maxDuration.count()));
}

class DataUtilitiesGetCroppedArea : public DataUtilitiesTest
{
public:
};

// In cases where data's area fits inside map's area we just loop the grid
// This process creates a cropped area which is equal to intersection of map's
// area and data's area.
TEST_F(DataUtilitiesGetCroppedArea, euroAreaIsTightlyCroppedToNordicDataArea)
{
    auto referenceArea = ReferenceAreaCreator::getReferenceAreaWith(nordicStereoInfo, *euroStereoArea);
    adapter.initializeDrawingData(nordicStereoInfo, *euroStereoArea);

    auto croppedArea = adapter.getCroppedArea();

    EXPECT_TRUE(PointsUtil::pointsAreEqual(croppedArea->BottomLeftLatLon(), referenceArea->BottomLeftLatLon()));
    EXPECT_TRUE(PointsUtil::pointsAreEqual(croppedArea->TopRightLatLon(), referenceArea->TopRightLatLon()));
}

TEST_F(DataUtilitiesGetCroppedArea, worldAreaIsTightlyCroppedToNordicDataArea)
{
    auto referenceArea = ReferenceAreaCreator::getReferenceAreaWith(nordicStereoInfo, *atlanticLatLonArea);
    adapter.initializeDrawingData(nordicStereoInfo, *atlanticLatLonArea);

    auto croppedArea = adapter.getCroppedArea();

    EXPECT_TRUE(PointsUtil::pointsAreEqual(croppedArea->BottomLeftLatLon(), referenceArea->BottomLeftLatLon()));
    EXPECT_TRUE(PointsUtil::pointsAreEqual(croppedArea->TopRightLatLon(), referenceArea->TopRightLatLon()));
}

TEST_F(DataUtilitiesGetCroppedArea, worldAreaIsTightlyCroppedToEuroDataArea)
{
    auto referenceArea = ReferenceAreaCreator::getReferenceAreaWith(euroStereoInfo, *atlanticLatLonArea);
    adapter.initializeDrawingData(euroStereoInfo, *atlanticLatLonArea);

    auto croppedArea = adapter.getCroppedArea();

    EXPECT_TRUE(PointsUtil::pointsAreEqual(croppedArea->BottomLeftLatLon(), referenceArea->BottomLeftLatLon()));
    EXPECT_TRUE(PointsUtil::pointsAreEqual(croppedArea->TopRightLatLon(), referenceArea->TopRightLatLon()));
}

// In cases where data's area does not fit inside map's area we loop the grid
// in a way that we crop out the values located outside map area's border values.
// Map area's border values are the minimum and maximum latlon values of map area.
// Because border values create a larger area than map area was the cropped area is
// larger than the intersection of map's area and data's area.
TEST_F(DataUtilitiesGetCroppedArea, euroAreaIsCroppedAroundWorldDataArea)
{
    auto* referenceArea = ReferenceAreaCreator::getReferenceAreaWith(atlanticLatLonInfo, *euroStereoArea);
    adapter.initializeDrawingData(atlanticLatLonInfo, *euroStereoArea);

    auto croppedArea = adapter.getCroppedArea();

    EXPECT_TRUE(croppedArea->IsInside(*referenceArea));
}

TEST_F(DataUtilitiesGetCroppedArea, nordicAreaIsCroppedAroundEuroDataArea)
{
    auto* referenceArea = ReferenceAreaCreator::getReferenceAreaWith(euroStereoInfo, *nordicStereoArea);
    adapter.initializeDrawingData(euroStereoInfo, *nordicStereoArea);

    auto croppedArea = adapter.getCroppedArea();

    EXPECT_TRUE(croppedArea->IsInside(*referenceArea));
}

TEST_F(DataUtilitiesGetCroppedArea, nordicAreaIsCroppedAroundWorldDataArea)
{
    auto* referenceArea = ReferenceAreaCreator::getReferenceAreaWith(atlanticLatLonInfo, *nordicStereoArea);
    adapter.initializeDrawingData(atlanticLatLonInfo, *nordicStereoArea);

    auto croppedArea = adapter.getCroppedArea();

    EXPECT_TRUE(croppedArea->IsInside(*referenceArea));
}

TEST_F(DataUtilitiesGetCroppedArea, croppedAreaIsAlwaysNormalized)
{
    adapter.initializeDrawingData(nordicStereoInfo, *euroStereoArea);
    checkThatAreaIsNormalized(*adapter.getCroppedArea());
    adapter.clear();

    adapter.initializeDrawingData(atlanticLatLonInfo, *nordicStereoArea);
    checkThatAreaIsNormalized(*adapter.getCroppedArea());
    adapter.clear();

    adapter.initializeDrawingData(euroStereoInfo, *nordicStereoArea);
    checkThatAreaIsNormalized(*adapter.getCroppedArea());
    adapter.clear();

    adapter.initializeDrawingData(atlanticLatLonInfo, *euroStereoArea);
    checkThatAreaIsNormalized(*adapter.getCroppedArea());
    adapter.clear();

    adapter.initializeDrawingData(nordicStereoInfo, *atlanticLatLonArea);
    checkThatAreaIsNormalized(*adapter.getCroppedArea());
}

class DataUtilitiesGetInterpolatedData : public DataUtilitiesTest
{
public:
    void areasAreEqual(NFmiArea& area, NFmiDataMatrix<NFmiPoint>& matrix)
    {
        auto matrixBottomLeft = matrix[0][0];
        auto matrixTopRight = matrix[matrix.NX() - 1][matrix.NY() - 1];

        EXPECT_TRUE(PointsUtil::pointsAreEqual(area.BottomLeftLatLon(), matrixBottomLeft));
        EXPECT_TRUE(PointsUtil::pointsAreEqual(area.TopRightLatLon(), matrixTopRight));
    }
};

TEST_F(DataUtilitiesGetInterpolatedData, interpolatedDataAreaIsCroppedArea)
{
    adapter.initializeDrawingData(nordicStereoInfo, *euroStereoArea);
    areasAreEqual(*adapter.getCroppedArea(), *adapter.getInterpolatedData());
    adapter.clear();

    adapter.initializeDrawingData(atlanticLatLonInfo, *nordicStereoArea);
    areasAreEqual(*adapter.getCroppedArea(), *adapter.getInterpolatedData());
    adapter.clear();

    adapter.initializeDrawingData(euroStereoInfo, *nordicStereoArea);
    areasAreEqual(*adapter.getCroppedArea(), *adapter.getInterpolatedData());
    adapter.clear();

    adapter.initializeDrawingData(atlanticLatLonInfo, *euroStereoArea);
    areasAreEqual(*adapter.getCroppedArea(), *adapter.getInterpolatedData());
    adapter.clear();

    adapter.initializeDrawingData(nordicStereoInfo, *atlanticLatLonArea);
    areasAreEqual(*adapter.getCroppedArea(), *adapter.getInterpolatedData());
}

//loggaaminen pit‰‰ testata (kun lumberjacket tai vastaava on valmis)

class DataUtilitiesExceptionHandling : public DataUtilitiesTest
{
public:
    DataUtilitiesAdapter<> adapterWithoutToolMaster;

    DataUtilitiesExceptionHandling()
        :adapterWithoutToolMaster(Toolmaster::NotAvailable, 0, "default")
    {
    }
};

TEST_F(DataUtilitiesExceptionHandling, getCroppedAreaThrowsExceptionIfModifiedDataDrawingIsFalse)
{
    adapter.initializeDrawingData(euroStereoInfo, *euroStereoArea);
    EXPECT_THAT(adapter.isModifiedDataDrawingPossible(), Eq(false));
    EXPECT_THROW(adapter.getCroppedArea(), CheckIsModifiedDataDrawingPossibleBeforeCallToGetCroppedArea);
}

TEST_F(DataUtilitiesExceptionHandling, getInterpolatedDataThrowsExceptionIfModifiedDataDrawingIsFalse)
{
    adapter.initializeDrawingData(atlanticLatLonInfo, *atlanticLatLonArea);
    EXPECT_THAT(adapter.isModifiedDataDrawingPossible(), Eq(false));
    EXPECT_THROW(adapter.getInterpolatedData(), CheckIsModifiedDataDrawingPossibleBeforeCallToGetInterpolatedData);
}

TEST_F(DataUtilitiesExceptionHandling, getCroppedAreaThrowsExceptionIfToolmasterIsNotAvailable)
{
    adapterWithoutToolMaster.initializeDrawingData(euroStereoInfo, *atlanticLatLonArea);
    EXPECT_THAT(adapterWithoutToolMaster.isModifiedDataDrawingPossible(), Eq(false));
    EXPECT_THROW(adapterWithoutToolMaster.getCroppedArea(), CheckIsModifiedDataDrawingPossibleBeforeCallToGetCroppedArea);
}

TEST_F(DataUtilitiesExceptionHandling, getInterpolatedDataThrowsExceptionIfToolmasterIsNotAvailable)
{
    adapterWithoutToolMaster.initializeDrawingData(atlanticLatLonInfo, *euroStereoArea);
    EXPECT_THAT(adapterWithoutToolMaster.isModifiedDataDrawingPossible(), Eq(false));
    EXPECT_THROW(adapterWithoutToolMaster.getInterpolatedData(), CheckIsModifiedDataDrawingPossibleBeforeCallToGetInterpolatedData);
}

TEST_F(DataUtilitiesExceptionHandling, getRootGridDoesNotThrowExceptionEvenCallingBeforeOneIsMade)
{
    EXPECT_NO_THROW(adapter.getRootGrid());
}

// muista asioista ei lenn‰ exceptionia (paitsi jos keksit‰‰n uusia tapoja k‰ytt‰‰ ohjelmaa v‰‰rin)
