#include "wmssupport/ChangedLayers.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

namespace Wms
{
    void LayerInfo::setTimeDimensions(const NFmiMetTime& start, const NFmiMetTime& end)
    {
        startTime = start;
        endTime = end;
        hasTimeDimension = (startTime != NFmiMetTime::gMissingTime) && (endTime != NFmiMetTime::gMissingTime);
    }

    bool operator<(const LayerInfo& info1, const LayerInfo& info2)
    {
        auto tmp1 = info1.name + info1.style.name;
        auto tmp2 = info2.name + info2.style.name;
        return tmp1 < tmp2;
    }

    void ChangedLayers::update(long hashedName, const LayerInfo& info, const std::string& timeWindow)
    {
        if(timeWindow.empty())
        {
            return;
        }
        auto res = earlierDataValues_.find(producerId_);
        if(res != earlierDataValues_.cend())
        {
            auto res2 = res->second.find(hashedName);
            if(res2 != res->second.cend())
            {
                if(res2->second != timeWindow)
                {
                    changedLayers.insert(info);
                }
            }
        }
        earlierDataValues_[producerId_][hashedName] = timeWindow;
    }
}
