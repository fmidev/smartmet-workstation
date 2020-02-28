#pragma once

#include "wmssupport/CapabilityTree.h"
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

    bool operator<(const LayerInfo& info1, const LayerInfo& info2);

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

        void update(long hashedName, const LayerInfo& info, const std::string& timeWindow);
    };
}