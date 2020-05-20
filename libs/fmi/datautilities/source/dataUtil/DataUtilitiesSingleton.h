#pragma once
#include "source/interpolation/InterpolationHandler.h"
#include "source/dataUtil/MapViewId.h"
#include "source/cache/InterpolatedDataCache.h"
#include "source/cache/RootDataCache.h"

#include "CoordinateMatrix.h"
#include "NFmiFastQueryInfo.h"
#include "NFmiGrid.h"
#include "NFmiArea.h"

namespace SmartMetDataUtilities
{
    class MapViewId;
}

class NFmiFastQueryInfo;

namespace SmartMetDataUtilities
{
    class DataUtilitiesSingleton
    {
        bool modifiedDataDrawingPossible_ = false;
        bool isThereAnythingToDraw_ = false;
        std::string originalDataResolution_;
        std::shared_ptr<NFmiArea> croppedArea_ = nullptr;
        std::shared_ptr<Fmi::CoordinateMatrix> interpolatedMatrix_ = nullptr;
        mutable RootDataCache rootDataCache_;
        mutable InterpolatedDataCache interpolatedDataCache_;
    public:
        static DataUtilitiesSingleton& instance()
        {
            static DataUtilitiesSingleton singleton;
            return singleton;
        }
        void initializeDrawingData(const NFmiFastQueryInfo& data, const NFmiArea& mapArea, const MapViewId& keys);
        bool isModifiedDataDrawingPossible(const MapViewId& keys) const;
        bool isThereAnythingToDraw(const MapViewId& keys) const;
        std::shared_ptr<NFmiArea> getCroppedArea() const;
        std::shared_ptr<Fmi::CoordinateMatrix> getInterpolatedData() const;
        // only for testing purposes
        std::shared_ptr<NFmiGrid> getRootGrid(const MapViewId& keys) const;
        void clear();

        DataUtilitiesSingleton(const DataUtilitiesSingleton& rhs) = delete;
        DataUtilitiesSingleton& DataUtilitiesSingleton::operator=(DataUtilitiesSingleton& rhs) = delete;
        DataUtilitiesSingleton(DataUtilitiesSingleton&& rhs) = delete;
        DataUtilitiesSingleton& operator=(DataUtilitiesSingleton&& rhs) = delete;
    protected:
        DataUtilitiesSingleton() = default;
        ~DataUtilitiesSingleton() = default;
    private:
        void interpolate(NFmiGrid& grid, const NFmiArea& mapArea, const MapViewId& keys);
        void initFlags(const NFmiFastQueryInfo& data, const NFmiArea& mapArea);
        void setFlags();
    };
}
