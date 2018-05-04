#include "ProducerData.h"
#include "SingleData.h"
#include "ParamAddingUtils.h"
#include "NFmiInfoOrganizer.h"
#include "NFmiFastQueryInfo.h"
#include "NFmiHelpDataInfo.h"

namespace
{
    bool isDataAccepted(const boost::shared_ptr<NFmiFastQueryInfo> &info)
    {
        if(info)
        {
            if(info->IsGrid())
            {
                if(info->SizeLevels() == 1)
                {
                    return true;
                }
            }
        }
        return false;
    }

    AddParams::SingleRowItem makeRowItem(const AddParams::SingleData &data, const std::string &uniqueId, const AddParams::SingleRowItem *rowItemMemory)
    {
        // If there is memory for this data's rowItem, use it, otherwise put singleData in collapsed mode
        bool nodeCollapsed = rowItemMemory ? rowItemMemory->dialogTreeNodeCollapsed() : true;
        return AddParams::SingleRowItem(AddParams::kDataType, data.dataName(), data.producerId(), nodeCollapsed, uniqueId, NFmiInfoData::kNoDataType);
    }
}

namespace AddParams
{
    ProducerData::ProducerData(const NFmiProducer &producer)
    :producer_(producer)
    ,dataVector_()
    {
    }

    ProducerData::~ProducerData() = default;

    // Returns true, if some new producer data is added or data's param or level structure is changed
    bool ProducerData::updateData(NFmiInfoOrganizer &infoOrganizer, NFmiHelpDataInfoSystem &helpDataInfoSystem)
    {
        bool dataStruckturesChanged = false;
        auto producerData = infoOrganizer.GetInfos(producer_.GetIdent());
        for(auto &info : producerData)
        {
            if(::isDataAccepted(info))
            {
                dataStruckturesChanged |= updateData(info, infoOrganizer, helpDataInfoSystem);
            }
        }
        return dataStruckturesChanged;
    }

    bool ProducerData::updateData(const boost::shared_ptr<NFmiFastQueryInfo> &info, NFmiInfoOrganizer &infoOrganizer, NFmiHelpDataInfoSystem &helpDataInfoSystem)
    {
        auto fileFilter = info->DataFilePattern();
        auto iter = std::find_if(dataVector_.begin(), dataVector_.end(),
            [fileFilter]
        (const auto &singleData)
        {
            return fileFilter == singleData->uniqueDataId();
        });

        if(iter != dataVector_.end())
        {
            return (*iter)->updateData(info);
        }
        else
        {
            addNewSingleData(info, helpDataInfoSystem);
            return true;
        }
    }

    void ProducerData::addNewSingleData(const boost::shared_ptr<NFmiFastQueryInfo> &info, NFmiHelpDataInfoSystem &helpDataInfoSystem)
    {
        auto helpDataInfo = helpDataInfoSystem.FindHelpDataInfo(info->DataFilePattern());
        if(helpDataInfo)
        {
            auto singleDataPtr = std::make_unique<SingleData>();
            singleDataPtr->updateData(info, helpDataInfo);
            dataVector_.push_back(std::move(singleDataPtr));
        }
    }

    std::vector<SingleRowItem> ProducerData::makeDialogRowData(const std::vector<SingleRowItem> &dialogRowDataMemory) const
    {
        std::vector<SingleRowItem> dialogRowData;
        for(const auto &singleData : dataVector_)
        {
            const std::string &uniqueId = singleData->uniqueDataId();
            auto *singleDataMemory = findDataRowItem(uniqueId, dialogRowDataMemory);
            dialogRowData.push_back(::makeRowItem(*singleData, uniqueId, singleDataMemory));
            auto rowData = singleData->makeDialogRowData();
            dialogRowData.insert(dialogRowData.end(), rowData.begin(), rowData.end());
        }
        return dialogRowData;
    }

    // This unique id string is used to mark producer's SingleRowItem object.
    // Unique id: name + "_" + prodId (e.g. "Ecmwf_240")
    std::string ProducerData::makeUniqueProducerIdString() const
    {
        std::string uniqueId = producer_.GetName();
        uniqueId += "_";
        uniqueId += std::to_string(producer_.GetIdent());
        return uniqueId;
    }
}
