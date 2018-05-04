#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "WmsSupport/Query.h"
#include "WmsSupport/WmsSupportHelpers.h"

#include "TestHelper.h"

#include "NFmiRect.h"

#include "cpprest/http_client.h"

using namespace testing;
using namespace std;
using namespace Wms;
using namespace utility::conversions;

class GetMapConstructor : public Test
{
public:
};

TEST_F(GetMapConstructor, takesQueryBaseAsArgument)
{
    auto base = make_shared<BaseQuery>();
    TestHelper::setDefaults(*base);

    GetMapQuery query(base);
}

TEST_F(GetMapConstructor, canBeConstructedWithoutQueryBase)
{
    GetMapQuery query;
}

class GetMapSetters : public GetMapConstructor
{
public:
    GetMapQuery query;
    shared_ptr<BaseQuery> base;
    string bboxStringified;
    string layersStringified;

    GetMapSetters()
        :base(make_shared<BaseQuery>())
    {
        TestHelper::setDefaults(*base);
        query = GetMapQuery(base);
        bboxStringified = stringifyRect(TestHelper::GetMapQueryDefaults::bbox);
        layersStringified = stringifyLayers(TestHelper::GetMapQueryDefaults::layers);
    }
};

TEST_F(GetMapSetters, workAsExpected)
{
    query.setFormat(TestHelper::GetMapQueryDefaults::format);
    query.setCrs(TestHelper::GetMapQueryDefaults::crs);
    query.setWidth(TestHelper::GetMapQueryDefaults::width);
    query.setHeight(TestHelper::GetMapQueryDefaults::height);
    query.setBbox(TestHelper::GetMapQueryDefaults::bbox);
    query.setLayers(TestHelper::GetMapQueryDefaults::layers);
    query.setSldPath(TestHelper::GetMapQueryDefaults::sldPath);

    EXPECT_THAT(query.getFormat(), Eq(TestHelper::GetMapQueryDefaults::format));
    EXPECT_THAT(query.getCrs(), Eq(TestHelper::GetMapQueryDefaults::crs));
    EXPECT_THAT(query.getWidth(), Eq(TestHelper::GetMapQueryDefaults::width));
    EXPECT_THAT(query.getHeight(), Eq(TestHelper::GetMapQueryDefaults::height));
    EXPECT_THAT(query.getBbox(), Eq(bboxStringified));
    EXPECT_THAT(query.getLayers(), Eq(layersStringified));
    EXPECT_THAT(query.getSldPath(), Eq(TestHelper::GetMapQueryDefaults::sldPath));
}

class GetMapGetQuery : public GetMapSetters
{
public:
};

TEST_F(GetMapGetQuery, throwsExceptionIfRequiredArgumentsAreMissing)
{
    EXPECT_THROW(query.getQuery(), runtime_error);
    query.setFormat(TestHelper::GetMapQueryDefaults::format);
    EXPECT_THROW(query.getQuery(), runtime_error);
    query.setCrs(TestHelper::GetMapQueryDefaults::crs);
    EXPECT_THROW(query.getQuery(), runtime_error);
    query.setWidth(TestHelper::GetMapQueryDefaults::width);
    EXPECT_THROW(query.getQuery(), runtime_error);
    query.setHeight(TestHelper::GetMapQueryDefaults::height);
    EXPECT_THROW(query.getQuery(), runtime_error);
    query.setBbox(TestHelper::GetMapQueryDefaults::bbox);
    EXPECT_THROW(query.getQuery(), runtime_error);
    query.setLayers(TestHelper::GetMapQueryDefaults::layers);
    EXPECT_THROW(query.getQuery(), runtime_error);
    query.setSldPath(TestHelper::GetMapQueryDefaults::sldPath);
    EXPECT_NO_THROW(query.getQuery());
}

TEST_F(GetMapGetQuery, throwsExceptionIfBaseQueryIsMissing)
{
    GetMapQuery missingBase;
    TestHelper::setDefaults(missingBase);

    EXPECT_THROW(missingBase.getQuery(), runtime_error);
}

TEST_F(GetMapGetQuery, throwsExceptionIfBaseQueryIsNotFullyInitialized)
{
    auto insufficientBase = make_shared<BaseQuery>();
    insufficientBase->setVersion("versio");
    insufficientBase->setCgiPath("/cgi/path");

    GetMapQuery getMapWithInsufficientBase(insufficientBase);
    TestHelper::setDefaults(getMapWithInsufficientBase);

    EXPECT_THROW(getMapWithInsufficientBase.getQuery(), runtime_error);
}

TEST_F(GetMapGetQuery, returnsCorrectlyFormattedUrl)
{
    TestHelper::setDefaults(query);
    auto capabilitiesQuery = to_utf8string(query.getQuery());
    EXPECT_THAT(capabilitiesQuery, Eq(TestHelper::GetMapQueryDefaults::defaultsAsQuery));
}

class GetMapStringify : public GetMapSetters
{
public:
    string produceCorrectStringifiedQuery(GetMapQuery &query)
    {
        return "GetMap"
            + query.getFormat()
            + query.getCrs()
            + query.getBbox()
            + query.getLayers()
            + query.getSldPath()
            + to_string(query.getWidth())
            + to_string(query.getHeight());
    }
};

TEST_F(GetMapStringify, stringifyProducesStringThatContainsMembers)
{
    TestHelper::setDefaults(query);
    auto expected = produceCorrectStringifiedQuery(query);
    EXPECT_THAT(query.stringify(), Eq(expected));
}
