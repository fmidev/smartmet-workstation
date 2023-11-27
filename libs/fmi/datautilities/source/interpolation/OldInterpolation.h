#pragma once

#include "source/interpolation/Interpolation.h"
#include "source/cache/InterpolatedDataCache.h"
#include "source/cache/CacheUtil.h"

namespace SmartMetDataUtilities
{
    class OldInterpolation : public Interpolation
    {
        NFmiGrid& grid_;
        const NFmiArea& mapArea_;
        InterpolatedDataCache& interpolatedCache_;
        MapViewId keys_;
        std::shared_ptr<NFmiArea> croppedArea_ = nullptr;
        std::shared_ptr<NFmiDataMatrix<NFmiPoint>> interpolatedMatrix_ = nullptr;
    public:
        OldInterpolation(NFmiGrid& grid, const NFmiArea& mapArea, InterpolatedDataCache& interpolatedCache, MapViewId keys)
            :grid_(grid)
            , mapArea_(mapArea)
            , interpolatedCache_(interpolatedCache)
            , keys_(keys)
        {
        }

        ~OldInterpolation();

        virtual void calculate() override
        {
            decltype(auto) interpolatedData = CacheUtil::get(interpolatedCache_, keys_, mapArea_, grid_);
            interpolatedMatrix_ = interpolatedData.getMatrix();
            croppedArea_ = interpolatedData.getArea();
        }

        virtual std::shared_ptr<NFmiArea> getArea() override
        {
            return croppedArea_;
        }
        virtual std::shared_ptr<NFmiDataMatrix<NFmiPoint>> getMatrix() override
        {
            return interpolatedMatrix_;
        }
    };
}
