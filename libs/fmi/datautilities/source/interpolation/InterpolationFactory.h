#pragma once
#include "source/interpolation/Interpolation.h"
#include "source/interpolation/OldInterpolation.h"
#include "source/interpolation/NewInterpolation.h"
#include "source/interpolation/CroppedInterpolation.h"

#include <memory>

namespace SmartMetDataUtilities
{
    namespace InterpolationFactory
    {
        namespace
        {
            bool rootDataCanBeUsed(const NFmiArea& mapArea, const NFmiGrid& originalGrid, RootDataCache& rootCache, MapViewId keys)
            {
                try
                {
                    auto rootData = CacheUtil::get(rootCache, keys, mapArea, originalGrid);
                    auto RootGridArea = rootData.getGrid()->Area();
                    auto originalMapArea = rootData.getOriginalMapArea();
                    return (originalMapArea->IsInside(mapArea) || RootGridArea->IsInside(mapArea))
                        && AreaUtil::isPacific(*originalMapArea) == AreaUtil::isPacific(mapArea);
                }
                catch(const DataNotFoundException&)
                {
                    return false;
                }
            }

            bool interpolatedDataCanBeUsed(const NFmiArea& mapArea, const NFmiGrid& originalGrid, InterpolatedDataCache& interpolatedCache, MapViewId keys)
            {
                auto cacheSpecificKeys = CacheUtil::createKeys<InterpolatedDataCache>(keys, mapArea, originalGrid);
                return interpolatedCache.exists(cacheSpecificKeys);
            }
        }

        std::unique_ptr<Interpolation> create(NFmiGrid& grid, const NFmiArea& mapArea, InterpolatedDataCache& interpolatedCache, RootDataCache& rootCache, MapViewId keys)
        {
            if(interpolatedDataCanBeUsed(mapArea, grid, interpolatedCache, keys))
            {
                return std::make_unique<OldInterpolation>(grid, mapArea, interpolatedCache, keys);
            }
            else if(rootDataCanBeUsed(mapArea, grid, rootCache, keys))
            {
                return std::make_unique<CroppedInterpolation>(grid, mapArea, rootCache, keys);
            }
            else
            {
                return std::make_unique<NewInterpolation>(grid, mapArea, rootCache, keys);
            }
        }
    }
}