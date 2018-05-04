#pragma once

#include "NFmiMetTime.h"

#include <set>
#include <map>
#include <string>

namespace Wms
{
    class LayerInfo
    {
    public:
        std::string name;
        Style style;
        NFmiMetTime startTime;
        NFmiMetTime endTime;
    };

    inline bool operator<(const LayerInfo& info1, const LayerInfo& info2)
    {
        auto tmp1 = info1.name + info1.style.name;
        auto tmp2 = info2.name + info2.style.name;
        return tmp1 < tmp2;
    }

    class ChangedLayers
    {
        std::map<long, std::map<long, std::string>> earlierDataValues_;
        long producerId_;
    public:
        std::set<LayerInfo> changedLayers;

        void setProducerId(long producerId)
        {
            producerId_ = producerId;
        }

        void update(long hashedName, const LayerInfo& info, const std::string& timeWindow)
        {
            if (timeWindow.empty())
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
    };
}