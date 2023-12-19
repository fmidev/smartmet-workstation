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
	class DimensionTimeData
	{
	public:
		bool hasTimeDimension() const;

		std::string nodeName_;
		std::string timeWindow_;
		NFmiMetTime startTime_ = NFmiMetTime::gMissingTime;
		NFmiMetTime endTime_ = NFmiMetTime::gMissingTime;
	};

    class CapabilityTreeParser
    {
        NFmiProducer producer_;
        std::string delimiter_;
        std::function<bool(long, const std::string&)> cacheHitCallback_;
		bool acceptTimeDimensionalLayersOnly_;

    public:
        CapabilityTreeParser(const NFmiProducer &producer, const std::string &delimiter, std::function<bool(long, const std::string&)> &cacheHitCallback, bool acceptTimeDimensionalLayersOnly);
		std::unique_ptr<CapabilityTree> parseXmlGeneral(std::string& xml, std::map<long, std::map<long, LayerInfo>>& hashes, ChangedLayers& changedLayers);

	private:
		void addWithPossibleStyles(LPXNode layerNode, std::unique_ptr<CapabilityNode>& subTree, std::list<std::string>& path,
			ChangedLayers& changedLayers, std::map<long, std::map<long, LayerInfo>>& hashes, DimensionTimeData & dimensionTimeData, std::string& name,
			bool hasFlatFmiLayerStructure) const;
		void addWithStyles(std::unique_ptr<CapabilityNode>& subTree, std::list<std::string>& path, ChangedLayers& changedLayers, std::map<long, std::map<long, LayerInfo>>& hashes, 
			DimensionTimeData& dimensionTimeData, std::string& name, std::set<Wms::Style>& styles, bool hasFlatFmiLayerStructure, bool useAlsoAlternateFmiDefaultLayerName) const;
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
		DimensionTimeData getDimensionTimeData(LPXNode layerNode);
	};
}