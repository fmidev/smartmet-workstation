#pragma once

#include "WmsQuery.h"

#include "NFmiSatelliteImageCacheHelpers.h"

#include <string>
#include <set>

namespace Wms
{
    class WmsQuery;
    class Setup;
}

namespace Wms
{
    inline
        std::string toKey1(const WmsQuery& query)
    {
        return query.query.at("LAYERS");
    }

    inline
        std::string toKey2(const WmsQuery& query)
    {
        auto key = query.query.at("BBOX") + query.query.at("CRS") + query.query.at("WIDTH") + query.query.at("HEIGHT");
        auto res = query.query.find("TIME");
        if(res != query.query.cend())
        {
            key += res->second;
        }
        return key;
    }

    class BitmapCache
    {
    public:
        virtual bool exists(const std::string& key1, const std::string& key2) = 0;
        virtual bool exists(const std::string& key1) = 0;
        virtual NFmiImageHolder get(const std::string& key1, const std::string& key2) = 0;
        virtual void put(const std::string& key1, const std::string& key2, NFmiImageHolder imageHolder) = 0;
        virtual void erase(const std::string& query) = 0;
    };
}
