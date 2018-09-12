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

    bool isThereNewFileAvailable(const NFmiHelpDataInfo *helpDataInfo, std::string latestDataFilePath)
    {
        if(helpDataInfo->LatestFileName().empty())
            return true;
        if(helpDataInfo->LatestFileName() != latestDataFilePath)
            return true;
        return false;
    }

    // When making rowItem from non-vertical data with only param info, rowItem (tree-node) is in
    // collapsed mode because otherwise dialog's update codes will open it always.
    // Here is used the SingleRowItem's parentItemId to store producerId
    
    AddParams::SingleRowItem makeRowItem(const NFmiDataIdent &dataIdent, NFmiInfoData::Type dataType, AddParams::RowType rowType, bool leafNode = false, const std::string& origTime = std::string())
    {
        auto rowItem = AddParams::SingleRowItem(rowType, std::string(dataIdent.GetParamName()), dataIdent.GetParamIdent(), true, "", dataType, dataIdent.GetProducer()->GetIdent(), std::string(dataIdent.GetProducer()->GetName()), leafNode);
        rowItem.origTime(origTime);
        return rowItem;
    }

    AddParams::SingleRowItem makeLevelRowItem(const NFmiDataIdent &dataIdent, NFmiInfoData::Type dataType, AddParams::RowType rowType, const std::shared_ptr<NFmiLevel>& level)
    {
        std::string name = dataIdent.GetParamName() + " " + std::to_string(int(level->LevelValue()));
        return AddParams::SingleRowItem(rowType, name, dataIdent.GetParamIdent(), true, "", dataType, dataIdent.GetProducer()->GetIdent(), std::string(dataIdent.GetProducer()->GetName()), true, level);
    }

    void addLevelRowItems(std::vector<AddParams::SingleRowItem> &dialogRowData, const NFmiDataIdent &dataIdent, NFmiInfoData::Type dataType, AddParams::RowType rowType, NFmiQueryInfo &queryInfo)
    {
        for(queryInfo.ResetLevel(); queryInfo.NextLevel(); )
        {
            const std::shared_ptr<NFmiLevel> level = std::make_shared<NFmiLevel>(*queryInfo.Level());
            dialogRowData.push_back(::makeLevelRowItem(dataIdent, dataType, rowType, level));
        }
    }

    void addPossibleSubParameters(std::vector<AddParams::SingleRowItem> &dialogRowData, const NFmiDataIdent &dataIdent, NFmiInfoData::Type dataType, AddParams::RowType rowType, NFmiQueryInfo &queryInfo)
    {
        if(dataIdent.HasDataParams())
        {
            auto subParams = dataIdent.GetDataParams();
            if(subParams)
            {
                for(const auto &subParam : subParams->ParamsVector())
                {
                    bool hasLevelData = queryInfo.SizeLevels() > 1;

                    if(hasLevelData) //Level wind data
                    {
                        dialogRowData.push_back(::makeRowItem(subParam, dataType, AddParams::RowType::kLevelType)); //Parameter name as "header"
                        ::addLevelRowItems(dialogRowData, subParam, dataType, AddParams::RowType::kSubParamLevelType, queryInfo); //Actual level data
                    }
                    else //Surface wind data
                    {
                        dialogRowData.push_back(::makeRowItem(subParam, dataType, rowType, true));
                    }
                }
            }
        }
    }

    //Create dailogRowData with proper RowType
    void addParameterAndPossibleSubParameters(std::vector<AddParams::SingleRowItem> &dialogRowData, const NFmiDataIdent &dataIdent, NFmiInfoData::Type dataType, NFmiQueryInfo &queryInfo)
    {
        AddParams::RowType rowType;
        //kNoType = 0,
        //kSubParamLevelType = 1,
        //kLevelType = 2,
        //kSubParamType = 3,
        //kParamType = 4,
        //kDataType = 5,
        //kProducerType = 6,
        //kCategoryType = 7,
        
        if(!dataIdent.HasDataParams()) 
        {

            bool hasLevelData = queryInfo.SizeLevels() > 1;
            rowType = AddParams::RowType::kParamType;

            if(hasLevelData) //Level data
            {
                dialogRowData.push_back(::makeRowItem(dataIdent, dataType, rowType, false)); //Parameter name as "header", not actual data
                ::addLevelRowItems(dialogRowData, dataIdent, dataType, AddParams::RowType::kLevelType, queryInfo); //Actual level data
            } 
            else //Surface data
            {
                dialogRowData.push_back(::makeRowItem(dataIdent, dataType, rowType, true));
            }
        }
        else //Wind sub menu
        {
            rowType = AddParams::RowType::kParamType;
            dialogRowData.push_back(::makeRowItem(dataIdent, dataType, rowType, false));
            rowType = AddParams::RowType::kSubParamType; 
            ::addPossibleSubParameters(dialogRowData, dataIdent, dataType, rowType, queryInfo);
        }
    }

}

namespace AddParams
{
    SingleData::SingleData() = default;

    SingleData::~SingleData() = default;

    // Returns true, if data's param or level structure is changed
    bool SingleData::updateData(const boost::shared_ptr<NFmiFastQueryInfo>& info, const NFmiHelpDataInfo *helpDataInfo)
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
            ::addParameterAndPossibleSubParameters(dialogRowData, dataIdent, dataType_, *latestMetaData_);
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

     std::string SingleData::OrigOrLastTime() const
    {
        NFmiFastQueryInfo fastInfo(*latestMetaData_);
        NFmiMetTime dataTime = fastInfo.OriginTime();
        if(dataType_ == NFmiInfoData::kObservations || dataType_ == NFmiInfoData::kAnalyzeData)
        {
            dataTime = fastInfo.TimeDescriptor().LastTime(); // If observation or analyze data, use data's last time.
        }

        std::string origTime = dataTime.ToStr("YYYY.MM.DD HH:mm");
        return !origTime.empty() ? origTime : "";
    }
}
