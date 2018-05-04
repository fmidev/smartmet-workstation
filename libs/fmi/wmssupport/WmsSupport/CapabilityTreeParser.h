#pragma once

#include "CapabilityTree.h"
#include "ChangedLayers.h"

#include "NFmiProducer.h"

#include <cppext/split.h>

#include <boost/property_tree/ptree.hpp>

#include <map>
#include <memory>
#include <set>
#include <list>

namespace Wms
{
    class CapabilityTreeParser
    {
        NFmiProducer producer_;
        std::string delimiter_;
        std::function<bool(long, const std::string&)> cacheHitCallback_;
    public:
        CapabilityTreeParser(NFmiProducer producer, std::string delimiter, std::function<bool(long, const std::string&)> cacheHitCallback);

        std::unique_ptr<CapabilityTree> parse(const boost::property_tree::ptree& layerTree, std::map<long, std::map<long, LayerInfo>>& hashes, ChangedLayers& changedLayers) const;
    };
}