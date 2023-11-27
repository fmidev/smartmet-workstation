#pragma once
#include "source/dataUtil/DataUtilitiesAdapterPimpl.h"
#include "source/dataUtil/DataUtilitiesSingleton.h"
#include "source/error/Error.h"

#include <memory>

namespace SmartMetDataUtilities
{
    class MapViewId;
}

class NFmiArea;
class NFmiFastQueryInfo;
class NFmiPoint;
class NFmiGrid;
template <typename A>
class NFmiDataMatrix;

namespace SmartMetDataUtilities
{
#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable : 4348 ) // En tied‰ miten estet‰‰n "redefinition of default parameter" varoitus t‰ss‰ tilanteessa (aiheutuu "typename InterpolatorType = DataUtilitiesSingleton" kohdasta)
#endif

    template<typename InterpolatorType = DataUtilitiesSingleton>
    class DataUtilitiesAdapter : public DataUtilitiesAdapterPimpl
    {
    public:
        DataUtilitiesAdapter() = default;
        DataUtilitiesAdapter(Toolmaster toolmasterAvailability, unsigned int mapWiewIndex, const std::string& fileFilter);
        ~DataUtilitiesAdapter() = default;
        void clear();

        void initializeDrawingData(const NFmiFastQueryInfo& data, const NFmiArea& mapArea);

        bool isModifiedDataDrawingPossible();
        bool isThereAnythingToDraw();
        std::shared_ptr<NFmiArea> getCroppedArea();
        std::shared_ptr<NFmiDataMatrix<NFmiPoint>> getInterpolatedData();

        // only for testing
        inline std::shared_ptr<NFmiGrid> getRootGrid() const;
    };

#ifdef _MSC_VER
#pragma warning( pop )
#endif

    template<typename InterpolatorType>
    DataUtilitiesAdapter<InterpolatorType>::DataUtilitiesAdapter(Toolmaster toolmasterAvailability, unsigned int mapWiewIndex, const std::string& fileFilter)
    : DataUtilitiesAdapterPimpl(std::move(toolmasterAvailability), std::move(mapWiewIndex), std::move(fileFilter))
    {
    }

    template<typename InterpolatorType>
    void DataUtilitiesAdapter<InterpolatorType>::clear()
    {
        if(toolmasterAvailable())
        {
            InterpolatorType::instance().clear();
        }
    }

    template<typename InterpolatorType>
    void DataUtilitiesAdapter<InterpolatorType>::initializeDrawingData(const NFmiFastQueryInfo& data, const NFmiArea& mapArea)
    {
        if(toolmasterAvailable())
        {
            InterpolatorType::instance().initializeDrawingData(data, mapArea, mapViewId());
        }
    }

    template<typename InterpolatorType>
    bool DataUtilitiesAdapter<InterpolatorType>::isModifiedDataDrawingPossible()
    {
        if(toolmasterAvailable())
        {
            return InterpolatorType::instance().isModifiedDataDrawingPossible(mapViewId());
        }
        return false;
    }

    template<typename InterpolatorType>
    bool DataUtilitiesAdapter<InterpolatorType>::isThereAnythingToDraw()
    {
        if(toolmasterAvailable())
        {
            return InterpolatorType::instance().isThereAnythingToDraw(mapViewId());
        }
        return false;
    }

    template<typename InterpolatorType>
    std::shared_ptr<NFmiArea> DataUtilitiesAdapter<InterpolatorType>::getCroppedArea()
    {
        if(toolmasterAvailable())
        {
            return InterpolatorType::instance().getCroppedArea();
        }
        throw ToolmasterNotAvailable("Check if modified data drawing is possible before calling getCroppedArea.");
    }

    template<typename InterpolatorType>
    std::shared_ptr<NFmiDataMatrix<NFmiPoint>> DataUtilitiesAdapter<InterpolatorType>::getInterpolatedData()
    {
        if(toolmasterAvailable())
        {
            return InterpolatorType::instance().getInterpolatedData();
        }
        throw ToolmasterNotAvailable("Check if modified data drawing is possible before calling getInterpolatedData.");
    }

    template<typename InterpolatorType>
    inline std::shared_ptr<NFmiGrid> DataUtilitiesAdapter<InterpolatorType>::getRootGrid() const
    {
        if(toolmasterAvailable())
        {
            return InterpolatorType::instance().getRootGrid(mapViewId());
        }
    }
}
