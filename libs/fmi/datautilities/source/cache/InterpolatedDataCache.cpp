#include "source/cache/InterpolatedDataCache.h"
#include "source/error/Error.h"

#include <tuple>

using namespace std;

namespace SmartMetDataUtilities
{
    bool InterpolatedDataCache::exists(const Keys& keys)
    {
        try
        {
            get(keys);
            return true;
        }
        catch (const exception&)
        {
            return false;
        }
    }

    const InterpolatedData& InterpolatedDataCache::get(const Keys& keys)
    {
        try
        {
            return cache_.get(keys).get(std::get<2>(keys));
        }
        catch(const exception&)
        {
            throw DataNotFoundException("Data for given key was not found.");
        }
    }

    void InterpolatedDataCache::put(const Keys& keys, InterpolatedData data)
    {
        try
        {
             auto& lru = cache_.get(keys);
             lru.put(std::get<2>(keys), move(data));
        }
        catch(const exception&)
        {
            Lru lru(2);
            lru.put(std::get<2>(keys), move(data));
            cache_.put(keys, move(lru));
        }
    }

    void InterpolatedDataCache::clear()
    {
        cache_.clear();
    }
}