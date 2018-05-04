#include "CategoryData.h"
#include "ProducerData.h"
#include "ParamAddingUtils.h"
#include "NFmiProducerSystem.h"

namespace
{
    AddParams::SingleRowItem makeRowItem(const AddParams::ProducerData &producerData, const std::string &uniqueId, const AddParams::SingleRowItem *rowItemMemory)
    {
        // If there is memory for this producer's rowItem, use it, otherwise put producerData in non-collapsed mode
        bool nodeCollapsed = rowItemMemory ? rowItemMemory->dialogTreeNodeCollapsed() : false;
        const auto &producer = producerData.producer();
        return AddParams::SingleRowItem(AddParams::kProducerType, std::string(producer.GetName()), producer.GetIdent(), nodeCollapsed, uniqueId, NFmiInfoData::kNoDataType);
    }
}

namespace AddParams
{
        CategoryData::CategoryData(const std::string &categoryName)
        :categoryName_(categoryName)
        ,producerDataVector_()
        {
        }

        CategoryData::~CategoryData() = default;

        // Returns true, if new producer is added or if some new producer data is added or data's param or level structure is changed
        bool CategoryData::updateData(NFmiProducerSystem &categoryProducerSystem, NFmiInfoOrganizer &infoOrganizer, NFmiHelpDataInfoSystem &helpDataInfoSystem)
        {
            bool dataStruckturesChanged = false;
            for(const auto &producerInfo : categoryProducerSystem.Producers())
            {
                NFmiProducer producer(producerInfo.ProducerId(), producerInfo.Name());
                auto iter = std::find_if(producerDataVector_.begin(), producerDataVector_.end(), [producer](const auto &producerData) {return producer == producerData->producer(); });
                if(iter != producerDataVector_.end())
                {
                    dataStruckturesChanged |= (*iter)->updateData(infoOrganizer, helpDataInfoSystem);
                }
                else
                {
                    addNewProducerData(producer, infoOrganizer, helpDataInfoSystem);
                    dataStruckturesChanged = true;
                }
            }
            return dataStruckturesChanged;
        }

        void CategoryData::addNewProducerData(const NFmiProducer &producer, NFmiInfoOrganizer &infoOrganizer, NFmiHelpDataInfoSystem &helpDataInfoSystem)
        {
            auto producerDataPtr = std::make_unique<ProducerData>(producer);
            producerDataPtr->updateData(infoOrganizer, helpDataInfoSystem);
            producerDataVector_.push_back(std::move(producerDataPtr));
        }

        std::vector<SingleRowItem> CategoryData::makeDialogRowData(const std::vector<SingleRowItem> &dialogRowDataMemory) const
        {
            std::vector<SingleRowItem> dialogRowData;
            for(const auto &producerData : producerDataVector_)
            {
                auto rowData = producerData->makeDialogRowData(dialogRowDataMemory);
                if(rowData.size())
                {
                    // Only add producer and its rowData, if there is any real data in use
                    const std::string uniqueId = producerData->makeUniqueProducerIdString();
                    auto *producerMemory = findDataRowItem(uniqueId, dialogRowDataMemory);
                    dialogRowData.push_back(::makeRowItem(*producerData, uniqueId, producerMemory));
                    dialogRowData.insert(dialogRowData.end(), rowData.begin(), rowData.end());
                }
            }
            return dialogRowData;
        }

}
