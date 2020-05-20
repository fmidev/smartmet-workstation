#pragma once
#include "source/interpolation/Interpolation.h"
#include "source/cache/RootDataCache.h"
#include "source/rootData/RootDataCropper.h"
#include "source/cache/CacheUtil.h"
#include "source/newbaseUtil/AreaUtil.h"
#include "CoordinateMatrix.h"
#include "NFmiGrid.h"
#include "NFmiArea.h"
#include "NFmiPoint.h"
namespace SmartMetDataUtilities
{
    class CroppedInterpolation : public Interpolation
    {
        NFmiGrid& grid_;
        const NFmiArea& mapArea_;
        RootDataCache& rootCache_;
        MapViewId keys_;
        std::shared_ptr<NFmiArea> croppedArea_ = nullptr;
        std::shared_ptr<Fmi::CoordinateMatrix> interpolatedMatrix_ = nullptr;
    public:
        CroppedInterpolation(NFmiGrid& grid, const NFmiArea& mapArea, RootDataCache& rootCache, MapViewId keys)
            :grid_(grid)
            , mapArea_(mapArea)
            , rootCache_(rootCache)
            , keys_(keys)
        {
        }

        virtual void calculate() override
        {
            auto area = AreaUtil::createCopyOf(mapArea_);
            AreaUtil::normalizeArea(*area);
            decltype(auto) rootData = CacheUtil::get(rootCache_, keys_, *area, grid_);
            decltype(auto) rootCroppedArea = rootData.getGrid()->Area();
            if(area->IsInside(*rootCroppedArea))
            {
                interpolatedMatrix_ = rootData.getMatrix();
                croppedArea_ = std::shared_ptr<NFmiArea>(AreaUtil::createCopyOf(*rootCroppedArea));
            }
            else
            {
                RootDataCropper rootDataCropper(*area, rootData);
                interpolatedMatrix_ = rootDataCropper.getCroppedRootMatrix();
                croppedArea_ = rootDataCropper.getCroppedRootArea();
            }
        }

        virtual std::shared_ptr<NFmiArea> getArea() override
        {
            return croppedArea_;
        }
        virtual std::shared_ptr<Fmi::CoordinateMatrix> getMatrix() override
        {
            return interpolatedMatrix_;
        }
    };
}
