#include "WmsSupport/GdiplusBitmapCache.h"
#include "WmsSupport/WmsQuery.h"
#include "WmsSupport/lrucache.h"
#include "WmsSupport/SetupParser.h"
#include "WmsSupport/Setup.h"

#include <cpprest/asyncrt_utils.h>
#undef U // This fixes cpprest's U -macro clash with boost library move code (really dangerous to give macro name like U !!!!)


using namespace std;

namespace Wms
{
    GdiplusBitmapCache::GdiplusBitmapCache(size_t maxSize, size_t numberOfLayersPerCache)
        :twoLayerCache_{ maxSize }
        , sndCacheSize_{ numberOfLayersPerCache }
    {
    };

    bool GdiplusBitmapCache::exists(const std::string& key1, const std::string& key2)
    {
        if(twoLayerCache_.exists(key1))
        {
            const auto& sndCache = twoLayerCache_.get(key1);
            return sndCache.exists(key2);
        }
        return false;
    }

    bool GdiplusBitmapCache::exists(const std::string& key1)
    {
        return twoLayerCache_.exists(key1);
    }

    NFmiImageHolder GdiplusBitmapCache::get(const std::string& key1, const std::string& key2)
    {
        return twoLayerCache_.modify(key1).get(key2);
    }

    void GdiplusBitmapCache::put(const std::string& key1, const std::string& key2, NFmiImageHolder image)
    {
        if(twoLayerCache_.exists(key1))
        {
            twoLayerCache_.modify(key1).put(key2, image);
        }
        else
        {
            auto sndCache = cache::lru_cache<std::string, NFmiImageHolder>{ sndCacheSize_ };
            sndCache.put(key2, image);
            twoLayerCache_.put(key1, std::move(sndCache));
        }
    }

    void GdiplusBitmapCache::erase(const std::string& key1)
    {
        if(twoLayerCache_.exists(key1))
        {
            twoLayerCache_.erase(key1);
        }
    }
}