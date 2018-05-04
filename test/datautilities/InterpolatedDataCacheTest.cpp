#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "datautilities/../source/interpolatedData/InterpolatedData.h"
#include "datautilities/../source/cache/InterpolatedDataCache.h"
#include "datautilities/../source/error/Error.h"

#include "Data.h"
#include "TestHelper.h"

using namespace testing;
using namespace std;
using namespace SmartMetDataUtilities;
using namespace Data;
using namespace TestHelper;
using CacheKeys = tuple<unsigned int, string, string>;

class InterpolatedDataCacheKeysTest : public Test
{
public:
};

TEST_F(InterpolatedDataCacheKeysTest, DataCacheOffersStaticMethodThatCreatesCacheKeys)
{
    auto keys1 = InterpolatedDataCache::createKeysFrom(0, "file", *euroStereoArea, *atlanticLatLonInfo.Grid());
    auto keys2 = InterpolatedDataCache::createKeysFrom(1, "file", *nordicStereoArea, *euroStereoInfo.Grid());
    auto keys3 = InterpolatedDataCache::createKeysFrom(2, "file", *atlanticLatLonArea, *nordicStereoInfo.Grid());

    EXPECT_THAT(get<0>(keys1), Eq(0u));
    EXPECT_THAT(get<0>(keys2), Eq(1u));
    EXPECT_THAT(get<0>(keys3), Eq(2u));

    EXPECT_THAT(get<1>(keys1), Eq("file"));
    EXPECT_THAT(get<1>(keys2), Eq("file"));
    EXPECT_THAT(get<1>(keys3), Eq("file"));

    auto resolution = to_string(default_x) + "x" + to_string(default_y);

    EXPECT_THAT(get<2>(keys1), Eq("stereographic,10,90,60:-19.22,25,79.7,57:" + resolution));
    EXPECT_THAT(get<2>(keys2), Eq("stereographic,20,90,60:6,51.3,49,70.2:" + resolution));
    EXPECT_THAT(get<2>(keys3), Eq("latlon:-180,-90,180,90:" + resolution));
}

class InterpolatedDataCacheTest : public Test
{
public:
    InterpolatedDataCache cache;
    CacheKeys defaultKeys = { 0u, "default", "map:-19.22,25,79.7,57" };
};

class InterpolatedDataCachePut : public InterpolatedDataCacheTest
{
public:
};

TEST_F(InterpolatedDataCachePut, takesKeyStructAndRootDataAsInput)
{
    InterpolatedData data;
    EXPECT_NO_THROW(cache.put(defaultKeys, data));
}

TEST_F(InterpolatedDataCachePut, replacesOldEntryIfAllKeysFieldsMatch)
{
    CacheKeys keys1(0, "fileFilter", "latlon:12,90,30,28");
    CacheKeys keys2(0, "fileFilter", "latlon:12,90,30,28");

    cache.put(keys1, interpolatedData1);
    EXPECT_THAT(interpolatedDatasAreEqual(interpolatedData1, cache.get(keys1)), Eq(true));

    cache.put(keys2, interpolatedData2);
    EXPECT_THAT(interpolatedDatasAreEqual(interpolatedData2, cache.get(keys2)), Eq(true));
}

TEST_F(InterpolatedDataCachePut, doesNotReplaceOldEntryIfmapViewsDoNotMatch)
{
    CacheKeys keys1(0, "fileFilter", "latlon:12,90,30,28");
    CacheKeys keys2(1, "fileFilter", "latlon:12,90,30,28");
    cache.put(keys1, interpolatedData1);

    cache.put(keys2, interpolatedData2);

    EXPECT_THAT(interpolatedDatasAreEqual(interpolatedData2, cache.get(keys2)), Eq(true));
    // ensure that value for key1 is still in cache
    EXPECT_THAT(interpolatedDatasAreEqual(interpolatedData1, cache.get(keys1)), Eq(true));
}

TEST_F(InterpolatedDataCachePut, doesNotReplaceOldEntryIfFileFiltersDoNotMatch)
{
    CacheKeys keys1(0, "fileFilter", "latlon:12,90,30,28");
    CacheKeys keys2(0, "path/to/file.sqd", "latlon:12,90,30,28");
    cache.put(keys1, interpolatedData1);

    cache.put(keys2, interpolatedData2);

    EXPECT_THAT(interpolatedDatasAreEqual(interpolatedData2, cache.get(keys2)), Eq(true));
    // ensure that value for key1 is still in cache
    EXPECT_THAT(interpolatedDatasAreEqual(interpolatedData1, cache.get(keys1)), Eq(true));
}

TEST_F(InterpolatedDataCachePut, doesNotReplaceOldEntryIfAreaStrsDoNotMatch)
{
    CacheKeys keys1(0, "fileFilter", "latlon:12,90,30,28");
    CacheKeys keys2(0, "fileFilter", "latlon:11,90,30,28");
    cache.put(keys1, interpolatedData1);

    cache.put(keys2, interpolatedData2);

    EXPECT_THAT(interpolatedDatasAreEqual(interpolatedData2, cache.get(keys2)), Eq(true));
    // ensure that value for key1 is still in cache
    EXPECT_THAT(interpolatedDatasAreEqual(interpolatedData1, cache.get(keys1)), Eq(true));
}

TEST_F(InterpolatedDataCachePut, cacheHoldsMaxTwoDatasWithSameMapViewAndFileFilter)
{
    CacheKeys keys1(1, "fileFilter", "latlon:12,90,30,28");
    CacheKeys keys2(1, "fileFilter", "latlon:11,90,30,28");
    CacheKeys keys3(1, "fileFilter", "latlon:11,90,20,28");
    cache.put(keys1, interpolatedData1);
    cache.put(keys2, interpolatedData1);
    cache.put(keys3, interpolatedData1);
    EXPECT_NO_THROW(cache.get(keys3));
    EXPECT_NO_THROW(cache.get(keys2));
    EXPECT_THROW(cache.get(keys1), DataNotFoundException);
}

TEST_F(InterpolatedDataCachePut, firstElementPutInGetsDroppedFirst)
{
    CacheKeys keys1(0, "fileFilter", "latlon:12,90,30,28");
    CacheKeys keys2(0, "fileFilter", "latlon:11,90,30,28");
    CacheKeys keys3(0, "fileFilter", "latlon:11,90,20,28");
    cache.put(keys1, interpolatedData1);
    cache.put(keys2, interpolatedData1);
    cache.put(keys3, interpolatedData1);
    EXPECT_NO_THROW(cache.get(keys3));
    EXPECT_NO_THROW(cache.get(keys2));
    EXPECT_THROW(cache.get(keys1), DataNotFoundException);
}

class InterpolatedDataCacheGet : public InterpolatedDataCachePut
{
public:
};

TEST_F(InterpolatedDataCacheGet, getTakesRootCacheKeysAsCacheKey)
{
    CacheKeys keys(0, "fileFilter", "latlon:12,90,30,28");
    cache.put(keys, interpolatedData1);

    EXPECT_NO_THROW(cache.get(keys));
}

TEST_F(InterpolatedDataCacheGet, getReturnsReferenceToConstInterpolatedData)
{
    cache.put(defaultKeys, interpolatedData1);

    EXPECT_NO_THROW(const InterpolatedData& data = cache.get(defaultKeys));
}

TEST_F(InterpolatedDataCacheGet, getReturnsDataCorrespondingToGivenKey)
{
    CacheKeys keys1(0, "fileFilter", "latlon:12,90,30,28");
    CacheKeys keys2(1, "fileFilter", "latlon:12,90,30,28");
    CacheKeys keys3(0, "tmp/file.sqd", "latlon:12,90,30,28");
    CacheKeys keys4(0, "fileFilter", "stereographic,10,90,60:-19.22,25,79.7,57");
    cache.put(keys1, interpolatedData1);
    cache.put(keys2, interpolatedData2);
    cache.put(keys3, interpolatedData3);
    cache.put(keys4, interpolatedData4);

    EXPECT_THAT(interpolatedDatasAreEqual(cache.get(keys1), interpolatedData1), Eq(true));
    EXPECT_THAT(interpolatedDatasAreEqual(cache.get(keys2), interpolatedData2), Eq(true));
    EXPECT_THAT(interpolatedDatasAreEqual(cache.get(keys3), interpolatedData3), Eq(true));
    EXPECT_THAT(interpolatedDatasAreEqual(cache.get(keys4), interpolatedData4), Eq(true));
}

TEST_F(InterpolatedDataCacheGet, getThrowsExceptionIfNotFound)
{
    CacheKeys keys(0, "fileFilter", "latlon:12,90,30,28");
    EXPECT_THROW(cache.get(keys), DataNotFoundException);
}

TEST_F(InterpolatedDataCacheGet, gettingElevatesElementsLruStyle)
{
    CacheKeys keys1(0, "fileFilter", "latlon:12,90,30,28");
    CacheKeys keys2(0, "fileFilter", "latlon:11,90,30,28");
    CacheKeys keys3(0, "fileFilter", "latlon:11,90,20,28");
    cache.put(keys1, interpolatedData3);
    cache.put(keys2, interpolatedData3);

    cache.get(keys1);

    cache.put(keys3, interpolatedData3);

    EXPECT_NO_THROW(cache.get(keys3));
    EXPECT_NO_THROW(cache.get(keys1));
    EXPECT_THROW(cache.get(keys2), DataNotFoundException);
}

class InterpolatedDataCacheClear : public InterpolatedDataCacheTest
{
public:
};

TEST_F(InterpolatedDataCacheClear, clearClearsAllCachedData)
{
    CacheKeys keys1(0, "fileFilter", "ykj");
    CacheKeys keys2(0, "fileFilter", "latlon");
    CacheKeys keys3(5, "tmp/file.sqd", "latlon");
    CacheKeys keys4(76, "fileFilter", "latlon");
    CacheKeys keys5(76, "fileFilter", "ykj");
    cache.put(keys1, interpolatedData1);
    cache.put(keys2, interpolatedData2);
    cache.put(keys3, interpolatedData3);
    cache.put(keys4, interpolatedData4);
    cache.put(keys5, interpolatedData5);
    
    EXPECT_NO_THROW(cache.get(keys1));
    EXPECT_NO_THROW(cache.get(keys2));
    EXPECT_NO_THROW(cache.get(keys3));
    EXPECT_NO_THROW(cache.get(keys4));
    EXPECT_NO_THROW(cache.get(keys5));

    cache.clear();

    EXPECT_THROW(cache.get(keys1), DataNotFoundException);
    EXPECT_THROW(cache.get(keys2), DataNotFoundException);
    EXPECT_THROW(cache.get(keys3), DataNotFoundException);
    EXPECT_THROW(cache.get(keys4), DataNotFoundException);
    EXPECT_THROW(cache.get(keys5), DataNotFoundException);
}