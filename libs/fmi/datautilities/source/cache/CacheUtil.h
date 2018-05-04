#pragma once

#include "source/dataUtil/MapViewId.h"
#include "NFmiArea.h"
namespace SmartMetDataUtilities
{
    namespace CacheUtil
    {
        template <typename CacheT>
        decltype(auto) createKeys(const MapViewId& id, const NFmiArea& area, const NFmiGrid& originalGrid)
        {
            return std::remove_reference<CacheT>::type::createKeysFrom(id.getMapViewIndex(), id.getFileFilter(), area, originalGrid);
        }

        template <typename CacheT>
        decltype(auto) get(CacheT&& cache, const MapViewId& id, const NFmiArea& area, const NFmiGrid& originalGrid)
        {
            auto cacheSpecificKeys = createKeys<CacheT>(id, area, originalGrid);
            return cache.get(std::move(cacheSpecificKeys));
        }

        template <typename CacheT, typename DataT>
        decltype(auto) put(CacheT&& cache, const MapViewId& id, const NFmiArea& area, DataT&& data, const NFmiGrid& originalGrid)
        {
            auto cacheSpecificKeys = createKeys<CacheT>(id, area, originalGrid);
            cache.put(std::move(cacheSpecificKeys), std::forward<DataT>(data));
        }
    }
}