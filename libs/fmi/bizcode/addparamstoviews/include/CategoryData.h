#pragma once

#include "SingleRowItem.h"
#include "boost/shared_ptr.hpp"
#include <vector>
#include "NFmiParamBag.h"
#include "NFmiLevelBag.h"

class NFmiHelpDataInfoSystem;
class NFmiInfoOrganizer;
class NFmiProducerSystem;
class NFmiProducer;
class NFmiMacroParamItem;
namespace cppext
{
	template <typename> class Tree;
	template <typename> class Node;
};
namespace Wms
{
	struct Capability;
	using CapabilityTree = cppext::Tree<Capability>;
};


namespace AddParams
{
    class ProducerData;

    class CategoryData
    {
		std::string categoryName_;
        NFmiInfoData::Type dataCategory_;
        std::vector<std::unique_ptr<ProducerData>> producerDataVector_;
        const NFmiLevelBag* soundingLevels_;
		bool staticDataAdded_;

    public:
        CategoryData(const std::string &categoryName, NFmiInfoData::Type dataCategory);
        ~CategoryData();

        bool updateData(NFmiProducerSystem &categoryProducerSystem, NFmiInfoOrganizer &infoOrganizer, NFmiHelpDataInfoSystem &helpDataInfoSystem, NFmiInfoData::Type dataCategory, 
            const std::vector<int> &helpDataIDs = {}, bool customCategory = false);
        bool updateNormalData(NFmiProducerSystem &categoryProducerSystem, NFmiInfoOrganizer &infoOrganizer, NFmiHelpDataInfoSystem &helpDataInfoSystem, NFmiInfoData::Type dataCategory, const std::vector<int> &helpDataIDs);
        bool updateCustomCategoryData(NFmiProducerSystem &categoryProducerSystem, NFmiInfoOrganizer &infoOrganizer, NFmiHelpDataInfoSystem &helpDataInfoSystem, NFmiInfoData::Type dataCategory);
        bool updateOperationalData(NFmiInfoOrganizer &infoOrganizer, NFmiHelpDataInfoSystem &helpDataInfoSystem, NFmiInfoData::Type dataCategory);
        bool updateMacroParamData(std::vector<NFmiMacroParamItem> &macroParamTree, NFmiInfoData::Type dataCategory);
		bool updateWmsData(const cppext::Node<Wms::Capability>& wmsLayerTree, NFmiInfoData::Type dataCategory);
		const std::string& categoryName() const { return categoryName_; }
        const std::vector<std::unique_ptr<ProducerData>>& producerDataVector() const { return producerDataVector_; }
        bool empty() const { return producerDataVector_.empty(); }
        std::vector<SingleRowItem> makeDialogRowData(const std::vector<SingleRowItem> &dialogRowDataMemory, NFmiInfoOrganizer &infoOrganizer, bool mapViewCase) const;
        NFmiInfoData::Type getDataType(NFmiInfoOrganizer &infoOrganizer, const NFmiProducer &producer);
        void setSoungindLevels(const NFmiLevelBag& soundingLevels);
        static std::string GetEditableDataRowName();
    private:
        std::vector<SingleRowItem> customObservationData(NFmiInfoOrganizer &infoOrganizer) const;
        void addNewProducerData(const NFmiProducer &producer, NFmiInfoOrganizer &infoOrganizer, NFmiHelpDataInfoSystem &helpDataInfoSystem, NFmiInfoData::Type dataCategory);
        bool addNewOrUpdateData(const NFmiProducer &producer, NFmiInfoOrganizer &infoOrganizer, NFmiHelpDataInfoSystem &helpDataInfoSystem, NFmiInfoData::Type dataCategory, bool customCategory = false);
        bool skipCustomProducerData(const NFmiProducer &producer, NFmiInfoOrganizer &infoOrganizer, NFmiHelpDataInfoSystem &helpDataInfoSystem, NFmiInfoData::Type dataCategory);
		bool addStaticData(NFmiInfoOrganizer& infoOrganizer, NFmiHelpDataInfoSystem& helpDataInfoSystem, NFmiInfoData::Type dataCategory);
	};
}
