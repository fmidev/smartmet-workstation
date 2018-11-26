#include "CategoryData.h"
#include "ProducerData.h"
#include "ParamAddingUtils.h"
#include "NFmiProducerSystem.h"
#include "NFmiMacroParamSystem.h"

namespace
{
    AddParams::SingleRowItem makeRowItem(const AddParams::ProducerData &producerData, const std::string &uniqueId, const AddParams::SingleRowItem *rowItemMemory)
    {
        // If there is memory for this producer's rowItem, use it, otherwise put producerData in collapsed mode
        bool nodeCollapsed = rowItemMemory ? rowItemMemory->dialogTreeNodeCollapsed() : true;
        const auto &producer = producerData.producer();
        return AddParams::SingleRowItem(AddParams::kProducerType, std::string(producer.GetName()), producer.GetIdent(), nodeCollapsed, uniqueId, NFmiInfoData::kNoDataType);
    }
}

namespace AddParams
{
    CategoryData::CategoryData(const std::string &categoryName, NFmiInfoData::Type dataCategory)
    :categoryName_(categoryName)
    ,dataCategory_(dataCategory)
    ,producerDataVector_()
    {
    }

    CategoryData::~CategoryData() = default;

    // Returns true, if new producer is added or if some new producer data is added or data's param or level structure is changed
    bool CategoryData::updateData(NFmiProducerSystem &categoryProducerSystem, NFmiInfoOrganizer &infoOrganizer, NFmiHelpDataInfoSystem &helpDataInfoSystem, NFmiInfoData::Type dataCategory, std::vector<int> helpDataIDs)
    {
        bool dataStruckturesChanged = false;
        for(const auto &producerInfo : categoryProducerSystem.Producers())
        {
            NFmiProducer producer(producerInfo.ProducerId(), producerInfo.Name());
            bool helpData = std::find(helpDataIDs.begin(), helpDataIDs.end(), producerInfo.ProducerId()) != helpDataIDs.end();

            if(helpData && dataCategory == NFmiInfoData::kModelHelpData) //Add help data only when handling Help Data category
            {
                dataStruckturesChanged = addNewOrUpdateData(producer, infoOrganizer, helpDataInfoSystem, dataCategory);
            }          
            else if(!helpData && dataCategory != NFmiInfoData::kModelHelpData)
            {
                dataStruckturesChanged = addNewOrUpdateData(producer, infoOrganizer, helpDataInfoSystem, dataCategory);
            }
        }
        return dataStruckturesChanged;
    }

    // Returns true, if new macro params are added
    bool CategoryData::updateMacroParamData(std::vector<NFmiMacroParamItem> &macroParamTree, NFmiInfoData::Type dataCategory)
    {
        bool dataStruckturesChanged = false;
        NFmiProducer producer(998, "Macro Param");
        auto iter = std::find_if(producerDataVector_.begin(), producerDataVector_.end(), [producer](const auto &producerData) {return producer == producerData->producer(); });
        if(iter != producerDataVector_.end())
        {
            dataStruckturesChanged |= (*iter)->updateMacroParamData(macroParamTree);
        }
        else
        {
            // Add macro params as new producer
            auto producerDataPtr = std::make_unique<ProducerData>(producer, dataCategory);
            producerDataPtr->updateMacroParamData(macroParamTree);
            producerDataVector_.push_back(std::move(producerDataPtr));
            dataStruckturesChanged = true;
        }
        return dataStruckturesChanged;
    }

    bool CategoryData::addNewOrUpdateData(NFmiProducer producer, NFmiInfoOrganizer &infoOrganizer, NFmiHelpDataInfoSystem &helpDataInfoSystem, NFmiInfoData::Type dataCategory)
    {
        bool dataStruckturesChanged = false;

        auto iter = std::find_if(producerDataVector_.begin(), producerDataVector_.end(), [producer](const auto &producerData) {return producer == producerData->producer(); });
        if(iter != producerDataVector_.end())
        {
            dataStruckturesChanged |= (*iter)->updateData(infoOrganizer, helpDataInfoSystem);
        }
        else
        {
            addNewProducerData(producer, infoOrganizer, helpDataInfoSystem, dataCategory);
            dataStruckturesChanged = true;
        }
        return dataStruckturesChanged;
    }

    void CategoryData::addNewProducerData(const NFmiProducer &producer, NFmiInfoOrganizer &infoOrganizer, NFmiHelpDataInfoSystem &helpDataInfoSystem, NFmiInfoData::Type dataCategory)
    {
        auto producerDataPtr = std::make_unique<ProducerData>(producer, dataCategory);
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
