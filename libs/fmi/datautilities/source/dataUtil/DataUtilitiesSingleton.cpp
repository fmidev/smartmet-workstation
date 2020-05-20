#include "source/dataUtil/DataUtilitiesSingleton.h"
#include "source/interpolation/InterpolationHandler.h"
#include "source/error/Error.h"
#include "source/dataUtil/MapViewId.h"
#include "source/cache/InterpolatedDataCache.h"
#include "source/cache/RootDataCache.h"
#include "source//newbaseUtil/AreaUtil.h"

#include "NFmiFastQueryInfo.h"
#include "NFmiQueryDataUtil.h"
#include "NFmiAreaFactory.h"
#include "NFmiGrid.h"
#include "NFmiArea.h"

using namespace std;

namespace SmartMetDataUtilities
{
    namespace
    {
        template <typename Lambda>
        void handleExceptions(Lambda&& lambda)
        {
            try
            {
                lambda();
            }
            catch(const exception &)
            {
                // loggaa
            }
        }

        bool shouldInterpolateGivenData(const NFmiFastQueryInfo &data, const NFmiArea &mapArea)
        {
            return data.IsGrid() 
                && (!NFmiQueryDataUtil::AreAreasSameKind(data.Area(), &mapArea)
                    || AreaUtil::isPacific(*data.Area()) && !AreaUtil::isPacific(mapArea)
                    || !AreaUtil::isPacific(*data.Area()) && AreaUtil::isPacific(mapArea));
        }

        NFmiGrid* getGridFrom(const NFmiFastQueryInfo &data)
        {
            return const_cast<NFmiGrid*>(data.Grid());
        }
    }

    void DataUtilitiesSingleton::clear()
    {
        modifiedDataDrawingPossible_ = false;
        isThereAnythingToDraw_ = false;
        croppedArea_ = nullptr;
        interpolatedMatrix_ = nullptr;
        rootDataCache_.clear();
        interpolatedDataCache_.clear();
    }

    void DataUtilitiesSingleton::initializeDrawingData(const NFmiFastQueryInfo& data, const NFmiArea& mapArea, const MapViewId& keys)
    {
        initFlags(data, mapArea);
        handleExceptions([&]
        {
            if(modifiedDataDrawingPossible_)
            {
                auto& grid = *getGridFrom(data);
                interpolate(grid, mapArea, keys);
                setFlags();
            }
        });
    }

    void DataUtilitiesSingleton::initFlags(const NFmiFastQueryInfo& data, const NFmiArea& mapArea)
    {
        modifiedDataDrawingPossible_ = shouldInterpolateGivenData(data, mapArea);
        isThereAnythingToDraw_ = false;
    }

    void DataUtilitiesSingleton::setFlags()
    {
        isThereAnythingToDraw_ = interpolatedMatrix_ && InterpolatedData::matrixIsNotEmpty(*interpolatedMatrix_);
    }

    void DataUtilitiesSingleton::interpolate(NFmiGrid& grid, const NFmiArea& mapArea, const MapViewId& keys)
    {
        InterpolationHandler handler(grid, mapArea, rootDataCache_, interpolatedDataCache_, keys);
        handler.interpolate();
        croppedArea_ = handler.getCroppedArea();
        interpolatedMatrix_ = handler.getInterpolatedMatrix();
        originalDataResolution_ = GridUtil::resolutionOf(grid);
    }

    bool DataUtilitiesSingleton::isModifiedDataDrawingPossible(const MapViewId& keys) const
    {
        return modifiedDataDrawingPossible_;
    }

    bool DataUtilitiesSingleton::isThereAnythingToDraw(const MapViewId& keys) const
    {
        return isThereAnythingToDraw_;
    }

    shared_ptr<NFmiArea> DataUtilitiesSingleton::getCroppedArea() const
    {
        if(modifiedDataDrawingPossible_)
        {
            return croppedArea_;
        }
        throw ToolmasterNotAvailable("Check if modified data drawing is possible before calling getInterpolatedData.");
    }

    shared_ptr<Fmi::CoordinateMatrix> DataUtilitiesSingleton::getInterpolatedData() const
    {
        if(modifiedDataDrawingPossible_)
        {
            return interpolatedMatrix_;
        }
        throw ToolmasterNotAvailable("Check if modified data drawing is possible before calling getInterpolatedData.");
    }

    // Only for testing purposes
    shared_ptr<NFmiGrid> DataUtilitiesSingleton::getRootGrid(const MapViewId& id) const
    {
        if(croppedArea_)
        {
            auto rootDataCacheKeys = RootDataCache::createKeysFrom(id.getMapViewIndex(), id.getFileFilter(), *croppedArea_, originalDataResolution_);
            try
            {
                auto rootData = rootDataCache_.get(rootDataCacheKeys);
                return rootData.getGrid();
            }
            catch(const DataNotFoundException&)
            {
            }
        }
        return nullptr;
    }
}
