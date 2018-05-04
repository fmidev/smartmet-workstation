#pragma once

#include <unordered_map>
#include <tuple>

namespace SmartMetDataUtilities
{
    template <typename Key1T, typename Key2T, typename DataT>
    class Cache
    {
    public:
        using Key1 = Key1T;
        using Key2 = Key2T;
        using Cache2 = std::unordered_map<Key2, DataT>;
        using Cache1 = std::unordered_map<Key1, Cache2>;
    private:
        Cache1 cache1;
    public:
        Cache() = default;
        ~Cache() = default;

        template<typename MapViewId>
        DataT& get(MapViewId&& keys);

        template<typename MapViewId, typename Data>
        void put(MapViewId&& keys, Data&& data);

        void clear();

    private:
        template <typename CacheT ,typename Key>
        DataT& get(CacheT&& cache, Key&& key);
    };

    template <typename Key1T, typename Key2T, typename DataT>
    template<typename MapViewId>
    DataT& Cache<Key1T, Key2T, DataT>::get(MapViewId&& keys)
    {
        auto result = cache1.find(std::get<0>(keys));
        if(result != cache1.end())
        {
            auto& cache2 = result->second;
            return get(cache2, std::get<1>(keys));
        }
        throw DataNotFoundException("Data for given key was not found.");
    }

    template <typename Key1T, typename Key2T, typename DataT>
    template<typename MapViewId, typename Data>
    void Cache<Key1T, Key2T, DataT>::put(MapViewId&& keys, Data&& data)
    {
        auto result = cache1.find(std::get<0>(keys));
        if(result != cache1.end())
        {
            auto& cache2 = result->second;
            cache2[std::get<1>(keys)] = std::forward<Data>(data);
        }
        else
        {
            Cache2 cache2;
            cache2[std::get<1>(keys)] = std::forward<Data>(data);
            cache1[std::get<0>(keys)] = std::forward<Cache2>(cache2);
        }
    }

    template <typename Key1T, typename Key2T, typename DataT>
    void Cache<Key1T, Key2T, DataT>::clear()
    {
        cache1.clear();
    }

    // Get helper function
    template <typename Key1T, typename Key2T, typename DataT>
    template <typename CacheT, typename Key>
    DataT& Cache<Key1T, Key2T, DataT>::get(CacheT&& cache, Key&& key)
    {
        auto result = cache.find(key);
        if(result != cache.end())
        {
            return result->second;
        }
        throw DataNotFoundException("Data for given key was not found.");
    }

}
