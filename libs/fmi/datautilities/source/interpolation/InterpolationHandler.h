#pragma once
#include "source/cache/InterpolatedDataCache.h"
#include "source/cache/RootDataCache.h"
#include "source/dataUtil/MapViewId.h"

#include "NFmiArea.h"
#include "CoordinateMatrix.h"
#include "NFmiPoint.h"

#include <memory>

namespace SmartMetDataUtilities
{
    class InterpolationHandler
    {
        NFmiGrid& grid_;
        const NFmiArea& mapArea_;
        RootDataCache& rootCache_;
        InterpolatedDataCache& interpolatedCache_;
        MapViewId keys_;

        std::shared_ptr<NFmiArea> croppedArea_ = nullptr;
        std::shared_ptr<Fmi::CoordinateMatrix> interpolatedMatrix_ = nullptr;
    public:
        InterpolationHandler(NFmiGrid& grid, const NFmiArea& mapArea, RootDataCache& rootCache, InterpolatedDataCache& interpolatedCache, const MapViewId &keys);
        InterpolationHandler() = default;
        ~InterpolationHandler() = default;

        void interpolate();
        std::shared_ptr<NFmiArea> getCroppedArea();
        std::shared_ptr<Fmi::CoordinateMatrix> getInterpolatedMatrix();
    private:
        void storeInterpolationResult();
    };
}
