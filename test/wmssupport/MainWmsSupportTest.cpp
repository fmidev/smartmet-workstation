#include "WmsSupport/stdafx.h"
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "WmsSupport/WmsSupport.h"
#include "WmsSupport/Query.h"
#include "WmsSupport/WmsSupportHelpers.h"
#include "WmsSupport/QueryCache.h"
#include "WmsSupport/Client.h"
#include "NFmiAreaFactory.h"
#include "NFmiRect.h"
#include "NFmiArea.h"
#include "NFmiSatelliteImageCacheHelpers.h"

#include "cpprest/http_client.h"
#include <initializer_list>

using namespace testing;
using namespace std;
using namespace Wms;
using namespace Gdiplus;
using namespace utility::conversions;

using resolution = pair<unsigned int, unsigned int>;
using state = NFmiImageData::ImageStateEnum;
using area_ptr = boost::shared_ptr<NFmiArea>;

struct EpsgValues
{
    static const string epsg1000;
    static const string epsg1010;
    static const string epsg1020;
    static const string epsg4326;
    static const string epsg2393;
    static const string epsg900913;
};

const string EpsgValues::epsg1000 = "EPSG:1000";
const string EpsgValues::epsg1010 = "EPSG:1010";
const string EpsgValues::epsg1020 = "EPSG:1020";
const string EpsgValues::epsg4326 = "EPSG:4326";
const string EpsgValues::epsg2393 = "EPSG:2393";
const string EpsgValues::epsg900913 = "EPSG:900913";

class MainWmsSupportGetImageParametersAsSetters : public Test
{
public:
    unique_ptr<WmsSupport> main;

    NFmiRect rect = NFmiRect(-1016264.49, -4076014.23000000, 1003739.19, -1810793.44);
    NFmiRect anotherRect = NFmiRect(-101624.49, -407014.23000000, 100739.19, -181793.44);
    resolution resolution_xy = { 200, 100 };
    resolution anotherResolution_xy = { 234234, 54355434 };
    // Empty values
    NFmiRect emptyRect;
    resolution emptyResolution_xy;

    MainWmsSupportGetImageParametersAsSetters()
    {
        main = make_unique<WmsSupport>();
    }

    auto stereographic(unsigned int orientation)
    {
        return NFmiAreaFactory::Create("stereographic," + to_string(orientation) + ",90,60:6,51.3,49,70.2");
    }

    auto mercator()
    {
        return NFmiAreaFactory::Create("mercator:6,51.3,49,70.2");
    }

    auto ykj()
    {
        return NFmiAreaFactory::Create("ykj:6,51.3,49,70.2");
    }

    auto latlon()
    {
        return NFmiAreaFactory::Create("latlon");
    }

    auto notSupported()
    {
        return NFmiAreaFactory::Create("gnomonic");
    }

    auto callGetImageWithGivenParametersAndCatchAll(const area_ptr &area, const NFmiRect &rect, const resolution &res)
    {
        try
        {
        main->getImage(area, rect, res);
        }
        catch (...)
        {
        }
    }

    auto crsValueOfGetMapQueryWhenImageIsQueriedWithAreaTypeOf(area_ptr area)
    {
        callGetImageWithGivenParametersAndCatchAll(area, emptyRect, emptyResolution_xy);
        return main->getGetMapQuery().getCrs();
    }

    auto bboxValueOfGetMapQueryWhenImageIsQueriedWithRectSizeOf(NFmiRect rect)
    {
        callGetImageWithGivenParametersAndCatchAll(latlon(), rect, emptyResolution_xy);
        return main->getGetMapQuery().getBbox();
    }

    resolution resolutionOfGetMapQueryWhenImageIsQueriedWithResolution(resolution res)
    {
        callGetImageWithGivenParametersAndCatchAll(latlon(), emptyRect, res);
        auto getMap = main->getGetMapQuery();
        return{ getMap.getWidth(),  getMap.getHeight() };
    }
};

TEST_F(MainWmsSupportGetImageParametersAsSetters, CrsIsSetAccordingToAreaType)
{
    auto crs = crsValueOfGetMapQueryWhenImageIsQueriedWithAreaTypeOf(stereographic(0));
    EXPECT_THAT(crs, Eq(EpsgValues::epsg1000));

    crs = crsValueOfGetMapQueryWhenImageIsQueriedWithAreaTypeOf(stereographic(10));
    EXPECT_THAT(crs, Eq(EpsgValues::epsg1010));

    crs = crsValueOfGetMapQueryWhenImageIsQueriedWithAreaTypeOf(stereographic(20));
    EXPECT_THAT(crs, Eq(EpsgValues::epsg1020));

    crs = crsValueOfGetMapQueryWhenImageIsQueriedWithAreaTypeOf(mercator());
    EXPECT_THAT(crs, Eq(EpsgValues::epsg900913));

    crs = crsValueOfGetMapQueryWhenImageIsQueriedWithAreaTypeOf(ykj());
    EXPECT_THAT(crs, Eq(EpsgValues::epsg2393));

    crs = crsValueOfGetMapQueryWhenImageIsQueriedWithAreaTypeOf(latlon());
    EXPECT_THAT(crs, Eq(EpsgValues::epsg4326));
}

TEST_F(MainWmsSupportGetImageParametersAsSetters, getImageThrowsIfAreaTypeIsNotSupported)
{
    // getImage throws anyway because setters and etc are not set up
    // emtpy crs value however proves that unsupported values are not queried
    auto crs = crsValueOfGetMapQueryWhenImageIsQueriedWithAreaTypeOf(notSupported());
    EXPECT_THAT(crs, Eq(""));
}

TEST_F(MainWmsSupportGetImageParametersAsSetters, throwsIfOrientationNotSupported)
{
    // getImage throws anyway because setters and etc are not set up
    // emtpy crs value however proves that unsupported values are not queried
    auto crs = crsValueOfGetMapQueryWhenImageIsQueriedWithAreaTypeOf(stereographic(17));
    EXPECT_THAT(crs, Eq(""));
}

TEST_F(MainWmsSupportGetImageParametersAsSetters, SetsBboxAccordingToGivenRect)
{
    auto bbox = bboxValueOfGetMapQueryWhenImageIsQueriedWithRectSizeOf(rect);
    EXPECT_THAT(bbox, Eq(stringifyRect(rect)));

    bbox = bboxValueOfGetMapQueryWhenImageIsQueriedWithRectSizeOf(anotherRect);
    EXPECT_THAT(bbox, Eq(stringifyRect(anotherRect)));
}

TEST_F(MainWmsSupportGetImageParametersAsSetters, SetsResolutionAccordingToGivenPair)
{
    auto res = resolutionOfGetMapQueryWhenImageIsQueriedWithResolution(resolution_xy);
    EXPECT_THAT(res.first, Eq(resolution_xy.first));
    EXPECT_THAT(res.second, Eq(resolution_xy.second));

    res = resolutionOfGetMapQueryWhenImageIsQueriedWithResolution(anotherResolution_xy);
    EXPECT_THAT(res.first, Eq(anotherResolution_xy.first));
    EXPECT_THAT(res.second, Eq(anotherResolution_xy.second));
}

class MainWmsSupportSetters : public MainWmsSupportGetImageParametersAsSetters
{
public:
    //client
    string serverAddress = "http://wms-intra2.fmi.fi";
    //QueryCache
    size_t cacheSize = 10;
    size_t defaultSize = 100;
    //QueryBase
    string mapPath = "/var/www/html/mapserver/mapfiles/smartmet/smartmet.map";
    string cgiPath = "/cgi-bin/mapserv";
    string service = "WMS";
    string version = "1.3.0";
    // GetMapQuery
    vector<string> layers = { "merialueet" };
    string sldPath = "wms-intra2.fmi.fi/mapserver/sld/smartmet.xml";
    string format = "image/png";

    unsigned int callbackCount = 0;
    function<void()> callback;

    MainWmsSupportSetters()
    {
        callback = [&callbackCount = callbackCount]()
        {
            ++callbackCount;
        };
    }

    string getCurrentUrlFromClient()
    {
        return to_utf8string(main->getServerTalker().getClient().base_uri().to_string());
    }
};

TEST_F(MainWmsSupportSetters, settersRelatedToGetMapPassValuesToGetMapQuery)
{
    main->setSldPath(sldPath);
    main->setImageFormat(format);
    main->setLayers(layers);

    auto getMapQuery = main->getGetMapQuery();

    EXPECT_THAT(getMapQuery.getSldPath(), Eq(sldPath));
    EXPECT_THAT(getMapQuery.getFormat(), Eq(format));
    EXPECT_THAT(getMapQuery.getLayers(), Eq(stringifyLayers(layers)));
}

TEST_F(MainWmsSupportSetters, settersRelatedToBaseQueryPassValuesToBaseQuery)
{
    main->setMapPath(mapPath);
    main->setCgiPath(cgiPath);
    main->setService(service);
    main->setVersion(version);

    auto getMapQuery = main->getBaseQuery();

    EXPECT_THAT(getMapQuery.getMapPath(), Eq(mapPath));
    EXPECT_THAT(getMapQuery.getCgiPath(), Eq(cgiPath));
    EXPECT_THAT(getMapQuery.getService(), Eq(service));
    EXPECT_THAT(getMapQuery.getVersion(), Eq(version));
}

TEST_F(MainWmsSupportSetters, settingCacheSizeCreatesNewCacheWithGivenSize)
{
    auto cache = main->getCache();

    ASSERT_THAT(cache.getMaxSize(), Eq(defaultSize));
    main->setCacheSize(cacheSize);

    cache = main->getCache();
    EXPECT_THAT(cache.getMaxSize(), Eq(cacheSize));
}

TEST_F(MainWmsSupportSetters, settingServerAddressCreatesnewServerTalkerWithGivenAddress)
{
    auto uri = getCurrentUrlFromClient();
    ASSERT_THAT(uri, Not(StartsWith(serverAddress + "2")));

    main->setServerAddress(serverAddress);

    uri = getCurrentUrlFromClient();
    EXPECT_THAT(uri, StartsWith(serverAddress));
}

TEST_F(MainWmsSupportSetters, settingCallbackWorksAsExpected)
{
    main->setImageLoadedCallback(callback);

    auto returnedCallback = main->getImageLoadedCallback();

    ASSERT_THAT(callbackCount, Eq(0));
    returnedCallback();
    EXPECT_THAT(callbackCount, Eq(1));
}

class MainWmsSupportGetImage : public MainWmsSupportSetters
{
public:

    void callSettersButDoNotSetCallBack()
    {
        main->setServerAddress(serverAddress);
        main->setCacheSize(cacheSize);

        main->setMapPath(mapPath);
        main->setCgiPath(cgiPath);
        main->setService(service);
        main->setVersion(version);

        main->setSldPath(sldPath);
        main->setImageFormat(format);
        main->setLayers(layers);
    }

    void callSettersWithFixedValues()
    {
        callSettersButDoNotSetCallBack();
        main->setImageLoadedCallback(callback);
    }

    void callSettersWithFixedValuesAndInitialize()
    {
        callSettersWithFixedValues();
        main->initialize();
    }

    NFmiImageHolder callGetImageWithFixedParameters()
    {
        auto area = stereographic(20);
        return main->getImage(area, rect, resolution_xy);
    }

    void waitForAsyncOperationsToFinish()
    {
        this_thread::sleep_for(500ms);
    }
};

TEST_F(MainWmsSupportGetImage, throwsExceptionIfCallbackNotSet)
{
    callSettersButDoNotSetCallBack();
    EXPECT_THROW(callGetImageWithFixedParameters(), runtime_error);
}

TEST_F(MainWmsSupportGetImage, throwsExceptionIfGetImageIsNotProperlyInitialized)
{
    callSettersWithFixedValues();
    main->setImageFormat("");
    EXPECT_THROW(callGetImageWithFixedParameters(), runtime_error);
}


TEST_F(MainWmsSupportGetImage, throwsExceptionIfQueryBaseIsNotProperlyInitialized)
{
    callSettersWithFixedValues();
    main->setCgiPath("");
    EXPECT_THROW(callGetImageWithFixedParameters(), runtime_error);
}

TEST_F(MainWmsSupportGetImage, GetImageCallsUserDefinedCallbackIfQueryIsNotInCache)
{
    callSettersWithFixedValuesAndInitialize();

    ASSERT_THAT(callbackCount, Eq(0));
    callGetImageWithFixedParameters();
    waitForAsyncOperationsToFinish();
    EXPECT_THAT(callbackCount, Eq(1));
}

TEST_F(MainWmsSupportGetImage, GetImageDoesNotCallUserDefinedCallbackIfQueryIsInCache)
{
    callSettersWithFixedValuesAndInitialize();
    ASSERT_THAT(callbackCount, Eq(0));

    callGetImageWithFixedParameters();
    waitForAsyncOperationsToFinish();

    EXPECT_THAT(callbackCount, Eq(1));

    callGetImageWithFixedParameters();
    waitForAsyncOperationsToFinish();

    EXPECT_THAT(callbackCount, Eq(1));
}

TEST_F(MainWmsSupportGetImage, ReturnsNFmiImageHolderContainingNullptrIfQueryIsNotInCache)
{
    callSettersWithFixedValuesAndInitialize();

    auto holder = callGetImageWithFixedParameters();

    EXPECT_THAT(holder->mImage, Eq(nullptr));
}

TEST_F(MainWmsSupportGetImage, ReturnsNFmiImageHolderInLoadingStateIfQueryIsNotInCache)
{
    callSettersWithFixedValuesAndInitialize();

    auto holder = callGetImageWithFixedParameters();
    EXPECT_THAT(holder->mState, Eq(state::kLoading));
}

TEST_F(MainWmsSupportGetImage, returnsNFmiImageHolderInOkStateIfQueryIsCached)
{
    callSettersWithFixedValuesAndInitialize();
    callGetImageWithFixedParameters();
    waitForAsyncOperationsToFinish();

    auto holder = callGetImageWithFixedParameters();

    EXPECT_THAT(holder->mState, Eq(state::kOk));
}

TEST_F(MainWmsSupportGetImage, returnsNFmiImageHolderContainingBitmapIfQueryIsCached)
{
    callSettersWithFixedValuesAndInitialize();
    callGetImageWithFixedParameters();
    waitForAsyncOperationsToFinish();

    auto holder = callGetImageWithFixedParameters();

    EXPECT_THAT(holder->mImage, Not(nullptr));
}

TEST_F(MainWmsSupportGetImage, emptyImageHolderIsReturnedExactly200msAfterCall)
{
    callSettersWithFixedValuesAndInitialize();
    auto area = stereographic(20);

    auto start = chrono::system_clock::now();
    main->getImage(area, rect, resolution_xy);
    auto end = chrono::system_clock::now();
    auto duration = end - start;

    auto durationInMs = chrono::duration_cast<chrono::milliseconds>(duration);

    auto twoHundredMs = 210ms;
    EXPECT_THAT(durationInMs.count(), Le(twoHundredMs.count()));
}

class MainWmsSupportInitialize : public MainWmsSupportGetImage
{
public:
};

TEST_F(MainWmsSupportInitialize, initThrowsIfCalledBeforeSetters)
{
    main->setImageLoadedCallback(callback);
    EXPECT_THROW(main->initialize(), runtime_error);

    callSettersButDoNotSetCallBack();

    EXPECT_NO_THROW(main->initialize());
}

TEST_F(MainWmsSupportInitialize, GetImageThrowsIfCalledBeforeInitialize)
{
    callSettersWithFixedValues();
    EXPECT_THROW(callGetImageWithFixedParameters(), runtime_error);
}

TEST_F(MainWmsSupportInitialize, GetImageDoesNotThrowIfCalledAfrerInitialize)
{
    callSettersWithFixedValues();

    main->initialize();

    EXPECT_NO_THROW(callGetImageWithFixedParameters());
}

TEST_F(MainWmsSupportInitialize, initThrowsIfImageLoadedCallbackIsNotSet)
{
    callSettersButDoNotSetCallBack();

    EXPECT_THROW(main->initialize(), runtime_error);

    main->setImageLoadedCallback(callback);
    EXPECT_NO_THROW(main->initialize());
}


// Kill testiajo

// isDead testiajo

