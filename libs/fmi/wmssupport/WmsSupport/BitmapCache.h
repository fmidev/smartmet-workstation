#pragma once

#include "wmssupport/WmsQuery.h"

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
    std::string toKey1(const WmsQuery& query);
    std::string toKey2(const WmsQuery& query);

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
