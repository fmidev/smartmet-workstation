#pragma once
#include "WmsSupport/BitmapCache.h"

#include "lrucache.h"
#include "NFmiSatelliteImageCacheHelpers.h"

#include <string>
#include <set>

namespace Wms
{
    class GdiplusBitmapCache : public Wms::BitmapCache
    {
        cache::lru_cache<std::string, cache::lru_cache<std::string, NFmiImageHolder>> twoLayerCache_;
        size_t sndCacheSize_;
    public:
        GdiplusBitmapCache(size_t maxSize, size_t numberOfLayers);
        ~GdiplusBitmapCache();

        bool exists(const std::string& key1, const std::string& key2) final;
        bool exists(const std::string& key1) final;
        NFmiImageHolder get(const std::string& key1, const std::string& key2) final;
        void put(const std::string& key1, const std::string& key2, NFmiImageHolder bitmap) final;
        void erase(const std::string& key1) final;
    };
}

