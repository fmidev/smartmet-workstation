#include "source/dataUtil/DataUtilitiesAdapterPimpl.h"
#include "source/dataUtil/MapViewId.h"
#include "source/error/Error.h"

using namespace std;

namespace SmartMetDataUtilities
{
    struct DataUtilitiesAdapterPimpl::Members
    {
        bool toolmasterAvailable_ = false;
        MapViewId keys_;
    };

    DataUtilitiesAdapterPimpl::DataUtilitiesAdapterPimpl(Toolmaster toolmasterAvailability, unsigned int mapWiewIndex, const std::string& fileFilter)
        :members_(make_unique<Members>())
    {
        setToolmasterAvailability(move(toolmasterAvailability));
        members_->keys_ = MapViewId(mapWiewIndex, fileFilter);
    }

    DataUtilitiesAdapterPimpl::DataUtilitiesAdapterPimpl()
        :members_(make_unique<Members>())
    {
    }

    DataUtilitiesAdapterPimpl::~DataUtilitiesAdapterPimpl()
    {
    }
    DataUtilitiesAdapterPimpl::DataUtilitiesAdapterPimpl(const DataUtilitiesAdapterPimpl &rhs)
        :members_(make_unique<Members>())
    {
        members_->keys_ = rhs.members_->keys_;
        members_->toolmasterAvailable_ = rhs.members_->toolmasterAvailable_;
    }
    DataUtilitiesAdapterPimpl& DataUtilitiesAdapterPimpl::DataUtilitiesAdapterPimpl::operator=(DataUtilitiesAdapterPimpl rhs)
    {
        swap(*this, rhs);
        return *this;
    }
    DataUtilitiesAdapterPimpl::DataUtilitiesAdapterPimpl(DataUtilitiesAdapterPimpl&& rhs)
        : DataUtilitiesAdapterPimpl()
    {
        swap(*this, rhs);
    }
    DataUtilitiesAdapterPimpl& DataUtilitiesAdapterPimpl::operator=(DataUtilitiesAdapterPimpl&& rhs)
    {
        swap(*this, rhs);
        return *this;
    }

    void swap(DataUtilitiesAdapterPimpl& first, DataUtilitiesAdapterPimpl& second) noexcept
    {
        using std::swap;
        swap(first.members_->toolmasterAvailable_, second.members_->toolmasterAvailable_);
        swap(first.members_->keys_, second.members_->keys_);
    }

    bool DataUtilitiesAdapterPimpl::toolmasterAvailable() const
    {
        return members_->toolmasterAvailable_;
    }

    MapViewId& DataUtilitiesAdapterPimpl::mapViewId() const
    {
        return members_->keys_;
    }

    void DataUtilitiesAdapterPimpl::setToolmasterAvailability(Toolmaster toolmasterAvailability)
    {
        members_->toolmasterAvailable_ = toolmasterAvailability == Toolmaster::Available;
    }

    void DataUtilitiesAdapterPimpl::setMapViewIndex(unsigned int mapWiewIndex)
    {
        members_->keys_ = MapViewId(mapWiewIndex, members_->keys_.getFileFilter());
    }

    void DataUtilitiesAdapterPimpl::setFileFilter(const std::string& fileFilter)
    {
        members_->keys_ = MapViewId(members_->keys_.getMapViewIndex(), fileFilter);
    }
}