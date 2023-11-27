#include "wmssupport/BitmapCache.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

namespace Wms
{
    std::string toKey1(const WmsQuery& query)
    {
        return query.query.at("LAYERS");
    }

    std::string toKey2(const WmsQuery& query)
    {
        auto key = query.query.at("BBOX");

        auto resCrs = query.query.find("CRS");
        if(resCrs != query.query.cend())
        {
            key += resCrs->second;
        }
        else
            key += query.query.at("sRS");

        key += query.query.at("STYLES") + query.query.at("WIDTH") + query.query.at("HEIGHT");
        auto res = query.query.find("TIME");
        if(res != query.query.cend())
        {
            key += res->second;
        }
        return key;
    }

}
