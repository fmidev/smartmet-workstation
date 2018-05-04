#include "source/cache/RootDataCache.h"
#include "source/error/Error.h"

using namespace std;

namespace SmartMetDataUtilities
{
    const RootData& RootDataCache::get(Keys& keys)
    {
        auto& result = cache_.get(keys);
        if(result.first == std::get<2>(keys))
        {
            return result.second;
        }
        throw DataNotFoundException("Data for given key was not found.");
    }

    void RootDataCache::put(Keys& keys, RootData data)
    {
        try
        {
            auto& result = cache_.get(keys);
            result.first = std::get<2>(keys);
            result.second = move(data);
        }
        catch(const DataNotFoundException&)
        {
            RootDataPair rootDataPair(std::get<2>(keys), move(data));
            cache_.put(keys, move(rootDataPair));
        }
    }

    void RootDataCache::clear()
    {
        cache_.clear();
    }
}