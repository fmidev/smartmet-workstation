#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "datautilities/../source/cache/RootDataCache.h"
#include "datautilities/../source/error/Error.h"

#include "Data.h"
#include "TestHelper.h"

using namespace testing;
using namespace std;
using namespace SmartMetDataUtilities;
using namespace Data;
using namespace TestHelper;
using CacheKeys = tuple<unsigned int, string, string>;

class RootDataCacheKeysTest : public Test
{
public:
};

TEST_F(RootDataCacheKeysTest, RootCacheOffersStaticMethodThatCreatesRootDataCacheKeys)
{
    auto keys1 = RootDataCache::createKeysFrom(0, "file", *euroStereoArea, *nordicStereoInfo.Grid());
    auto keys2 = RootDataCache::createKeysFrom(1, "file", *nordicStereoArea, *atlanticLatLonInfo.Grid());
    auto keys3 = RootDataCache::createKeysFrom(2, "file", *atlanticLatLonArea, *euroStereoInfo.Grid());

    EXPECT_THAT(get<0>(keys1), Eq(0u));
    EXPECT_THAT(get<0>(keys2), Eq(1u));
    EXPECT_THAT(get<0>(keys3), Eq(2u));

    EXPECT_THAT(get<1>(keys1), Eq("file"));
    EXPECT_THAT(get<1>(keys2), Eq("file"));
    EXPECT_THAT(get<1>(keys3), Eq("file"));

    auto resolution = to_string(default_x) + "x" + to_string(default_y);

    EXPECT_THAT(get<2>(keys1), Eq("stereographic,10,90,60:"+resolution));
    EXPECT_THAT(get<2>(keys2), Eq("stereographic,20,90,60:"+resolution));
    EXPECT_THAT(get<2>(keys3), Eq("latlon:"+ resolution));
}

class RootDataCacheTest : public Test
{
public:
    RootDataCache cache;
    CacheKeys rootDefaultKeys = {0u, "default", "map"};
};

class RootDataCachePut : public RootDataCacheTest
{
public:
};

TEST_F(RootDataCachePut, putTakesKeyStructAndRootDataAsInput)
{
    RootData data;
    EXPECT_NO_THROW(cache.put(rootDefaultKeys, data));
}

TEST_F(RootDataCachePut, replacesOldEntryIfAllKeysFieldsMatch)
{
    CacheKeys keys1(0, "fileFilter", "latlon");
    CacheKeys keys2(0, "fileFilter", "latlon");

    cache.put(keys1, rootData1);
    EXPECT_THAT(rootDatasAreEqual(rootData1, cache.get(keys1)), Eq(true));

    cache.put(keys2, rootData2);
    EXPECT_THAT(rootDatasAreEqual(rootData2, cache.get(keys2)), Eq(true));
}

TEST_F(RootDataCachePut, doesNotReplaceOldEntryIfmapViewsDoNotMatch)
{
    CacheKeys keys1(0, "fileFilter", "latlon");
    CacheKeys keys2(1, "fileFilter", "latlon");
    cache.put(keys1, rootData1);

    cache.put(keys2, rootData2);

    EXPECT_THAT(rootDatasAreEqual(rootData2, cache.get(keys2)), Eq(true));
    // ensure that value for key1 is still in cache
    EXPECT_THAT(rootDatasAreEqual(rootData1, cache.get(keys1)), Eq(true));
}

TEST_F(RootDataCachePut, doesNotReplaceOldEntryIfFileFiltersDoNotMatch)
{
    CacheKeys keys1(0, "fileFilter", "latlon");
    CacheKeys keys2(0, "path/to/file.sqd", "latlon");
    cache.put(keys1, rootData1);

    cache.put(keys2, rootData2);

    EXPECT_THAT(rootDatasAreEqual(rootData2, cache.get(keys2)), Eq(true));
    // ensure that value for key1 is still in cache
    EXPECT_THAT(rootDatasAreEqual(rootData1, cache.get(keys1)), Eq(true));
}

TEST_F(RootDataCachePut, replacesOldEntryIfmapViewIdAndFileFilterMatchButAreaStrsDoNotMatch)
{
    CacheKeys keys1(0, "fileFilter", "latlon");
    CacheKeys keys2(0, "fileFilter", "stereographic");
    cache.put(keys1, rootData1);

    cache.put(keys2, rootData2);

    EXPECT_THAT(rootDatasAreEqual(rootData2, cache.get(keys2)), Eq(true));
    EXPECT_THROW(cache.get(keys1), DataNotFoundException);
}

class RootDataCacheGet : public RootDataCachePut
{
public:
};

TEST_F(RootDataCacheGet, getTakesRootDataCacheKeysAsCacheKey)
{
    CacheKeys keys(0, "fileFilter", "latlon");
    cache.put(keys, rootData1);

    EXPECT_NO_THROW(cache.get(keys));
}

TEST_F(RootDataCacheGet, getReturnsReferenceToConstRootData)
{
    cache.put(rootDefaultKeys, rootData1);

    EXPECT_NO_THROW(const RootData& data = cache.get(rootDefaultKeys));
}

TEST_F(RootDataCacheGet, getReturnsDataCorrespondingToGivenKey)
{
    CacheKeys keys1(0, "fileFilter", "latlon");
    CacheKeys keys2(1, "fileFilter", "latlon");
    CacheKeys keys3(0, "tmp/file.sqd", "latlon");
    cache.put(keys1, rootData1);
    cache.put(keys2, rootData2);
    cache.put(keys3, rootData3);

    EXPECT_THAT(rootDatasAreEqual(cache.get(keys1), rootData1), Eq(true));
    EXPECT_THAT(rootDatasAreEqual(cache.get(keys2), rootData2), Eq(true));
    EXPECT_THAT(rootDatasAreEqual(cache.get(keys3), rootData3), Eq(true));
}

TEST_F(RootDataCacheGet, getThrowsExceptionIfNotFound)
{
    CacheKeys keys(0, "fileFilter", "latlon");
    EXPECT_THROW(cache.get(keys), DataNotFoundException);
}

TEST_F(RootDataCacheGet, getThrowsExceptionIfDataHasBeenReplaced)
{
    CacheKeys keys1(0, "fileFilter", "latlon");
    CacheKeys keys2(0, "fileFilter", "stereographic");
    cache.put(keys1, rootData1);
    cache.put(keys2, rootData2);

    EXPECT_THROW(cache.get(keys1), DataNotFoundException);
}

class RootDataCacheClear : public RootDataCacheTest
{
public:
};

TEST_F(RootDataCacheClear, clearWithoutKeyClearsAllCachedData)
{
    CacheKeys keys1(0, "fileFilter", "ykj");
    CacheKeys keys2(2, "fileFilter", "latlon");
    CacheKeys keys3(5, "tmp/file.sqd", "latlon");
    CacheKeys keys4(26, "fileFilter", "latlon");
    CacheKeys keys5(76, "fileFilter", "ykj");
    cache.put(keys1, rootData1);
    cache.put(keys2, rootData2);
    cache.put(keys3, rootData3);
    cache.put(keys4, rootData4);
    cache.put(keys5, rootData5);
    
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