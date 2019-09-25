#pragma once

#include "NFmiProducer.h"
#include "SingleRowItem.h"
#include "boost/shared_ptr.hpp"
#include <vector>

class NFmiHelpDataInfoSystem;
class NFmiHelpDataInfo;
class NFmiInfoOrganizer;
class NFmiFastQueryInfo;
class NFmiMacroParamItem;
namespace cppext
{
	template <typename> class Node;
};
namespace Wms
{
	struct Capability;
};

namespace AddParams
{
    class SingleData;

    class ProducerData
    {
		NFmiProducer producer_;
        NFmiInfoData::Type dataCategory_;  //This distinguish model, observation, satellite, etc. datatypes
        std::vector<std::unique_ptr<SingleData>> dataVector_;
        std::vector<std::unique_ptr<SingleRowItem>> satelliteDataVector_; // Satellite data doesn't have queryData, so it's handled differently. Biggest difference is that it skips singleData creation.
        std::vector<std::unique_ptr<SingleRowItem>> macroParamDataVector_;
		std::vector<std::unique_ptr<SingleRowItem>> wmsDataVector_;


    public:
        ProducerData(const NFmiProducer &producer, NFmiInfoData::Type dataCategory);
        ~ProducerData();

        bool updateData(NFmiInfoOrganizer &infoOrganizer, NFmiHelpDataInfoSystem &helpDataInfoSystem, NFmiInfoData::Type dataType = NFmiInfoData::kNoDataType);
        bool updateOperationalData(const boost::shared_ptr<NFmiFastQueryInfo> &info, NFmiHelpDataInfoSystem &helpDataInfoSystem);
        bool updateMacroParamData(std::vector<NFmiMacroParamItem> &macroParamTree);
		bool updateWmsData(const cppext::Node<Wms::Capability>& wmsLayerTree);
		const NFmiProducer& producer() const { return producer_; }
        const std::vector<std::unique_ptr<SingleData>>& dataVector() const { return dataVector_; }
        bool empty() const { return dataVector_.empty(); }
        std::vector<SingleRowItem> makeDialogRowData(const std::vector<SingleRowItem> &dialogRowDataMemory) const;
        std::string makeUniqueProducerIdString() const;
    private:
        bool updateData(const boost::shared_ptr<NFmiFastQueryInfo> &info, NFmiInfoOrganizer &infoOrganizer, NFmiHelpDataInfoSystem &helpDataInfoSystem);
        bool updateSatelliteData(NFmiHelpDataInfoSystem &helpDataInfoSystem);
        void addNewSingleData(const boost::shared_ptr<NFmiFastQueryInfo> &info, NFmiHelpDataInfoSystem &helpDataInfoSystem);
        std::vector<SingleRowItem> makeDialogRowData(const std::vector<SingleRowItem> &dialogRowDataMemory, const std::vector<std::unique_ptr<SingleRowItem>> &thisDataVector) const;
        int macroParamsToVector(std::vector<std::unique_ptr<SingleRowItem>> &macroParamVector, const std::vector<NFmiMacroParamItem> &macroParamTree, int treeDepth);
		int wmsDataToVector(std::vector<std::unique_ptr<SingleRowItem>>& wmsVector, const cppext::Node<Wms::Capability>& wmsLayerTree, int treeDepth);
		std::vector<std::unique_ptr<SingleRowItem>> createMacroParamVector(const std::vector<NFmiMacroParamItem>& macroParamTree, int treeDepth);
		std::vector<std::unique_ptr<SingleRowItem>> createWmsDataVector(const cppext::Node<Wms::Capability>& wmsLayerTree, int treeDepth);
	};
}
