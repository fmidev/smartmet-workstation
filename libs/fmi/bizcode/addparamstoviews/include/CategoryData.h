#pragma once

#include "SingleRowItem.h"
#include "boost/shared_ptr.hpp"
#include <vector>

class NFmiHelpDataInfoSystem;
class NFmiInfoOrganizer;
class NFmiProducerSystem;
class NFmiProducer;

namespace AddParams
{
    class ProducerData;

    class CategoryData
    {
		std::string categoryName_;
        std::vector<std::unique_ptr<ProducerData>> producerDataVector_;
    public:
        CategoryData(const std::string &categoryName);
        ~CategoryData();

        bool updateData(NFmiProducerSystem &categoryProducerSystem, NFmiInfoOrganizer &infoOrganizer, NFmiHelpDataInfoSystem &helpDataInfoSystem);
        const std::string& categoryName() const { return categoryName_; }
        const std::vector<std::unique_ptr<ProducerData>>& producerDataVector() const { return producerDataVector_; }
        bool empty() const { return producerDataVector_.empty(); }
        std::vector<SingleRowItem> makeDialogRowData(const std::vector<SingleRowItem> &dialogRowDataMemory) const;
    private:
        void addNewProducerData(const NFmiProducer &producer, NFmiInfoOrganizer &infoOrganizer, NFmiHelpDataInfoSystem &helpDataInfoSystem);
    };
}
