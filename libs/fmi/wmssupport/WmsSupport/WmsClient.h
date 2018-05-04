#pragma once
#include "wmssupport/QueryBuilder.h"
#include "wmssupport/Setup.h"

#include "NFmiSatelliteImageCacheHelpers.h"

#include <cppback/background-manager.h>

#include <memory>
#include <string>
#include <functional>
#include <future>
#include <mutex>

#include <boost/property_tree/ptree.hpp>

namespace Web
{
    class Client;
}

namespace BitmapHandler
{
    class BitmapParser;
}

namespace Wms
{
    class BitmapCache;
    class WmsQuery;
    class ServerSetup;

    class WmsClient
    {
        std::function<void()> imageLoadedCallback_;
        std::unique_ptr<BitmapHandler::BitmapParser> bitmapParser_;
        std::unique_ptr<Web::Client> client_;
        std::unique_ptr<BitmapCache> cache_;

        std::shared_ptr<cppback::BackgroundManager> bManager_;
        mutable std::mutex cacheMutex_;

        std::vector<QueryBuilder> parsedUserUrls_;
        std::unique_ptr<QueryBuilder> qb_;
    public:
        WmsClient(std::unique_ptr<BitmapCache> cache,
            std::unique_ptr<BitmapHandler::BitmapParser> parser,
            std::unique_ptr<Web::Client> client,
            std::shared_ptr<cppback::BackgroundManager> bManager,
            std::unique_ptr<QueryBuilder> qb
            );

        void setImageLoadedCallback(std::function<void()> imageLoadedCallback);
        void initializeDynamic(const DynamicServerSetup& serverSetup, const std::string& proxyUrl);
        void initializeUserUrl(const UserUrlServerSetup& serverSetup, const std::string& proxyUrl);
        void kill();

        QueryBuilder getQB() const;
        QueryBuilder getUserUrlQB(int id) const;

        NFmiImageHolder getLegend(const std::string& domain, const std::string& request);
        NFmiImageHolder getImage(const WmsQuery& query);

        bool isCached(const std::string& key1) const;
        bool isCached(WmsQuery key) const;
        void dirtyLayer(const std::string& key);
    private:
        NFmiImageHolder asyncWait(const WmsQuery& query, std::shared_future<std::string> response);
        NFmiImageHolder getFromServer(const WmsQuery& query);
        void waitUntilReadyAndNotifyByCallback(const WmsQuery& query, std::shared_future<std::string> response);
        NFmiImageHolder parseResponse(std::shared_future<std::string> fut) const;
        void putToCache(const WmsQuery& query, NFmiImageHolder image);
        bool isCached(const std::string& key1, const std::string& key2) const;
    };
}

