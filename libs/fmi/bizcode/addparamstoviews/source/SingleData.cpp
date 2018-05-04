#include "SingleData.h"
#include "NFmiFastQueryInfo.h"
#include "NFmiHelpDataInfo.h"

namespace
{
    bool isDataStructuresChanged(const boost::shared_ptr<NFmiFastQueryInfo>& newInfo, const std::unique_ptr<NFmiQueryInfo> &latestMetaData)
    {
        if(latestMetaData == nullptr)
            return true;
        if(!(newInfo->ParamBag() == latestMetaData->ParamBag()))
            return true;
        if(!(newInfo->VPlaceDescriptor() == latestMetaData->VPlaceDescriptor()))
            return true;
        return false;
    }

    // When making rowItem from non-vertical data with only param info, rowItem (tree-node) is in
    // collapsed mode because otherwise dialog's update codes will open it allways.
    // Here is used the SingleRowItem's parentItemId to store producerId
    AddParams::SingleRowItem makeRowItem(const NFmiDataIdent &dataIdent, NFmiInfoData::Type dataType, AddParams::RowType rowType)
    {
        return AddParams::SingleRowItem(rowType, std::string(dataIdent.GetParamName()), dataIdent.GetParamIdent(), true, "", dataType, dataIdent.GetProducer()->GetIdent(), std::string(dataIdent.GetProducer()->GetName()));
    }

    void addPossibleSubParameters(std::vector<AddParams::SingleRowItem> &dialogRowData, const NFmiDataIdent &dataIdent, NFmiInfoData::Type dataType, AddParams::RowType rowType)
    {
        if(dataIdent.HasDataParams())
        {
            auto subParams = dataIdent.GetDataParams();
            if(subParams)
            {
                for(const auto &subParam : subParams->ParamsVector())
                {
                    dialogRowData.push_back(::makeRowItem(subParam, dataType, rowType));
                }
            }
        }
    }
}

namespace AddParams
{
    SingleData::SingleData() = default;

    SingleData::~SingleData() = default;

    // Returns true, if data's param or level structure is changed
    bool SingleData::updateData(const boost::shared_ptr<NFmiFastQueryInfo>& info, const NFmiHelpDataInfo * helpDataInfo)
    {
        bool dataStruckturesChanged = false;
        // No need to do update, if the latest data from infoOrganizer is still the same (with same filename timestamps)
        if(latestDataFilePath_ != info->DataFileName())
        {
            dataStruckturesChanged = ::isDataStructuresChanged(info, latestMetaData_);
            latestDataFilePath_ = info->DataFileName();
            latestMetaData_ = std::make_unique<NFmiQueryInfo>(*info);
            uniqueDataId_ = info->DataFilePattern();
            dataType_ = info->DataType();
            if(helpDataInfo)
            {
                dataName_ = helpDataInfo->GetCleanedName();
            }
        }
        return dataStruckturesChanged;
    }

    std::vector<SingleRowItem> SingleData::makeDialogRowData() const
    {
        std::vector<SingleRowItem> dialogRowData;
        const auto &paramVector = latestMetaData_->ParamBag().ParamsVector();
        for(const auto &dataIdent : paramVector)
        {
            dialogRowData.push_back(::makeRowItem(dataIdent, dataType_, kParamType));
            ::addPossibleSubParameters(dialogRowData, dataIdent, dataType_, kSubParamType);
        }
        return dialogRowData;
    }

    unsigned long SingleData::producerId() const
    {
        if(latestMetaData_)
            return latestMetaData_->Producer()->GetIdent();
        else
            return 0;
    }
}
