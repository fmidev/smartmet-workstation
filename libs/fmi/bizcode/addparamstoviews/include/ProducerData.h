#pragma once

#include "NFmiProducer.h"
#include "SingleRowItem.h"
#include "boost/shared_ptr.hpp"
#include <vector>

class NFmiHelpDataInfoSystem;
class NFmiInfoOrganizer;
class NFmiFastQueryInfo;

namespace AddParams
{
    class SingleData;

    class ProducerData
    {
		NFmiProducer producer_;
        std::vector<std::unique_ptr<SingleData>> dataVector_;
    public:
        ProducerData(const NFmiProducer &producer);
        ~ProducerData();

        bool updateData(NFmiInfoOrganizer &infoOrganizer, NFmiHelpDataInfoSystem &helpDataInfoSystem);
        const NFmiProducer& producer() const { return producer_; }
        const std::vector<std::unique_ptr<SingleData>>& dataVector() const { return dataVector_; }
        bool empty() const { return dataVector_.empty(); }
        std::vector<SingleRowItem> makeDialogRowData(const std::vector<SingleRowItem> &dialogRowDataMemory) const;
        std::string makeUniqueProducerIdString() const;
    private:
        bool updateData(const boost::shared_ptr<NFmiFastQueryInfo> &info, NFmiInfoOrganizer &infoOrganizer, NFmiHelpDataInfoSystem &helpDataInfoSystem);
        void addNewSingleData(const boost::shared_ptr<NFmiFastQueryInfo> &info, NFmiHelpDataInfoSystem &helpDataInfoSystem);
    };
}
