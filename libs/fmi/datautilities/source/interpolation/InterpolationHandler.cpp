#include "source/interpolation/InterpolationHandler.h"
#include "source/newbaseUtil/Intersection.h"
#include "source/cache/CacheUtil.h"
#include "source/interpolation/InterpolationFactory.h"

#include <type_traits>

using namespace std;

namespace SmartMetDataUtilities
{
    InterpolationHandler::InterpolationHandler(NFmiGrid& grid, const NFmiArea& mapArea, RootDataCache& rootCache, InterpolatedDataCache& interpolatedCache, const MapViewId &keys)
        :grid_(grid)
        , mapArea_(mapArea)
        , rootCache_(rootCache)
        , interpolatedCache_(interpolatedCache)
        , keys_(keys)
    {
    }

    void InterpolationHandler::interpolate()
    {
        if(Intersection::gridAndMapAreasIntersect(grid_, mapArea_))
        {
            auto interpolation = InterpolationFactory::create(grid_, mapArea_, interpolatedCache_, rootCache_, keys_);
            interpolation->calculate();
            croppedArea_ = interpolation->getArea();
            interpolatedMatrix_ = interpolation->getMatrix();
            storeInterpolationResult();
        }
    }

    void InterpolationHandler::storeInterpolationResult()
    {
        InterpolatedData dataToStore(interpolatedMatrix_, croppedArea_);
        CacheUtil::put(interpolatedCache_, keys_, mapArea_, move(dataToStore), grid_);
    }

    std::shared_ptr<NFmiArea> InterpolationHandler::getCroppedArea()
    {
        return croppedArea_;
    }

    shared_ptr<Fmi::CoordinateMatrix> InterpolationHandler::getInterpolatedMatrix()
    {
        return interpolatedMatrix_;
    }
}
