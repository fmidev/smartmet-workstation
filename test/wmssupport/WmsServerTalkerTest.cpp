#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "WmsSupport/Client.h"
#include "WmsSupport/Query.h"
#include "WmsSupport/AssertionMacros.h"

#include "TestHelper.h"
#include "ClientMock.h"

#include <memory>

#include "cpprest/http_client.h"

using namespace testing;
using namespace std;
using namespace Wms;
using namespace utility;
using namespace utility::conversions;

using SharedBaseQuery = shared_ptr<BaseQuery>;
using cancellation_token = concurrency::cancellation_token;

class WmsServerTalkerConstructors : public Test
{
public:
    string serverAddress = "http://wms-intra2.fmi.fi";
};

TEST_F(WmsServerTalkerConstructors, constructorTakesClientAsArgument)
{
    ClientMock clientMock(serverAddress);
    EXPECT_NO_THROW(WmsServerTalker<ClientMock&> talker(clientMock));
}

TEST_F(WmsServerTalkerConstructors, clientDefaultsToCasablancaClient)
{
    WmsServerTalker<> talker;
    web::http::client::http_client casablancaClient(to_string_t(serverAddress));

    auto defaultClient = talker.getClient();

    EXPECT_TRUE(TestHelper::typesMatch(defaultClient, casablancaClient));
}

class WmsServerTalkerQueryFor : public Test
{
public:
    ClientMock clientMock;
    WmsServerTalker<ClientMock*> talker;
    string serverAddress = "http://wms-intra2.fmi.fi";
    SharedBaseQuery base;

    WmsServerTalkerQueryFor()
    {
        clientMock = ClientMock(serverAddress);
        talker = WmsServerTalker<ClientMock*>(&clientMock);
        base = make_shared<BaseQuery>();
        TestHelper::setDefaults(*base);
    }

    GetMapQuery initializedGetMapWith(SharedBaseQuery base)
    {
        GetMapQuery getMap(base);
        TestHelper::setDefaults(getMap);
        return getMap;
    }

    GetCapabilitiesQuery initializedGetCapabilitiesWith(SharedBaseQuery base)
    {
        return GetCapabilitiesQuery(base);
    }

    template <typename Query>
    void setExpectationsForQuery(Query &query)
    {
        EXPECT_CALL(clientMock, request(_, Eq(query.getQuery()), _))
            .WillOnce(Return(pplx::task<web::http::http_response>()));
    }
};

TEST_F(WmsServerTalkerQueryFor, queryForWorksWithGetMap)
{
    auto getMap = initializedGetMapWith(base);
    setExpectationsForQuery(getMap);
    EXPECT_NO_THROW(talker.queryFor(getMap, cancellation_token::none()));
}

TEST_F(WmsServerTalkerQueryFor, queryForWorksWithGetCapabilities)
{
    auto getCapabilities = initializedGetCapabilitiesWith(base);
    setExpectationsForQuery(getCapabilities);
    EXPECT_NO_THROW(talker.queryFor(getCapabilities, cancellation_token::none()));
}

TEST_F(WmsServerTalkerQueryFor, queryForThrowsExceptionIfQueryIsMissingRequiredParams)
{
    TestHelper::setDefaults(*base);
    auto getCapabilities = GetCapabilitiesQuery(base);
    auto getMap = GetMapQuery(base);
    EXPECT_THROW(talker.queryFor(getCapabilities, cancellation_token::none()), runtime_error);
    EXPECT_THROW(talker.queryFor(getMap, cancellation_token::none()), runtime_error);
}

class WmsServerTalkerClient : public WmsServerTalkerQueryFor
{
public:
    string_t correctGetMapQuery;
    string_t correctGetCapabilitiesQuery;
    WmsServerTalkerClient()
    {
        auto getCapabilities = initializedGetCapabilitiesWith(base);
        auto getMap = initializedGetMapWith(base);
        correctGetCapabilitiesQuery = getCapabilities.getQuery();
        correctGetMapQuery = getMap.getQuery();
    }
};

TEST_F(WmsServerTalkerClient, queryForGetCapabilitiesQueriesClientWithCorrectQuery)
{
    auto getCapabilities = initializedGetCapabilitiesWith(base);
    setExpectationsForQuery(getCapabilities);

    talker.queryFor(getCapabilities, cancellation_token::none());
}

TEST_F(WmsServerTalkerClient, queryForGetMapQueriesClientWithCorrectQuery)
{
    auto getMap = initializedGetMapWith(base);
    setExpectationsForQuery(getMap);

    talker.queryFor(getMap, cancellation_token::none());
}
