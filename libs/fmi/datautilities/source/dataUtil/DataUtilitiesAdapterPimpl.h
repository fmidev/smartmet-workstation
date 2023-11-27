#pragma once
#include <memory>
#include <string>

namespace SmartMetDataUtilities
{
    class MapViewId;
}

namespace SmartMetDataUtilities
{
    enum class Toolmaster
    {
        Available,
        NotAvailable
    };

    class DataUtilitiesAdapterPimpl
    {
    public:
        struct Members;
        std::unique_ptr<Members> members_;

        DataUtilitiesAdapterPimpl(Toolmaster toolmasterAvailability, unsigned int mapWiewIndex, const std::string& fileFilter);
        DataUtilitiesAdapterPimpl();
        virtual ~DataUtilitiesAdapterPimpl();
        DataUtilitiesAdapterPimpl(const DataUtilitiesAdapterPimpl& rhs);
        DataUtilitiesAdapterPimpl& DataUtilitiesAdapterPimpl::operator=(DataUtilitiesAdapterPimpl rhs);
        DataUtilitiesAdapterPimpl(DataUtilitiesAdapterPimpl&& rhs);
        DataUtilitiesAdapterPimpl& operator=(DataUtilitiesAdapterPimpl&& rhs);

        friend void swap(DataUtilitiesAdapterPimpl& first, DataUtilitiesAdapterPimpl& second) noexcept;

        bool toolmasterAvailable() const;
        MapViewId& mapViewId() const;

        void setToolmasterAvailability(Toolmaster toolmasterAvailability);
        void setMapViewIndex(unsigned int mapWiewIndex);
        void setFileFilter(const std::string& fileFilter);
    };
}
