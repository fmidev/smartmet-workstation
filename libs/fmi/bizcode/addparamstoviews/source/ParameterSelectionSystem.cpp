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
#include "NFmiHelpDataInfo.h"
#ifndef DISABLE_CPPRESTSDK
#include "WmsSupportInterface.h"
#include "CapabilitiesHandler.h"
#include "CapabilityTree.h"
#include "NFmiFastInfoUtils.h"
#endif // DISABLE_CPPRESTSDK

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

    
class NFmiInfoOrganizer;

namespace
{
    AddParams::SingleRowItem makeRowItem(const AddParams::CategoryData &categoryData, const std::string &uniqueId, const AddParams::SingleRowItem *rowItemMemory)
    {
        // If there is memory for this category's rowItem, use it, otherwise put categoryData in non-collapsed mode
        bool nodeCollapsed = rowItemMemory ? rowItemMemory->dialogTreeNodeCollapsed() : false;
        return AddParams::SingleRowItem(AddParams::kCategoryType, categoryData.categoryName(), 0, nodeCollapsed, uniqueId, NFmiInfoData::kNoDataType);
    }

    boost::shared_ptr<NFmiFastQueryInfo> getFastInfo(NFmiInfoOrganizer& infoOrganizer, const std::string& uniqueDataId)
    {
        auto infoVector = infoOrganizer.GetInfos(uniqueDataId);
        if(!infoVector.empty())
        {
            return infoVector.front();
        }

        return nullptr;
    }
}

namespace AddParams
{
    ParameterSelectionSystem::ParameterSelectionSystem()
    :updateWaitTimeoutInSeconds_(10)
    ,updatePending_(false)
	,staticDataAdded_(false)
    ,dialogRowData_()
    ,dialogTreePatternArray_()
    ,dialogDataNeedsUpdate_(true)
    ,modelProducerSystem_(nullptr)
	,obsProducerSystem_(nullptr)
	,satelImageProducerSystem_(nullptr)
    ,infoOrganizer_(nullptr)
    ,helpDataInfoSystem_(nullptr)
    ,itsLastActivatedDesktopIndex(0)
    ,helpDataIDs_()
    ,customCategories_()
	,soundingLevels_(nullptr)
    {
    }

    ParameterSelectionSystem::~ParameterSelectionSystem() = default;

    void ParameterSelectionSystem::initialize(NFmiProducerSystem &modelProducerSystem, NFmiProducerSystem &obsProducerSystem, NFmiProducerSystem &satelImageProducerSystem, 
        NFmiInfoOrganizer &infoOrganizer, NFmiHelpDataInfoSystem &helpDataInfoSystem, const std::vector<int> &idVector, const std::vector<std::string> &customCategories)
    {
        modelProducerSystem_ = &modelProducerSystem;
        obsProducerSystem_ = &obsProducerSystem;
        satelImageProducerSystem_ = &satelImageProducerSystem;
        infoOrganizer_ = &infoOrganizer;
        helpDataInfoSystem_ = &helpDataInfoSystem;
        helpDataIDs_ = idVector; // Help Data id's. These are added to Help Data Category
        customCategories_ = customCategories;
    } 

    // Kun esim. luetaan Case-study dataa, pitää koko systeemi alustaa uusiksi
    void ParameterSelectionSystem::reInitialize(NFmiProducerSystem& modelProducerSystem, NFmiProducerSystem& obsProducerSystem, 
        NFmiProducerSystem& satelImageProducerSystem, NFmiHelpDataInfoSystem& helpDataInfoSystem)
    {
        modelProducerSystem_ = &modelProducerSystem;
        obsProducerSystem_ = &obsProducerSystem;
        satelImageProducerSystem_ = &satelImageProducerSystem;
        helpDataInfoSystem_ = &helpDataInfoSystem;
        clearData();
    }

    void ParameterSelectionSystem::clearData()
    {
        categoryDataVector_.clear();
        // Laitetaan tämä false:ksi, jotta varmistetaan dialogin update toiminto myöhemmin
        updatePending_ = false; 
        dialogRowData_.clear();
        dialogTreePatternArray_.clear();
        dialogDataNeedsUpdate_ = true;
    }

    void ParameterSelectionSystem::addHelpData(const NFmiProducer &producer, const std::string &menuString, NFmiInfoData::Type dataType, const std::string &displayName) //Add at the end of help data list
    {
        std::string uniqueDataId = std::string(producer.GetName()) + " - " + menuString;
        SingleRowItem item = SingleRowItem(kParamType, menuString, producer.GetIdent(), true, uniqueDataId, dataType, 0, "", true, nullptr, 2, displayName);
        otherHelpData_.push_back(item);
    }

    void ParameterSelectionSystem::updateData()
    {
        updateOperationalData(OperationalDataStr, NFmiInfoData::kEditable);
        updateData(ModellDataStr, *modelProducerSystem_, NFmiInfoData::kViewable);
        updateData(ObservationDataStr, *obsProducerSystem_, NFmiInfoData::kObservations);
        updateData(SatelliteImagesStr, *satelImageProducerSystem_, NFmiInfoData::kSatelData);
		updateWmsData(WmsStr, NFmiInfoData::kWmsData);
        updateMacroParamData(MacroParametersStr, NFmiInfoData::kMacroParam);
        updateCustomCategories();
        updateData(HelpDataStr, *modelProducerSystem_, NFmiInfoData::kModelHelpData);
		updateData(HelpDataStr, *obsProducerSystem_, NFmiInfoData::kModelHelpData);
		updateData(HelpDataStr, *obsProducerSystem_, NFmiInfoData::kStationary);
    }

    void ParameterSelectionSystem::updateData(const std::string &catName, NFmiProducerSystem &producerSystem, NFmiInfoData::Type dataCategory, bool customCategory)
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

    void ParameterSelectionSystem::updateOperationalData(const std::string &categoryName, NFmiInfoData::Type dataCategory)
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

    void ParameterSelectionSystem::updateMacroParamData(const std::string &categoryName, NFmiInfoData::Type dataCategory)
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

	void ParameterSelectionSystem::updateWmsData(const std::string &categoryName, NFmiInfoData::Type dataCategory)
	{
	#ifndef DISABLE_CPPRESTSDK

		try
		{
			auto wmsSupport = getWmsCallback_();
			if (!wmsSupport->isCapabilityTreeAvailable())
				return;
			auto layerTree = wmsSupport->getCapabilityTree();
            if(layerTree)
            {
                const auto& wmsLayerTree = dynamic_cast<const Wms::CapabilityNode&>(*layerTree);

                auto iter = std::find_if(categoryDataVector_.begin(), categoryDataVector_.end(), [categoryName](const auto& categoryData) {return categoryName == categoryData->categoryName(); });
                if(iter != categoryDataVector_.end())
                {
                    dialogDataNeedsUpdate_ |= (*iter)->updateWmsData(wmsLayerTree, dataCategory);
                }
                else
                {
                    // Add wms layers as a new category
                    auto categoryDataPtr = std::make_unique<CategoryData>(categoryName, dataCategory);
                    categoryDataPtr->updateWmsData(wmsLayerTree, dataCategory);
                    categoryDataVector_.push_back(std::move(categoryDataPtr));
                    dialogDataNeedsUpdate_ = true;
                }

                updatePending(false);
            }
		}
		catch (...)
		{
		}
	#endif // DISABLE_CPPRESTSDK
	}

    void ParameterSelectionSystem::updateCustomCategories()
    {
        for(const auto &customCat : customCategories_)
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
            auto gategoryRowData = category->makeDialogRowData(dialogRowDataMemory, *infoOrganizer_, isMapViewCase());
            dialogRowData_.insert(dialogRowData_.end(), gategoryRowData.begin(), gategoryRowData.end());
			if (category->categoryName() == HelpDataStr) { otherHelpDataTodialog(); }
        }
		trimDialogRowDataDependingOnActiveView();
    }

    bool ParameterSelectionSystem::isMapViewCase() const
    {
        return itsLastActivatedDesktopIndex <= CtrlViewUtils::kFmiMaxMapDescTopIndex;
    }

	void ParameterSelectionSystem::otherHelpDataTodialog()
	{
		for (const auto& rowItem : otherHelpData_)
		{
			dialogRowData_.push_back(rowItem);
		}
	}

	void ParameterSelectionSystem::trimDialogRowDataDependingOnActiveView()
	{
		std::vector<AddParams::SingleRowItem> trimmedRowData;
		int index = 0;
		bool suitableCategory = true;

		if (itsLastActivatedDesktopIndex == CtrlViewUtils::kFmiCrossSectionView)
		{
			dialogRowData_.swap(crossSectionData());
		}
		else if (itsLastActivatedDesktopIndex == CtrlViewUtils::kFmiTimeSerialView)
		{
			dialogRowData_.swap(timeSeriesData());
		}
	}

	std::vector<SingleRowItem> ParameterSelectionSystem::crossSectionData()
	{
		std::vector<AddParams::SingleRowItem> trimmedRowData;
		int index = 0;
		bool suitableCategory = true;

		for (const auto& row : dialogRowData_)
		{
			if (row.rowType() == AddParams::RowType::kCategoryType)
				suitableCategory = (row.displayName() == ModellDataStr || row.displayName() == MacroParametersStr) ? true : false;

			if (!suitableCategory)
			{
				index++;
				continue;
			}

			if (row.rowType() == AddParams::RowType::kCategoryType || row.rowType() == AddParams::RowType::kProducerType)
			{
				trimmedRowData.push_back(row);
			}
			if (row.dataType() == NFmiInfoData::kMacroParam)
			{
				trimmedRowData.push_back(row);
			}
			auto infoVector = infoOrganizer_->GetInfos(row.uniqueDataId());
			if (!infoVector.empty())
			{
				const auto &info = infoVector.at(0);
				if (info->SizeLevels() > 1 && info->IsGrid())
				{
					trimmedRowData.push_back(row);
					auto subRows = addSubmenu(row, index);
					trimmedRowData.insert(trimmedRowData.end(), subRows.begin(), subRows.end());
				}
			}
			index++;
		}
		removeNodesThatDontHaveLeafs(trimmedRowData);
		return trimmedRowData;
	}

    std::vector<SingleRowItem> ParameterSelectionSystem::timeSeriesData()
    {
        std::vector<AddParams::SingleRowItem> trimmedRowData;
        int index = 0;
        bool suitableCategory = true;

        for(auto& row : dialogRowData_)
        {
            if(row.rowType() == AddParams::RowType::kCategoryType || row.rowType() == AddParams::RowType::kProducerType)
            {
                trimmedRowData.push_back(row);
            }
            else if(row.itemId() == 2001 || row.parentItemId() == 2001)
            {
                trimmedRowData.push_back(row);
            }
            if(row.dataType() == NFmiInfoData::kMacroParam)
            {
                trimmedRowData.push_back(row);
            }
            if(row.rowType() == AddParams::RowType::kDataType)
            {
                auto hasActualGridDataVariable = hasActualGridData(row);
                auto isOservationDataVariable = isObservationsData(row, index);
                if(hasActualGridDataVariable || isOservationDataVariable)
                {
                    trimmedRowData.push_back(row);
                    auto subRows = addAllChildNodes(row, index);
                    trimmedRowData.insert(trimmedRowData.end(), subRows.begin(), subRows.end());
                }
            }
            index++;
        }
        removeNodesThatDontHaveLeafs(trimmedRowData);
        return trimmedRowData;
    }

    bool ParameterSelectionSystem::hasActualGridData(const SingleRowItem& row)
    {
        auto info = getFastInfo(*infoOrganizer_, row.uniqueDataId());
        if(info && info->IsGrid())
        {
            return true;
        }

        // Editoitavan datan erikoistapaus: aikasarjaan pitää hyväksyä editoitava data, vaikka 
        // sitä ei löydy infoOrganizerilta (sille löytyy syy) ja se ei ole hiladataa.
        if(row.itemName() == AddParams::CategoryData::GetEditableDataRowName())
        {
            return true;
        }

        return false;
    }

	bool ParameterSelectionSystem::isObservationsData(const SingleRowItem& row, int index)
	{
        // Ensin pitää blokata mahdolliset lightning tyyppiset datat, sillä niitä ei voi visualisoida aikasarjassa.
        auto info = getFastInfo(*infoOrganizer_, row.uniqueDataId());
        if(info)
        {
            if(NFmiFastInfoUtils::IsLightningTypeData(info))
            {
                return false;
            }
        }

		if (row.rowType() == AddParams::RowType::kDataType)
		{
			return dialogRowData_.at(++index).dataType() == NFmiInfoData::kObservations;
		}
		return false;
	}

	std::vector<SingleRowItem> ParameterSelectionSystem::addSubmenu(const SingleRowItem& row, int index)
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

	std::vector<SingleRowItem> ParameterSelectionSystem::addAllChildNodes(const SingleRowItem& row, int index)
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

    void ParameterSelectionSystem::searchItemsThatMatchToSearchWords(const std::string &words)
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
        for(const auto &row : dialogRowData_)
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
        for(const auto &row : resultRowData)
        {
            if(row.leafNode() || (itsLastActivatedDesktopIndex == CtrlViewUtils::kFmiCrossSectionView && row.crossSectionLeafNode())) 
			{ 
				rowData.push_back(row); 
			}
            else if((index + 1 < resultRowData.size()))
            {
				if (resultRowData.at(index + 1).treeDepth() != row.treeDepth())
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
        for(const auto &row : resultRowData)
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

    int ParameterSelectionSystem::LastActivatedRowIndex() const
    {
        return GetLastActivatedRowIndexFromWantedDesktop(itsLastActivatedDesktopIndex);
    }

    void ParameterSelectionSystem::SetLastActiveIndexes(unsigned int desktopIndex, int rowIndex)
    {
        itsLastActivatedDesktopIndex = desktopIndex;
        if(isMapViewCase())
            itsLastActivatedMapRowIndex = rowIndex;
        else if(itsLastActivatedDesktopIndex == CtrlViewUtils::kFmiCrossSectionView)
            itsLastActivatedCrossSectionRowIndex = rowIndex;
        else if(itsLastActivatedDesktopIndex == CtrlViewUtils::kFmiTimeSerialView)
            itsLastActivatedTimeSerialRowIndex = rowIndex;
    }

    int ParameterSelectionSystem::GetLastActivatedRowIndexFromWantedDesktop(unsigned int desktopIndex) const
    {
        if(isMapViewCase())
            return itsLastActivatedMapRowIndex;
        else if(desktopIndex == CtrlViewUtils::kFmiCrossSectionView)
            return itsLastActivatedCrossSectionRowIndex;
        else if(desktopIndex == CtrlViewUtils::kFmiTimeSerialView)
            return itsLastActivatedTimeSerialRowIndex;
        else
            return 1; // virhe, pitäisi oikeasti kai heittää poikkeus
    }

}
