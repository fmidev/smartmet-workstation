#pragma once

#include "SingleRowItem.h"
#include "boost/shared_ptr.hpp"
#include <vector>

class NFmiHelpDataInfoSystem;
class NFmiInfoOrganizer;
class NFmiProducerSystem;
class NFmiProducer;
class NFmiMacroParamItem;

namespace AddParams
{
    class ProducerData;

    class CategoryData
    {
		std::string categoryName_;
        NFmiInfoData::Type dataCategory_;
        std::vector<std::unique_ptr<ProducerData>> producerDataVector_;
    public:
        CategoryData(const std::string &categoryName, NFmiInfoData::Type dataCategory);
        ~CategoryData();

        bool updateData(NFmiProducerSystem &categoryProducerSystem, NFmiInfoOrganizer &infoOrganizer, NFmiHelpDataInfoSystem &helpDataInfoSystem, NFmiInfoData::Type dataCategory, std::vector<int> helpDataIDs = {});
        bool updateMacroParamData(std::vector<NFmiMacroParamItem> &macroParamTree, NFmiInfoData::Type dataCategory);
        const std::string& categoryName() const { return categoryName_; }
        const std::vector<std::unique_ptr<ProducerData>>& producerDataVector() const { return producerDataVector_; }
        bool empty() const { return producerDataVector_.empty(); }
        std::vector<SingleRowItem> makeDialogRowData(const std::vector<SingleRowItem> &dialogRowDataMemory) const;
    private:
        void addNewProducerData(const NFmiProducer &producer, NFmiInfoOrganizer &infoOrganizer, NFmiHelpDataInfoSystem &helpDataInfoSystem, NFmiInfoData::Type dataCategory);
        bool addNewOrUpdateData(NFmiProducer producer, NFmiInfoOrganizer &infoOrganizer, NFmiHelpDataInfoSystem &helpDataInfoSystem, NFmiInfoData::Type dataCategory);
        bool addCustomProducerData(const NFmiProducer &producer, NFmiInfoOrganizer &infoOrganizer, NFmiHelpDataInfoSystem &helpDataInfoSystem, NFmiInfoData::Type dataCategory);
    };
}
