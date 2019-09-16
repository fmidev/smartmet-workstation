#include "ParameterSelectionSystem.h"
#include "CategoryData.h"
#include "ParameterSelectionUtils.h"
#include "NFmiDictionaryFunction.h"
#include "NFmiMacroParamSystem.h"
#include "NFmiFastQueryInfo.h"
#include "NFmiMetEditorTypes.h"
#include "NFmiInfoOrganizer.h"
#include "NFmiProducerSystem.h"
#include "..\..\..\catlog\catlog\catlogutils.h"
#include "SpecialDesctopIndex.h"
    
class NFmiInfoOrganizer;

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
	,obsProducerSystem_(nullptr)
	,satelImageProducerSystem_(nullptr)
    ,infoOrganizer_(nullptr)
    ,helpDataInfoSystem_(nullptr)
    ,itsLastActivatedDesktopIndex(0)
    ,itsLastActivatedRowIndex(1)
    ,helpDataIDs_()
    ,customCategories_()
	,soundingLevels_(nullptr)
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
        updateOperationalData("Operational data", NFmiInfoData::kEditable);
        updateData("Model data", *modelProducerSystem_, NFmiInfoData::kViewable);
        updateData("Observation data", *obsProducerSystem_, NFmiInfoData::kObservations);
        updateData("Satellite images", *satelImageProducerSystem_, NFmiInfoData::kSatelData);
        updateMacroParamData("Macro Parameters", NFmiInfoData::kMacroParam);
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

    void ParameterSelectionSystem::updateOperationalData(std::string categoryName, NFmiInfoData::Type dataCategory)
    {
        auto iter = std::find_if(categoryDataVector_.begin(), categoryDataVector_.end(), [categoryName](const auto &categoryData) {return categoryName == categoryData->categoryName(); });
        if(iter != categoryDataVector_.end())
        {
            dialogDataNeedsUpdate_ |= (*iter)->updateOperationalData(*infoOrganizer_, *helpDataInfoSystem_, dataCategory);
        }
        else
        {
            auto categoryDataPtr = std::make_unique<CategoryData>(categoryName, dataCategory);
            categoryDataPtr->updateOperationalData(*infoOrganizer_, *helpDataInfoSystem_, dataCategory);
            categoryDataVector_.push_back(std::move(categoryDataPtr));
            dialogDataNeedsUpdate_ = true;
        }
    }

    void ParameterSelectionSystem::updateMacroParamData(std::string categoryName, NFmiInfoData::Type dataCategory)
    {
        if(getMacroParamSystemCallback_)
        {
            auto &macroParamSystem = getMacroParamSystemCallback_();
            auto &macroParamTree = macroParamSystem.MacroParamItemTree();

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
        if(dataCategory == NFmiInfoData::kObservations) 
            categoryDataPtr->setSoungindLevels(*soundingLevels_);
        categoryDataPtr->updateData(producerSystem, infoOrganizer, helpDataInfoSystem, dataCategory, helpDataIDs_, customCategory);
        if(categoryDataPtr->producerDataVector().size() > 0) { //Hide categories that don't have data
            categoryDataVector_.push_back(std::move(categoryDataPtr));
        }
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
            updateData();
            updateDialogRowData();
            updateDialogTreePatternData();
            dialogDataNeedsUpdate_ = false;
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
            auto gategoryRowData = category->makeDialogRowData(dialogRowDataMemory, *infoOrganizer_);
            dialogRowData_.insert(dialogRowData_.end(), gategoryRowData.begin(), gategoryRowData.end());
        }
        for(const auto &rowItem : otherHelpData_)
        {
            dialogRowData_.push_back(rowItem);
        }

		trimDialogRowDataDependingOnActiveView();
    }

	void ParameterSelectionSystem::trimDialogRowDataDependingOnActiveView()
	{
		std::vector<AddParams::SingleRowItem> trimmedRowData;
		int index = 0;

		if (itsLastActivatedDesktopIndex == CtrlViewUtils::kFmiCrossSectionView)
		{
			for (auto& row : dialogRowData_)
			{
				if (row.rowType() == AddParams::RowType::kCategoryType || row.rowType() == AddParams::RowType::kProducerType)
				{
					trimmedRowData.push_back(row);
				}
				if (row.dataType() == NFmiInfoData::kMacroParam)
				{
					trimmedRowData.push_back(row);
				}
				checkedVector<boost::shared_ptr<NFmiFastQueryInfo>> infoVector = infoOrganizer_->GetInfos(row.uniqueDataId());
				if (!infoVector.empty())
				{
					auto info = infoVector.at(0);
					if (info->SizeLevels() > 1 && info->IsGrid())
					{
						trimmedRowData.push_back(row);
						auto subRows = addSubmenu(row, index);
						trimmedRowData.insert(trimmedRowData.end(), subRows.begin(), subRows.end());
					}
				}
				index++;
			}
			dialogRowData_.swap(trimmedRowData);
		}
		else if (itsLastActivatedDesktopIndex == CtrlViewUtils::kFmiTimeSerialView)
		{
			for (auto& row : dialogRowData_)
			{
				if (row.rowType() == AddParams::RowType::kCategoryType || row.rowType() == AddParams::RowType::kProducerType)
				{
					trimmedRowData.push_back(row);
				}
				else if (row.itemId() == 2001 || row.parentItemId() == 2001)
				{
					trimmedRowData.push_back(row);
				}
				checkedVector<boost::shared_ptr<NFmiFastQueryInfo>> infoVector = infoOrganizer_->GetInfos(row.uniqueDataId());
				if (!infoVector.empty())
				{
					auto info = infoVector.at(0);
					if (info->IsGrid())
					{
						trimmedRowData.push_back(row);
						auto subRows = addAllChildNodes(row, index);
						trimmedRowData.insert(trimmedRowData.end(), subRows.begin(), subRows.end());
					}
				}
				index++;
			}
			dialogRowData_.swap(trimmedRowData);
		}
	}

	std::vector<SingleRowItem> ParameterSelectionSystem::addSubmenu(SingleRowItem& row, int index)
	{
		//Add all sub items
		std::vector<AddParams::SingleRowItem> rowData;
		if (index + 1 < dialogRowData_.size())
		{
			while (dialogRowData_.at(index + 1).treeDepth() > row.treeDepth())
			{
				if (!dialogRowData_.at(index + 1).leafNode())
				{
					dialogRowData_.at(index + 1).crossSectionLeafNode(true);
					rowData.push_back(dialogRowData_.at(index + 1));
				}
				index++;
				if (index + 1 >= dialogRowData_.size())
					break;
			}
		}
		return rowData;
	}

	std::vector<SingleRowItem> ParameterSelectionSystem::addAllChildNodes(SingleRowItem& row, int index)
	{
		//Add all child items
		std::vector<AddParams::SingleRowItem> rowData;
		if (index + 1 < dialogRowData_.size())
		{
			while (dialogRowData_.at(index + 1).treeDepth() > row.treeDepth())
			{
				rowData.push_back(dialogRowData_.at(index + 1));
				index++;
				if (index + 1 >= dialogRowData_.size())
					break;
			}
		}
		return rowData;
	}

    // Must be called after updateDialogRowData call.
    void ParameterSelectionSystem::updateDialogTreePatternData()
    {
        dialogTreePatternArray_.clear();
        for(const auto &rowItem : dialogRowData_)
            dialogTreePatternArray_.push_back(rowItem.treeDepth());
    }

    void ParameterSelectionSystem::searchItemsThatMatchToSearchWords(std::string words)
    {
        // Needs to fill dialogRowData before new search
        updateDialogRowData();

        if(words.empty())
        {
            updateDialogTreePatternData();
            dialogDataNeedsUpdate_ = true;
            return;
        }
             
        std::vector<SingleRowItem> resultRowData;
        auto searchedWords = CatLogUtils::getSearchedWords(words);
        for(auto row : dialogRowData_)
        {
            if(!row.leafNode()) { resultRowData.push_back(row); }
            else if(CatLogUtils::containsAllSearchedWordsCaseInsensitive(row.searchWords(), searchedWords))
            {
                resultRowData.push_back(row);
            }
        }

        removeNodesThatDontHaveLeafs(resultRowData);
        if(!resultRowData.empty())
        dialogRowData_.swap(resultRowData);
        updateDialogTreePatternData();
        dialogDataNeedsUpdate_ = true;
    }

	void ParameterSelectionSystem::removeNodesThatDontHaveLeafs(std::vector<SingleRowItem>& resultRowData)
    {
        std::vector<SingleRowItem> rowData;
        int index = 0;

        //Remove nodes with no childs
        for(auto row : resultRowData)
        {
            if(row.leafNode() || (itsLastActivatedDesktopIndex == CtrlViewUtils::kFmiCrossSectionView && row.crossSectionLeafNode())) 
			{ 
				rowData.push_back(row); 
			}
            if((index + 1 < resultRowData.size()))
            {
                if(resultRowData.at(index + 1).treeDepth() != row.treeDepth())
                {
                    rowData.push_back(row);
                }
            }
            index++;
        }
        resultRowData.swap(rowData);
        rowData.clear();

        //Then remove nodes with no leaf node as a child
        index = 0;
        for(auto row : resultRowData)
        {
            if(row.treeDepth() == 1) { rowData.push_back(row); }
            else if(hasLeafNodeAsAChild(index, resultRowData))
            {
                rowData.push_back(row);
            }
            index++;
        }
        resultRowData.swap(rowData);
    }

    bool ParameterSelectionSystem::hasLeafNodeAsAChild(int index, std::vector<SingleRowItem> &resultRowData)
    {
        auto row = resultRowData.at(index);
        
        if(row.leafNode() || (itsLastActivatedDesktopIndex == CtrlViewUtils::kFmiCrossSectionView && row.crossSectionLeafNode()))
        {
            return true;
        }
        else if(index + 1 <  resultRowData.size())
        {
            if(resultRowData.at(++index).treeDepth() > row.treeDepth())
            {
                return hasLeafNodeAsAChild(index, resultRowData);
            }
        }
        return false;
    }

}
