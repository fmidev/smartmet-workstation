#include "wmssupport/stdafx.h"
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "WmsSupport/QueryCache.h"
#include "WmsSupport/Query.h"

#include "cpprest/http_client.h"
#include "TestHelper.h"


using namespace testing;
using namespace std;
using namespace Wms;

using SharedBaseQuery = shared_ptr<BaseQuery>;
using Bitmap = Gdiplus::Bitmap;

class QueryCacheConstructor : public Test
{
public:
    QueryCache cache;
};

TEST_F(QueryCacheConstructor, cacheIsEmptyWhenConstructed)
{
    EXPECT_THAT(cache.getSize(), Eq(0));
}

TEST_F(QueryCacheConstructor, maxSizeDefaultsToHundred)
{
    EXPECT_THAT(cache.getMaxSize(), Eq(100));
}

TEST(QueryCacheConstructorWithMaxSize, cacheMaxSizeIsGivenInConstructor)
{
    QueryCache cache100(100);
    QueryCache cache10(10);

    EXPECT_THAT(cache100.getMaxSize(), Eq(100));
    EXPECT_THAT(cache10.getMaxSize(), Eq(10));
}

class QueryCacheGet : public QueryCacheConstructor
{
public:
    SharedBaseQuery base;
    SharedBaseQuery differentBase;
    function<QueryCache::bitmap_ptr(GetMapQuery)> nullCallback;
    QueryCacheGet()
    {
        base = constructBaseQueryWithDefaultValues();
        differentBase = constructBaseQueryWithNonDefaultValues();

        nullCallback = [](auto query)
        {
            return nullptr;
        };
        cache.setGetMapCallback(nullCallback);
    }

    string getRandomStringLengthOf(size_t length) {
        static auto alphanum =
            "0123456789"
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "abcdefghijklmnopqrstuvwxyz";
        string s;
        s.reserve(length);
        for(int i = 0; i < length; ++i)
        {
            s.push_back(alphanum[rand() % (sizeof(alphanum) - 1)]);
        }
        return s;
    }

    SharedBaseQuery constructBaseQueryWithNonDefaultValues()
    {
        auto tmpBase = make_shared<BaseQuery>();
        TestHelper::setDefaults(*tmpBase);
        tmpBase->setMapPath("some-other-path");
        return tmpBase;
    }

    SharedBaseQuery constructBaseQueryWithDefaultValues()
    {
        auto tmpBase = make_shared<BaseQuery>();
        TestHelper::setDefaults(*tmpBase);
        return tmpBase;
    }

    GetMapQuery constructQueryWithRandomValues(SharedBaseQuery &base)
    {
        auto query = GetMapQuery(base);
        TestHelper::setDefaults(query);
        query.setFormat(getRandomStringLengthOf(10));
        query.setCrs(getRandomStringLengthOf(5));
        query.setSldPath(getRandomStringLengthOf(20));
        return query;
    }

    GetMapQuery constructQueryWithDefaultValues(SharedBaseQuery &base)
    {
        auto query = GetMapQuery(base);
        TestHelper::setDefaults(query);
        return query;
    }

    vector<GetMapQuery> constructVectorOfRandomQueriesWithLength(size_t length)
    {
        vector<GetMapQuery> vec;
        vec.reserve(length);
        for(int i = 0; i < length; ++i)
        {
            vec.push_back(constructQueryWithRandomValues(base));
        }
        return vec;
    }
};

TEST_F(QueryCacheGet, cacheSizeGrowsWhenAskedForNewQuery)
{
    auto query = constructQueryWithRandomValues(base);

    ASSERT_THAT(cache.getSize(), Eq(0));
    cache.get(query);
    EXPECT_THAT(cache.getSize(), Eq(1));
}

TEST_F(QueryCacheGet, cacheSizeDoesNotGrowOverMaxSize)
{
    QueryCache cache2(2);
    cache2.setGetMapCallback(nullCallback);
    auto queries = constructVectorOfRandomQueriesWithLength(3);

    ASSERT_THAT(cache2.getSize(), Eq(0));
    cache2.get(queries[0]);
    EXPECT_THAT(cache2.getSize(), Eq(1));
    cache2.get(queries[1]);
    EXPECT_THAT(cache2.getSize(), Eq(2));
    cache2.get(queries[2]);
    EXPECT_THAT(cache2.getSize(), Eq(2));
}


TEST_F(QueryCacheGet, twoDifferentQueriesWithDifferentMembersAreBothCached)
{
    auto query1 = constructQueryWithDefaultValues(base);
    auto query2 = constructQueryWithDefaultValues(base);
    query1.setFormat("a");
    query2.setFormat("b");

    cache.get(query1);
    ASSERT_THAT(cache.getSize(), Eq(1));
    cache.get(query2);
    EXPECT_THAT(cache.getSize(), Eq(2));
}

TEST_F(QueryCacheGet, twoDifferentQueriesWithEqualMembersOnlyOneIsCached)
{
    auto query1 = constructQueryWithDefaultValues(base);
    auto query2 = constructQueryWithDefaultValues(base);

    cache.get(query1);
    ASSERT_THAT(cache.getSize(), Eq(1));
    cache.get(query2);
    EXPECT_THAT(cache.getSize(), Eq(1));
}

TEST_F(QueryCacheGet, twoEqualQueriesWithDifferenBaseQueryOnlyOneIsCached)
{
    auto query1 = constructQueryWithDefaultValues(base);
    auto query2 = constructQueryWithDefaultValues(differentBase);

    cache.get(query1);
    ASSERT_THAT(cache.getSize(), Eq(1));
    cache.get(query2);
    EXPECT_THAT(cache.getSize(), Eq(1));
}

class QueryCacheSetCallback : public QueryCacheGet
{
public:
    int callbackCallTimes = 0;
    function<QueryCache::bitmap_ptr(GetMapQuery)> callback;
    vector<GetMapQuery> queries;
    unordered_map<string, Bitmap*> queryPointerMap;

    QueryCacheSetCallback()
    {
        queries = constructVectorOfRandomQueriesWithLength(10);
        queryPointerMap = constructMapOfQueriesToPointers(queries);
        callback = [=](auto query)
        {
            callbackCallTimes++;
            return unique_ptr<Bitmap>(queryPointerMap[query.stringify()]);
        };
    }

    unordered_map<string, Bitmap*> constructMapOfQueriesToPointers(vector<GetMapQuery> queries)
    {
        unordered_map<string, Bitmap*> map;
        for each (GetMapQuery query in queries)
        {
            auto key = query.stringify();
            map[key] = new Bitmap(10, 10);
        }
        return map;
    }

    Bitmap* getCorrespondingPointer(GetMapQuery query, unordered_map<string, Bitmap*> map)
    {
        return map[query.stringify()];
    }
};

TEST_F(QueryCacheSetCallback, QueryCacheCallsCallbackIfQueryResultIsNotInTheCache)
{
    cache.setGetMapCallback(callback);
    auto query = queries[0];

    cache.get(query);

    EXPECT_THAT(callbackCallTimes, Eq(1));
}

TEST_F(QueryCacheSetCallback, QueryCacheDoesNotCallCallbackIfQueryResultIsInTheCache)
{
    cache.setGetMapCallback(callback);
    auto query = queries[0];

    cache.get(query);
    ASSERT_THAT(callbackCallTimes, Eq(1));

    cache.get(query);
    EXPECT_THAT(callbackCallTimes, Eq(1));
}

TEST_F(QueryCacheSetCallback, QueryCacheReturnsCallbackResult)
{
    cache.setGetMapCallback(callback);
    auto query = queries[0];
    auto bitmapPtr = getCorrespondingPointer(query, queryPointerMap);

    auto result = cache.get(query);
    EXPECT_THAT(result.get(), Eq(bitmapPtr));
}

TEST_F(QueryCacheSetCallback, QueryCacheStoresCallbackResult)
{
    cache.setGetMapCallback(callback);
    auto query = queries[0];
    auto bitmapPtr = getCorrespondingPointer(query, queryPointerMap);
    ASSERT_THAT(callbackCallTimes, Eq(0));

    auto result = cache.get(query);
    EXPECT_THAT(callbackCallTimes, Eq(1));
    EXPECT_THAT(result.get(), Eq(bitmapPtr));

    result = cache.get(query);
    EXPECT_THAT(callbackCallTimes, Eq(1));
    EXPECT_THAT(result.get(), Eq(bitmapPtr));
}

class QueryCacheLru : public QueryCacheSetCallback
{
public:
    QueryCache cache5;

    QueryCacheLru()
        :cache5(5)
    {
        cache5.setGetMapCallback(callback);
    };
};

TEST_F(QueryCacheLru, cacheDiscardsElementsInLeastRecentUsedOrder)
{
    EXPECT_THAT(callbackCallTimes, Eq(0));
    cache5.get(queries[0]);
    EXPECT_THAT(callbackCallTimes, Eq(1));
    // Just to check that calling same query second time does not call callback when query is in cache
    cache5.get(queries[0]);
    EXPECT_THAT(callbackCallTimes, Eq(1));

    cache5.get(queries[1]);
    EXPECT_THAT(callbackCallTimes, Eq(2));
    cache5.get(queries[2]);
    EXPECT_THAT(callbackCallTimes, Eq(3));
    cache5.get(queries[3]);
    EXPECT_THAT(callbackCallTimes, Eq(4));
    cache5.get(queries[4]);
    EXPECT_THAT(callbackCallTimes, Eq(5));
    //next call should drop first query
    cache5.get(queries[5]);
    EXPECT_THAT(callbackCallTimes, Eq(6));
    // and if first call is dropped then next call should invoke the callback
    cache5.get(queries[0]);
    EXPECT_THAT(callbackCallTimes, Eq(7));
}

TEST_F(QueryCacheLru, callingElementAgainMakesItRecentUsed)
{
    cache5.get(queries[0]);
    cache5.get(queries[1]);
    cache5.get(queries[2]);
    cache5.get(queries[3]);
    cache5.get(queries[4]);

    auto callbacksCalls = callbackCallTimes;
    // lets call 0 again
    cache5.get(queries[0]);
    EXPECT_THAT(callbackCallTimes, Eq(callbacksCalls));
    // next call should drop 1
    cache5.get(queries[5]);
    EXPECT_THAT(callbackCallTimes, Eq(callbacksCalls + 1));
    // so calling 1 should invoke the callback
    cache5.get(queries[1]);
    EXPECT_THAT(callbackCallTimes, Eq(callbacksCalls + 2));
}