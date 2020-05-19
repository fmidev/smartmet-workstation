#pragma once

#include "source/interpolatedData/InterpolatedData.h"
#include "source/cache/Cache.h"
#include "source/cache/LruCache.h"
#include "source/newbaseUtil/GridUtil.h"
#include "source/newbaseUtil/AreaUtil.h"

class NFmiArea;

namespace SmartMetDataUtilities
{
    class InterpolatedDataCache
    {
    public:
        using LruKey = std::string;
        using Lru = LruCache<LruKey, InterpolatedData>;
        using Base = Cache<unsigned int, std::string, Lru>;
        using Keys = std::tuple<Base::Key1, Base::Key2, LruKey>;
    private:
        Base cache_;
    public:

        bool exists(const Keys& keys);
        const InterpolatedData& get(const Keys& keys);
        void put(const Keys& keys, InterpolatedData data);
        void clear();

        template<typename Key1, typename Key2>
        static Keys createKeysFrom(Key1&& key1, Key2&& key2, const NFmiArea& area, const NFmiGrid& originalGrid);

        template<typename Key1, typename Key2>
        static Keys createKeysFrom(Key1&& key1, Key2&& key2, const NFmiArea& area, const std::string& originalGridResolution);
    };

    template<typename Key1, typename Key2>
    InterpolatedDataCache::Keys InterpolatedDataCache::createKeysFrom(Key1&& key1, Key2&& key2, const NFmiArea& area, const NFmiGrid& originalGrid)
    {
        auto areaStr = area.AreaFactoryStr();
        auto originalGridResolution = GridUtil::resolutionOf(originalGrid);
        return Keys(std::forward<Key1>(key1), std::forward<Key2>(key2), areaStr + ":" + originalGridResolution);
    }

    template<typename Key1, typename Key2>
    InterpolatedDataCache::Keys InterpolatedDataCache::createKeysFrom(Key1&& key1, Key2&& key2, const NFmiArea& area, const std::string& originalGridResolution = "default")
    {
        auto areaStr = area.AreaFactoryStr();
        return Keys(std::forward<Key1>(key1), std::forward<Key2>(key2), areaStr + ":" + originalGridResolution);
    }
}

