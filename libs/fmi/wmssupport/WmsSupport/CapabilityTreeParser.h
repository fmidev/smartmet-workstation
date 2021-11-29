#pragma once

#include "wmssupport/CapabilityTree.h"
#include "wmssupport/ChangedLayers.h"
#include "NFmiProducer.h"
#include "xmlliteutils/XMLite.h"

#include <cppext/split.h>

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
        CapabilityTreeParser(const NFmiProducer &producer, const std::string &delimiter, std::function<bool(long, const std::string&)> &cacheHitCallback);
		std::unique_ptr<CapabilityTree> parseXmlGeneral(std::string& xml, std::map<long, std::map<long, LayerInfo>>& hashes, ChangedLayers& changedLayers);

	private:
		void addWithPossibleStyles(LPXNode layerNode, std::unique_ptr<CapabilityNode>& subTree, std::list<std::string>& path,
			std::string& timeWindow, ChangedLayers& changedLayers, std::map<long, std::map<long, LayerInfo>>& hashes, std::pair<NFmiMetTime, NFmiMetTime>& startEnd, std::string& name,
			bool hasFlatFmiLayerStructure) const;
		void addWithStyles(std::unique_ptr<CapabilityNode>& subTree, std::list<std::string>& path, std::string& timeWindow, ChangedLayers& changedLayers, std::map<long, std::map<long, LayerInfo>>& hashes, 
			std::pair<NFmiMetTime, NFmiMetTime>& startEnd, std::string& name, std::set<Wms::Style>& styles, bool hasFlatFmiLayerStructure, bool useAlsoAlternateFmiDefaultLayerName) const;
		void ParseCapability(std::unique_ptr<Wms::CapabilityNode>& subTree, LPXNode capabilityNode
			, std::list<std::string>& path, std::map<long, std::map<long, LayerInfo>>& hashes, ChangedLayers& changedLayers);
		void ParseLayer(std::unique_ptr<CapabilityNode>& subTree, LPXNode layerNode, std::list<std::string>& path, 
			std::map<long, std::map<long, LayerInfo>>& hashes, ChangedLayers& changedLayers);
		bool DoPossibleSubLayerParsing(std::unique_ptr<CapabilityNode>& subTree, LPXNode layerNode, std::list<std::string>& path,
			std::map<long, std::map<long, LayerInfo>>& hashes, ChangedLayers& changedLayers);
		void ParseLeafLayer(std::unique_ptr<CapabilityNode>& subTree, LPXNode layerNode, std::list<std::string>& path,
			std::map<long, std::map<long, LayerInfo>>& hashes, ChangedLayers& changedLayers);
		bool HasFlatWmsStructure(LPXNode layerNode, std::list<std::string>& path);
		bool DoGeneralPathHandling(LPXNode layerNode, std::list<std::string>& path);
	};
}