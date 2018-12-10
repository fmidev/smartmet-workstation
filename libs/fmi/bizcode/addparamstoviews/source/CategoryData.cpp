#include "CategoryData.h"
#include "ProducerData.h"
#include "ParameterSelectionUtils.h"
#include "NFmiProducerSystem.h"
#include "NFmiMacroParamSystem.h"
#include "NFmiInfoOrganizer.h"
#include "NFmiHelpDataInfo.h"
#include "NFmiOwnerInfo.h"

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
    bool CategoryData::updateData(NFmiProducerSystem &categoryProducerSystem, NFmiInfoOrganizer &infoOrganizer, NFmiHelpDataInfoSystem &helpDataInfoSystem, 
        NFmiInfoData::Type dataCategory, std::vector<int> helpDataIDs, bool customCategory)
    {
        bool dataStructuresChanged = false;

        //Handle custom categories first
        if(customCategory)
        {
            dataStructuresChanged = updateCustomCategoryData(categoryProducerSystem, infoOrganizer, helpDataInfoSystem, dataCategory);
        }
        else
        {
            for(const auto &producerInfo : categoryProducerSystem.Producers())
            {
                NFmiProducer producer(producerInfo.ProducerId(), producerInfo.Name());
                bool helpData = std::find(helpDataIDs.begin(), helpDataIDs.end(), producerInfo.ProducerId()) != helpDataIDs.end();
                auto fastQueryInfoVector = infoOrganizer.GetInfos(producerInfo.ProducerId());
                auto dataType = !fastQueryInfoVector.empty() ? fastQueryInfoVector.at(0)->DataType() : NFmiInfoData::kNoDataType;

                if(helpData && dataCategory == NFmiInfoData::kModelHelpData) //Add help data only when handling Help Data category
                {
                    dataStructuresChanged = addNewOrUpdateData(producer, infoOrganizer, helpDataInfoSystem, dataCategory);
                }
                else if(categoryName() == "Operational data" && dataType == NFmiInfoData::kKepaData)
                {
                    dataStructuresChanged = addNewOrUpdateData(producer, infoOrganizer, helpDataInfoSystem, dataCategory);
                }
                else if(!helpData && dataCategory != NFmiInfoData::kModelHelpData && !(categoryName() == "Operational data"))
                {
                    dataStructuresChanged = addNewOrUpdateData(producer, infoOrganizer, helpDataInfoSystem, dataCategory);
                }
            }
        }

        return dataStructuresChanged;
    }

    // Returns true, if new custom categories are added
    bool CategoryData::updateCustomCategoryData(NFmiProducerSystem &categoryProducerSystem, NFmiInfoOrganizer &infoOrganizer, NFmiHelpDataInfoSystem &helpDataInfoSystem, NFmiInfoData::Type dataCategory)
    {
        bool dataStructuresChanged = false;
        for(const auto &info : helpDataInfoSystem.DynamicHelpDataInfos())
        {
            if(info.CustomMenuFolder() == categoryName())
            {
                checkedVector<boost::shared_ptr<NFmiFastQueryInfo> > infoVector = infoOrganizer.GetInfos(info.UsedFileNameFilter(helpDataInfoSystem));
                if(infoVector.size())
                {
                    auto queryInfo = infoVector[0];
                    auto producer = queryInfo->Producer();
                    dataStructuresChanged = addNewOrUpdateData(*producer, infoOrganizer, helpDataInfoSystem, dataCategory, true);
                }
            }
        }
        return dataStructuresChanged;
    }

    // Returns true, if new macro params are added
    bool CategoryData::updateMacroParamData(std::vector<NFmiMacroParamItem> &macroParamTree, NFmiInfoData::Type dataCategory)
    {
        bool dataStructuresChanged = false;
        NFmiProducer producer(998, "Macro Param");
        auto iter = std::find_if(producerDataVector_.begin(), producerDataVector_.end(), [producer](const auto &producerData) {return producer == producerData->producer(); });
        if(iter != producerDataVector_.end())
        {
            dataStructuresChanged |= (*iter)->updateMacroParamData(macroParamTree);
        }
        else
        {
            // Add macro params as new producer
            auto producerDataPtr = std::make_unique<ProducerData>(producer, dataCategory);
            producerDataPtr->updateMacroParamData(macroParamTree);
            producerDataVector_.push_back(std::move(producerDataPtr));
            dataStructuresChanged = true;
        }
        return dataStructuresChanged;
    }

    bool CategoryData::addNewOrUpdateData(NFmiProducer producer, NFmiInfoOrganizer &infoOrganizer, NFmiHelpDataInfoSystem &helpDataInfoSystem, NFmiInfoData::Type dataCategory, bool customCategory)
    {
        bool dataStruckturesChanged = false;
        // Add only when handling custom category
        if(!customCategory && skipCustomProducerData(producer, infoOrganizer, helpDataInfoSystem))
            return dataStruckturesChanged;

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

    bool CategoryData::skipCustomProducerData(const NFmiProducer &producer, NFmiInfoOrganizer &infoOrganizer, NFmiHelpDataInfoSystem &helpDataInfoSystem)
    {
        auto producerData = infoOrganizer.GetInfos(producer.GetIdent());
        for(auto &info : producerData)
        {
            auto filePattern = info->DataFilePattern();
            NFmiHelpDataInfo *helpDataInfo = helpDataInfoSystem.FindHelpDataInfo(filePattern);
            if(helpDataInfo && !helpDataInfo->CustomMenuFolder().empty())
            {
                return true;
            }
        }
        return false;
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
