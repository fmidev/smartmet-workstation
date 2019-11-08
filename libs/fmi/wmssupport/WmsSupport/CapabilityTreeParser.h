#pragma once

#include "CapabilityTree.h"
#include "ChangedLayers.h"
#include "NFmiProducer.h"
#include "xmlliteutils/XMLite.h"

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
		std::unique_ptr<CapabilityTree> parseXml(XNode& xmlNode, std::map<long, std::map<long, LayerInfo>>& hashes, ChangedLayers& changedLayers);

	private:
		void addWithPossibleStyles(const std::pair<const std::string, boost::property_tree::ptree>& layerKV, std::unique_ptr<CapabilityNode>& subTree, std::list<std::string>& path, 
			std::string& timeWindow, ChangedLayers& changedLayers, std::map<long, std::map<long, LayerInfo>>& hashes, std::pair<NFmiMetTime, NFmiMetTime>& startEnd, std::string& name) const;
		void parseNodes(std::unique_ptr<Wms::CapabilityNode>& subTree, const std::pair<const std::string, boost::property_tree::ptree>& layerKV, std::list<std::string>& path, 
			std::map<long, std::map<long, LayerInfo>>& hashes, ChangedLayers& changedLayers) const;
	};
}