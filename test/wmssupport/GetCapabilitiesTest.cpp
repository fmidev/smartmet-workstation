#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "WmsSupport/GetCapabilitiesQuery.h"
#include "WmsSupport/BaseQuery.h"

#include "TestHelper.h"

#include <memory>
#include "cpprest/http_client.h"

using namespace testing;
using namespace std;
using namespace Wms;
using namespace utility::conversions;

class GetCapabilitiesConstructor : public Test
{
public:
    std::shared_ptr<BaseQuery> base = make_shared<BaseQuery>();

    GetCapabilitiesConstructor()
    {
        TestHelper::setDefaults(*base);
    }
};

TEST_F(GetCapabilitiesConstructor, ConstructorTakesSharedPointerToBaseQueryAsAParameter)
{
    EXPECT_NO_THROW(GetCapabilitiesQuery query(base));
}

TEST_F(GetCapabilitiesConstructor, GetCapabilitiesCanBeConstructedWithoutBaseQuery)
{
    GetCapabilitiesQuery query;
}

class GetCapabilitiesSetters : public GetCapabilitiesConstructor
{
public:
};

TEST_F(GetCapabilitiesSetters, BaseQueryCanBeInjectedThroughSetter)
{
    GetCapabilitiesQuery query;
    query.setBaseQuery(base);
}

class GetCapabilitiesGetQuery : public GetCapabilitiesConstructor
{
public:
    string correctQuery = "/cgi-bin/mapserv?map=/var/www/html/mapserver/mapfiles/smartmet/smartmet.map&SERVICE=WMS&VERSION=1.3.0&REQUEST=GetCapabilities";
};

TEST_F(GetCapabilitiesGetQuery, returnsCorrectlyFormattedUrl)
{
    auto getCapabilities = *TestHelper::getCapabilitiesWith(base);
    string resultingQuery = to_utf8string(getCapabilities.getQuery());
    EXPECT_THAT(resultingQuery, Eq(correctQuery));
}

TEST_F(GetCapabilitiesGetQuery, throwsExceptionIfMissingBaseQuery)
{
    GetCapabilitiesQuery missingBase;

    EXPECT_THROW(missingBase.getQuery(), runtime_error);
}

TEST_F(GetCapabilitiesGetQuery, throwsExceptionIfBaseQueryIsNotFullyInitialized)
{
    auto insufficientBase = make_shared<BaseQuery>();
    insufficientBase->setVersion("versio");
    insufficientBase->setCgiPath("/cgi/path");

    GetCapabilitiesQuery getCapabilitiesWithInsufficientBase(insufficientBase);

    EXPECT_THROW(getCapabilitiesWithInsufficientBase.getQuery(), runtime_error);
}