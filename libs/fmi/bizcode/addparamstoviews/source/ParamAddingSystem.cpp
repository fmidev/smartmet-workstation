#include "ParamAddingSystem.h"
#include "CategoryData.h"
#include "ParamAddingUtils.h"
#include "NFmiDictionaryFunction.h"

namespace
{
    unsigned char getDialogTreeDepth(const AddParams::SingleRowItem &rowItem)
    {
        switch(rowItem.rowType())
        {
        case AddParams::kCategoryType:
            return 1;
        case AddParams::kProducerType:
            return 2;
        case AddParams::kDataType:
            return 3;
        case AddParams::kParamType:
            return 4;
        case AddParams::kSubParamType:
        case AddParams::kLevelType:
            return 5;
        case AddParams::kSubParamLevelType:
            return 6;
        default:
            throw std::runtime_error("Error in getDialogTreeDepth function: Illegal row-item type");
        }
    }

    AddParams::SingleRowItem makeRowItem(const AddParams::CategoryData &categoryData, const std::string &uniqueId, const AddParams::SingleRowItem *rowItemMemory)
    {
        // If there is memory for this category's rowItem, use it, otherwise put categoryData in non-collapsed mode
        bool nodeCollapsed = rowItemMemory ? rowItemMemory->dialogTreeNodeCollapsed() : false;
        return AddParams::SingleRowItem(AddParams::kCategoryType, categoryData.categoryName(), 0, nodeCollapsed, uniqueId, NFmiInfoData::kNoDataType);
    }
}

namespace AddParams
{
    ParamAddingSystem::ParamAddingSystem()
    :updateWaitTimeoutInSeconds_(15)
    ,updatePending_(false)
    ,dialogRowData_()
    ,dialogTreePatternArray_()
    ,dialogDataNeedsUpdate_(true)
    ,modelProducerSystem_(nullptr)
    ,infoOrganizer_(nullptr)
    ,helpDataInfoSystem_(nullptr)
    ,itsLastAcivatedDescTopIndex(0)
    ,itsLastActivatedRowIndex(0)
    {
    }

    ParamAddingSystem::~ParamAddingSystem() = default;

    void ParamAddingSystem::initialize(NFmiProducerSystem &modelProducerSystem, NFmiInfoOrganizer &infoOrganizer, NFmiHelpDataInfoSystem &helpDataInfoSystem)
    {
        modelProducerSystem_ = &modelProducerSystem;
        infoOrganizer_ = &infoOrganizer;
        helpDataInfoSystem_ = &helpDataInfoSystem;
    }

    void ParamAddingSystem::updateModelData()
    {
        std::string categoryName = ::GetDictionaryString("Model data");
        auto iter = std::find_if(categoryDataVector_.begin(), categoryDataVector_.end(), [categoryName](const auto &categoryData) {return categoryName == categoryData->categoryName(); });
        if(iter != categoryDataVector_.end())
        {
            dialogDataNeedsUpdate_ |= (*iter)->updateData(*modelProducerSystem_, *infoOrganizer_, *helpDataInfoSystem_);
        }
        else
        {
            addNewCategoryData(categoryName, *modelProducerSystem_, *infoOrganizer_, *helpDataInfoSystem_);
        }

        updatePending(false);
    }

    void ParamAddingSystem::addNewCategoryData(const std::string &categoryName, NFmiProducerSystem &modelProducerSystem, NFmiInfoOrganizer &infoOrganizer, NFmiHelpDataInfoSystem &helpDataInfoSystem)
    {
        auto categoryDataPtr = std::make_unique<CategoryData>(categoryName);
        categoryDataPtr->updateData(modelProducerSystem, infoOrganizer, helpDataInfoSystem);
        categoryDataVector_.push_back(std::move(categoryDataPtr));
        dialogDataNeedsUpdate_ = true;
    }

    std::vector<SingleRowItem>& ParamAddingSystem::dialogRowData()
    {
        return dialogRowData_;
    }

    const std::vector<SingleRowItem>& ParamAddingSystem::dialogRowData() const
    { 
        return dialogRowData_; 
    }
    const std::vector<unsigned char>& ParamAddingSystem::dialogTreePatternArray() const
    { 
        return dialogTreePatternArray_;
    }

    void ParamAddingSystem::updateDialogData()
    {
        if(dialogDataNeedsUpdate_)
        {
            dialogDataNeedsUpdate_ = false;
            updateModelData();
            updateDialogRowData();
            updateDialogTreePatternData();
        }
    }

    void ParamAddingSystem::updateDialogRowData()
    {
        std::vector<SingleRowItem> dialogRowDataMemory;
        dialogRowDataMemory.swap(dialogRowData_);
        for(const auto &category : categoryDataVector_)
        {
            const std::string &uniqueId = category->categoryName();
            auto *categoryMemory = findDataRowItem(uniqueId, dialogRowDataMemory);
            dialogRowData_.push_back(::makeRowItem(*category, uniqueId, categoryMemory));
            auto gategoryRowData = category->makeDialogRowData(dialogRowDataMemory);
            dialogRowData_.insert(dialogRowData_.end(), gategoryRowData.begin(), gategoryRowData.end());
        }
    }

    // Must be called after updateDialogRowData call.
    void ParamAddingSystem::updateDialogTreePatternData()
    {
        dialogTreePatternArray_.clear();
        for(const auto &rowItem : dialogRowData_)
            dialogTreePatternArray_.push_back(getDialogTreeDepth(rowItem));
    }

}
