#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "WmsSupport/BaseQuery.h"

#include "TestHelper.h"

#include "cpprest/http_client.h"

using namespace testing;
using namespace std;
using namespace Wms;
using namespace utility::conversions;
using namespace TestHelper::BaseQueryDefaults;

class BaseQuerySetters : public Test
{
public:
};

TEST_F(BaseQuerySetters, settersWorkAsExpected)
{
    BaseQuery query;

    query.setCgiPath(cgiPath);
    query.setMapPath(mapPath);
    query.setService(service);
    query.setVersion(version);

    EXPECT_THAT(query.getCgiPath(), Eq(cgiPath));
    EXPECT_THAT(query.getMapPath(), Eq(mapPath));
    EXPECT_THAT(query.getService(), Eq(service));
    EXPECT_THAT(query.getVersion(), Eq(version));
}

class BaseQueryGetQuery : public Test
{
public:
    BaseQuery query;
    string correctQuery = "/cgi-bin/mapserv?map=/var/www/html/mapserver/mapfiles/smartmet/smartmet.map&SERVICE=WMS&VERSION=1.3.0";
};

TEST_F(BaseQueryGetQuery, throwsExceptionIfRequiredParametersAreMissing)
{
    EXPECT_THROW(query.getQuery(), runtime_error);
    query.setCgiPath(cgiPath);
    EXPECT_THROW(query.getQuery(), runtime_error);
    query.setMapPath(mapPath);
    EXPECT_THROW(query.getQuery(), runtime_error);
    query.setService(service);
    EXPECT_THROW(query.getQuery(), runtime_error);
    query.setVersion(version);
    EXPECT_NO_THROW(query.getQuery());
}

TEST_F(BaseQueryGetQuery, returnsCorrectlyFormattedUrl)
{
    TestHelper::setDefaults(query);
    string resultingQuery = to_utf8string(query.getQuery());
    EXPECT_THAT(resultingQuery, Eq(correctQuery));
}