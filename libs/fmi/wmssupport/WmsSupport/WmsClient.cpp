#include "WmsSupport/WmsClient.h"
#include "WmsSupport/WmsQuery.h"
#include "WmsSupport/BitmapCache.h"
#include "WmsSupport/CapabilitiesHandler.h"

#include "bitmaphandler/BitmapParser.h"

#include <webclient/Client.h>

#include <cppext/split.h>
#include <cppback/background-manager.h>

#include <future>
#include <utility>
#include <functional>

using namespace std::chrono_literals;

namespace Wms
{
    namespace
    {
        auto imageWithLoadingState()
        {
            auto holder = std::make_shared<NFmiImageData>();
            holder->mState = NFmiImageData::kLoading;
            holder->mImage = nullptr;
            return holder;
        }
    }

    WmsClient::WmsClient(std::unique_ptr<BitmapCache> cache,
        std::unique_ptr<BitmapHandler::BitmapParser> parser,
        std::unique_ptr<Web::Client> client,
        std::shared_ptr<cppback::BackgroundManager> bManager,
        std::unique_ptr<QueryBuilder> qb,
        int imgTimeoutInSeconds,
        int lgndTimeoutInSeconds
    )
        : cache_{ std::move(cache) }
        , bitmapParser_{ std::move(parser) }
        , client_{ std::move(client) }
        , bManager_{ bManager }
        , qb_{ std::move(qb) }
        , imageTimeoutInSeconds(imgTimeoutInSeconds)
        , legendTimeoutInSeconds(lgndTimeoutInSeconds)
    {}

    void WmsClient::setImageLoadedCallback(std::function<void()> imageLoadedCallback)
    {
        imageLoadedCallback_ = imageLoadedCallback;
    }

    void WmsClient::initializeDynamic(const DynamicServerSetup& serverSetup, const std::string& proxyUrl)
    {
        qb_ = std::make_unique<QueryBuilder>(serverSetup.generic.stereo00, serverSetup.generic.stereo10, serverSetup.generic.stereo20, serverSetup.generic.useCrs);
        qb_
            ->setScheme(serverSetup.generic.scheme)
            .setHost(serverSetup.generic.host)
            .setPath(serverSetup.generic.path)
            .setMap(serverSetup.generic.map)
            .setVersion(serverSetup.version)
            .setToken(serverSetup.generic.token)
            .setService("WMS")
            .setFormat("image/png")
            .setTransparency(serverSetup.transparency);
    }

    void WmsClient::initializeUserUrl(const UserUrlServerSetup& serverSetup, const std::string& proxyUrl)
    {
        for(const auto& parsedSetup : serverSetup.parsedServers)
        {
            auto qb = QueryBuilder{ parsedSetup.stereo00, parsedSetup.stereo10, parsedSetup.stereo20, parsedSetup.useCrs };
            qb
                .setScheme(parsedSetup.scheme)
                .setHost(parsedSetup.host)
                .setPath(parsedSetup.path)
                .setMap(parsedSetup.map)
                .setVersion(serverSetup.version)
                .setToken(parsedSetup.token)
                .setService("WMS")
                .setFormat("image/png")
                .setTransparency(serverSetup.transparency);
            parsedUserUrls_.push_back(qb);
        }
    }

    void WmsClient::kill()
    {
        client_->cancel();
    }

    QueryBuilder WmsClient::getQB() const
    {
        return *qb_;
    }
    QueryBuilder WmsClient::getUserUrlQB(int id) const
    {
        return parsedUserUrls_[id];
    }

    NFmiImageHolder WmsClient::getLegend(const std::string& domain, const std::string& request)
    {
        try
        {
            auto response = client_->queryFor(domain, request, legendTimeoutInSeconds).share();
            return parseResponse(response);
        }
        catch(const std::exception&)
        {
            return nullptr;
        }
    }

    NFmiImageHolder WmsClient::getImage(const WmsQuery& query)
    {
        auto key1 = toKey1(query);
        auto key2 = toKey2(query);
        if(isCached(key1, key2))
        {
            std::unique_lock<std::mutex> getLock{ cacheMutex_ };
            return cache_->get(key1, key2);
        }
        else
        {
            return getFromServer(query);
        }
    }

    NFmiImageHolder WmsClient::getFromServer(const WmsQuery& query)
    {
        try
        {
            auto response = client_->queryFor(toBaseUri(query), toRequest(query), imageTimeoutInSeconds).share();
            if(imageLoadedCallback_)
            {
                return asyncWait(query, response);
            }
            else
            {
                auto image = parseResponse(response);
                putToCache(query, image);
                return image;
            }
        }
        catch(const std::exception &e)
        {
            std::string errorMessage = __FUNCTION__;
            errorMessage += "failed: ";
            errorMessage += e.what();
            CatLog::logMessage(errorMessage, CatLog::Severity::Error, CatLog::Category::NetRequest, true);
            return nullptr;
        }
    }

    NFmiImageHolder WmsClient::asyncWait(const WmsQuery& query, std::shared_future<std::string> response)
    {
        if(response.wait_for(200ms) == std::future_status::ready)
        {
            auto holder = parseResponse(response);
            putToCache(query, holder);
            return holder;
        }
        else
        {
            waitUntilReadyAndNotifyByCallback(query, response);
            return imageWithLoadingState();
        }
    }

    void WmsClient::waitUntilReadyAndNotifyByCallback(const WmsQuery& query, std::shared_future<std::string> response)
    {
        bManager_->addTask([&,
            fut = std::move(response),
            imageLoadedCallback = imageLoadedCallback_,
            query = query] () mutable
        {
            auto holder = parseResponse(fut);
            if(holder)
            {
                putToCache(query, holder);
                imageLoadedCallback();
            }
        });
    }

    NFmiImageHolder WmsClient::parseResponse(std::shared_future<std::string> fut) const
    {
        if(fut.valid())
        {
            return bitmapParser_->parse(fut.get());
        }
        return nullptr;
    }

    void WmsClient::putToCache(const WmsQuery& query, NFmiImageHolder image)
    {
        if(image)
        {
            std::unique_lock<std::mutex> putLock{ cacheMutex_ };
            cache_->put(toKey1(query), toKey2(query), image);
        }
    }

    bool WmsClient::isCached(WmsQuery query) const
    {
        return isCached(toKey1(query), toKey2(query));
    }

    bool WmsClient::isCached(const std::string& key1, const std::string& key2) const
    {
        std::unique_lock<std::mutex> existsLock{ cacheMutex_ };
        return cache_->exists(key1, key2);
    }

    bool WmsClient::isCached(const std::string& key1) const
    {
        std::unique_lock<std::mutex> existsLock{ cacheMutex_ };
        return cache_->exists(key1);
    }

    void WmsClient::dirtyLayer(const std::string& key1)
    {
        std::unique_lock<std::mutex> removeLock{ cacheMutex_ };
        cache_->erase(key1);
    }
}
