#pragma once

#include "source/rootData/RootData.h"
#include "source/cache/Cache.h"
#include "source/newbaseUtil/GridUtil.h"
#include "source/newbaseUtil/AreaUtil.h"

namespace SmartMetDataUtilities
{
    class RootDataCache
    {
    public:
        using PairKey = std::string;
        using RootDataPair = std::pair<PairKey, RootData>;
        using Base = Cache<unsigned int, std::string, RootDataPair>;
        using Keys = std::tuple<Base::Key1, Base::Key2, PairKey>;
    private:
        Base cache_;
    public:

        const RootData& get(Keys &keys);
        void put(Keys& keys, RootData data);
        void clear();

        template<typename Key1, typename Key2>
        static Keys createKeysFrom(Key1&& key1, Key2&& key2, const NFmiArea& area, const NFmiGrid& originalGrid);

        template<typename Key1, typename Key2>
        static Keys createKeysFrom(Key1&& key1, Key2&& key2, const NFmiArea& area, const std::string& originalGridResolution);
    };

    template<typename Key1, typename Key2>
    RootDataCache::Keys RootDataCache::createKeysFrom(Key1&& key1, Key2&& key2, const NFmiArea& area, const NFmiGrid& originalGrid)
    {
        auto projStr = area.ProjStr();
        auto originalGridResolution = GridUtil::resolutionOf(originalGrid);
        return Keys(std::forward<Key1>(key1), std::forward<Key2>(key2), projStr + ":" + originalGridResolution);
    }

    template<typename Key1, typename Key2>
    RootDataCache::Keys RootDataCache::createKeysFrom(Key1&& key1, Key2&& key2, const NFmiArea& area, const std::string& originalGridResolution = "default")
    {
        auto projStr = area.ProjStr();
        return Keys(std::forward<Key1>(key1), std::forward<Key2>(key2), projStr+":"+ originalGridResolution);
    }
}

