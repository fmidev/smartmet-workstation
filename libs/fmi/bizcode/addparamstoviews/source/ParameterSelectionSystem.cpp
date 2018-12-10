#include "ParameterSelectionSystem.h"
#include "CategoryData.h"
#include "ParameterSelectionUtils.h"
#include "NFmiDictionaryFunction.h"
#include "NFmiMacroParamSystem.h"
    

namespace
{
    AddParams::SingleRowItem makeRowItem(const AddParams::CategoryData &categoryData, const std::string &uniqueId, const AddParams::SingleRowItem *rowItemMemory)
    {
        // If there is memory for this category's rowItem, use it, otherwise put categoryData in non-collapsed mode
        bool nodeCollapsed = rowItemMemory ? rowItemMemory->dialogTreeNodeCollapsed() : false;
        return AddParams::SingleRowItem(AddParams::kCategoryType, categoryData.categoryName(), 0, nodeCollapsed, uniqueId, NFmiInfoData::kNoDataType);
    }
}

namespace AddParams
{
    ParameterSelectionSystem::ParameterSelectionSystem()
    :updateWaitTimeoutInSeconds_(10)
    ,updatePending_(false)
    ,dialogRowData_()
    ,dialogTreePatternArray_()
    ,dialogDataNeedsUpdate_(true)
    ,modelProducerSystem_(nullptr)
    ,infoOrganizer_(nullptr)
    ,helpDataInfoSystem_(nullptr)
    ,itsLastAcivatedDescTopIndex(0)
    ,itsLastActivatedRowIndex(1)
    ,helpDataIDs_()
    ,customCategories_()
    {
    }

    ParameterSelectionSystem::~ParameterSelectionSystem() = default;

    void ParameterSelectionSystem::initialize(NFmiProducerSystem &modelProducerSystem, NFmiProducerSystem &obsProducerSystem, NFmiProducerSystem &satelImageProducerSystem, 
        NFmiInfoOrganizer &infoOrganizer, NFmiHelpDataInfoSystem &helpDataInfoSystem, std::vector<int> idVector, std::vector<std::string> customCategories)
    {
        modelProducerSystem_ = &modelProducerSystem;
        obsProducerSystem_ = &obsProducerSystem;
        satelImageProducerSystem_ = &satelImageProducerSystem;
        infoOrganizer_ = &infoOrganizer;
        helpDataInfoSystem_ = &helpDataInfoSystem;
        helpDataIDs_ = idVector; // Help Data id's. These are added to Help Data Category
        customCategories_ = customCategories;
    }

    void ParameterSelectionSystem::addHelpData(NFmiProducer &producer, const std::string &menuString, NFmiInfoData::Type dataType) //Add at the end of help data list
    {
        addHelpData(producer, menuString, dataType, std::string());
    }

    void ParameterSelectionSystem::addHelpData(NFmiProducer &producer, const std::string &menuString, NFmiInfoData::Type dataType, std::string &displayName) //Add at the end of help data list
    {
        std::string uniqueDataId = std::string(producer.GetName()) + " - " + menuString;
        SingleRowItem item = SingleRowItem(kParamType, menuString, producer.GetIdent(), true, uniqueDataId, dataType, 0, "", true, nullptr, 2, displayName);
        otherHelpData_.push_back(item);
    }

    void ParameterSelectionSystem::updateData()
    {
        updateData("Model data", *modelProducerSystem_, NFmiInfoData::kViewable);
        updateData("Observation data", *obsProducerSystem_, NFmiInfoData::kObservations);
        updateData("Satellite images", *satelImageProducerSystem_, NFmiInfoData::kSatelData);
        updateMacroParamData("Macro Params", NFmiInfoData::kMacroParam);
        updateCustomCategories();
        updateData("Help data", *modelProducerSystem_, NFmiInfoData::kModelHelpData);
        updateData("Help data", *obsProducerSystem_, NFmiInfoData::kModelHelpData);
    }

    void ParameterSelectionSystem::updateData(std::string catName, NFmiProducerSystem &producerSystem, NFmiInfoData::Type dataCategory, bool customCategory)
    {
        std::string categoryName = ::GetDictionaryString(catName.c_str());
        auto iter = std::find_if(categoryDataVector_.begin(), categoryDataVector_.end(), [categoryName](const auto &categoryData) {return categoryName == categoryData->categoryName(); });
        if(iter != categoryDataVector_.end())
        {
            dialogDataNeedsUpdate_ |= (*iter)->updateData(producerSystem, *infoOrganizer_, *helpDataInfoSystem_, dataCategory, helpDataIDs_, customCategory);
        }
        else
        {
            addNewCategoryData(categoryName, producerSystem, *infoOrganizer_, *helpDataInfoSystem_, dataCategory, customCategory);
        }

        updatePending(false);
    }

    void ParameterSelectionSystem::updateMacroParamData(std::string catName, NFmiInfoData::Type dataCategory)
    {
        if(getMacroParamSystemCallback_)
        {
            auto &macroParamSystem = getMacroParamSystemCallback_();
            auto &macroParamTree = macroParamSystem.MacroParamItemTree();

            std::string categoryName = ::GetDictionaryString(catName.c_str());
            auto iter = std::find_if(categoryDataVector_.begin(), categoryDataVector_.end(), [categoryName](const auto &categoryData) {return categoryName == categoryData->categoryName(); });
            if(iter != categoryDataVector_.end())
            {
                    dialogDataNeedsUpdate_ |= (*iter)->updateMacroParamData(macroParamTree, dataCategory);
            }
            else
            {
                // Add macro params as a new category
                auto categoryDataPtr = std::make_unique<CategoryData>(categoryName, dataCategory);
                categoryDataPtr->updateMacroParamData(macroParamTree, dataCategory);
                categoryDataVector_.push_back(std::move(categoryDataPtr));
                dialogDataNeedsUpdate_ = true;
            }

            updatePending(false);
        }
    }

    void ParameterSelectionSystem::updateCustomCategories()
    {
        for(auto customCat : customCategories_)
        {
            updateData(customCat, *modelProducerSystem_, NFmiInfoData::kViewable, true);
            updateData(customCat, *obsProducerSystem_, NFmiInfoData::kObservations, true);
        }
    }

    void ParameterSelectionSystem::addNewCategoryData(const std::string &categoryName, NFmiProducerSystem &producerSystem, NFmiInfoOrganizer &infoOrganizer, NFmiHelpDataInfoSystem &helpDataInfoSystem, NFmiInfoData::Type dataCategory, bool customCategory)
    {
        auto categoryDataPtr = std::make_unique<CategoryData>(categoryName, dataCategory);
        categoryDataPtr->updateData(producerSystem, infoOrganizer, helpDataInfoSystem, dataCategory, helpDataIDs_, customCategory);
        categoryDataVector_.push_back(std::move(categoryDataPtr));
        dialogDataNeedsUpdate_ = true;
    }

    std::vector<SingleRowItem>& ParameterSelectionSystem::dialogRowData()
    {
        return dialogRowData_;
    }

    const std::vector<SingleRowItem>& ParameterSelectionSystem::dialogRowData() const
    { 
        return dialogRowData_; 
    }
    const std::vector<unsigned char>& ParameterSelectionSystem::dialogTreePatternArray() const
    { 
        return dialogTreePatternArray_;
    }

    void ParameterSelectionSystem::updateDialogData()
    {
        if(dialogDataNeedsUpdate_)
        {
            dialogDataNeedsUpdate_ = false;
            updateData();
            updateDialogRowData();
            updateDialogTreePatternData();
        }
    }

    void ParameterSelectionSystem::updateDialogRowData()
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
        for(const auto &rowItem : otherHelpData_)
        {
            dialogRowData_.push_back(rowItem);
        }
    }

    // Must be called after updateDialogRowData call.
    void ParameterSelectionSystem::updateDialogTreePatternData()
    {
        dialogTreePatternArray_.clear();
        for(const auto &rowItem : dialogRowData_)
            dialogTreePatternArray_.push_back(rowItem.treeDepth());
    }
}
