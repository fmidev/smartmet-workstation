#pragma once
#include <string>

namespace SmartMetDataUtilities
{
    class MapViewId
    {
        unsigned int mapViewIndex_;
        std::string fileFilter_;
    public:
        MapViewId(unsigned int mapViewIndex, std::string fileFilter)
            :mapViewIndex_(mapViewIndex), fileFilter_(fileFilter) {}

        unsigned int getMapViewIndex() const
        {
            return mapViewIndex_;
        }

        std::string getFileFilter() const
        {
            return fileFilter_;
        }

        MapViewId() = default;
        MapViewId(const MapViewId& rhs) = default;
        MapViewId& MapViewId::operator=(MapViewId& rhs) = default;
        MapViewId(MapViewId&& rhs) = default;
        MapViewId& operator=(MapViewId&& rhs) = default;
    };
}
