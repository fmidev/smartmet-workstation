#pragma once

#include "WmsSupport\Query.h"

#include "NFmiRect.h"

#include <type_traits>
#include <string>

namespace TestHelper
{
    namespace BaseQueryDefaults
    {
        static const std::string cgiPath = "/cgi-bin/mapserv";
        static const std::string mapPath = "/var/www/html/mapserver/mapfiles/smartmet/smartmet.map";
        static const std::string service = "WMS";
        static const std::string version = "1.3.0";
    }

    namespace GetMapQueryDefaults
    {
        static const std::string format = "image/png";
        static const std::string crs = "EPSG:620";
        static const unsigned int width = 2020;
        static const unsigned int height = 2265;
        static const NFmiRect bbox = NFmiRect(-1016264.49, -1810793.44, 1003739.19, -4076014.23);
        static const std::vector<std::string> layers = { "jarvet","tiet","merialueet" };
        static const std::string sldPath = "wms-intra2.fmi.fi/mapserver/sld/smartmet.xml";

        static const std::string defaultsAsQuery = "/cgi-bin/mapserv?map=/var/www/html/mapserver/mapfiles/smartmet/smartmet.map&SERVICE=WMS&VERSION=1.3.0&REQUEST=GetMap&FORMAT=image/png&CRS=EPSG:620&WIDTH=2020&HEIGHT=2265&BBOX=-1016264.490000,-4076014.230000,1003739.190000,-1810793.440000&LAYERS=jarvet,tiet,merialueet&SLD=wms-intra2.fmi.fi/mapserver/sld/smartmet.xml";
    }

    template<typename A, typename B>
    inline bool typesMatch(A a, B b)
    {
        return std::is_same<A, B>::value;
    }

    inline void setDefaults(Wms::BaseQuery& baseQuery)
    {
        baseQuery.setCgiPath(BaseQueryDefaults::cgiPath);
        baseQuery.setMapPath(BaseQueryDefaults::mapPath);
        baseQuery.setService(BaseQueryDefaults::service);
        baseQuery.setVersion(BaseQueryDefaults::version);
    }

    inline void setDefaults(Wms::GetMapQuery& getMapQuery)
    {
        getMapQuery.setFormat(GetMapQueryDefaults::format);
        getMapQuery.setCrs(GetMapQueryDefaults::crs);
        getMapQuery.setWidth(GetMapQueryDefaults::width);
        getMapQuery.setHeight(GetMapQueryDefaults::height);
        getMapQuery.setBbox(GetMapQueryDefaults::bbox);
        getMapQuery.setLayers(GetMapQueryDefaults::layers);
        getMapQuery.setSldPath(GetMapQueryDefaults::sldPath);
    }

    inline std::unique_ptr<Wms::GetCapabilitiesQuery> getCapabilitiesWith(std::shared_ptr<Wms::BaseQuery>& baseQuery)
    {
        return std::make_unique<Wms::GetCapabilitiesQuery>(baseQuery);
    }
}