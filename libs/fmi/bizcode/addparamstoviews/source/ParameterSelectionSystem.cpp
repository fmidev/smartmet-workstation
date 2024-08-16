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
#include "NFmiFastInfoUtils.h"
#ifndef DISABLE_CPPRESTSDK
#include "WmsSupportInterface.h"
#include "CapabilityTree.h"
#include "CapabilitiesHandler.h" // Tuottaa C++17 ympäristössä ParameterSelectionSystem.cpp tiedostossa byte ongelman
#endif // DISABLE_CPPRESTSDK

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

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
        dialogDataNeedsUpdate(true);
        dataNeedsUpdate(true);
        queryDataNeedsUpdate(true);
        imageDataNeedsUpdate(true);
        wmsDataNeedsUpdate(true);
        macroParamDataNeedsUpdate(true);
    }

    void ParameterSelectionSystem::addHelpData(const NFmiProducer &producer, const std::string &menuString, NFmiInfoData::Type dataType, const std::string &displayName) //Add at the end of help data list
    {
        std::string uniqueDataId = std::string(producer.GetName()) + " - " + menuString;
        SingleRowItem item = SingleRowItem(kParamType, menuString, producer.GetIdent(), true, uniqueDataId, dataType, 0, "", true, nullptr, 2, displayName);
        otherHelpData_.push_back(item);
    }

    std::string MakeBoolLogString(const std::string& name, bool value)
    {
        std::string str = name + " = ";
        str += value ? "true" : "false";
        return str;
    }

    std::string MakeInfoDataString(NFmiInfoData::Type infoData)
    {
        switch(infoData)
        {
        case NFmiInfoData::kNoDataType:
            return "NoDataType";
        case NFmiInfoData::kEditable:
            return "Editable";
        case NFmiInfoData::kViewable:
            return "Viewable";
        case NFmiInfoData::kStationary:
            return "Stationary";
        case NFmiInfoData::kCopyOfEdited:
            return "CopyOfEdited";
        case NFmiInfoData::kObservations:
            return "Observations";
        case NFmiInfoData::kCalculatedValue:
            return "CalculatedValue";
        case NFmiInfoData::kKepaData:
            return "KepaData";
        case NFmiInfoData::kClimatologyData:
            return "ClimatologyData";
        case NFmiInfoData::kAnalyzeData:
            return "AnalyzeData";
        case NFmiInfoData::kSatelData:
            return "SatelData";
        case NFmiInfoData::kMacroParam:
        case NFmiInfoData::kCrossSectionMacroParam:
        case NFmiInfoData::kQ3MacroParam:
        case NFmiInfoData::kTimeSerialMacroParam:
            return "MacroParam";
        case NFmiInfoData::kHybridData:
            return "HybridData";
        case NFmiInfoData::kModelHelpData:
            return "ModelHelpData";
        case NFmiInfoData::kEditingHelpData:
            return "EditingHelpData";
        case NFmiInfoData::kConceptualModelData:
            return "ConceptualModelData";
        case NFmiInfoData::kWmsData:
            return "WmsData";
        default:
            return "UnknownData";
        }
    }

    std::string ParameterSelectionSystem::MakeBoolLogStringUpdate() const
    {
        return MakeBoolLogString("dataNeedsUpdate", dataNeedsUpdate());
    }

    void ParameterSelectionSystem::MakeUpdateLogging(const std::string& funcName, const std::string& stepName, const std::string& categoryName, NFmiInfoData::Type dataCategory, std::string extraInfo) const
    {
        std::string finalMessage = funcName + " " + stepName + ": " + categoryName + " ";
        if(dataCategory != NFmiInfoData::kNoDataType)
        {
            finalMessage += "'" + MakeInfoDataString(dataCategory) + "' ";
        }
        finalMessage += MakeBoolLogStringUpdate();
        if(!extraInfo.empty())
        {
            finalMessage += " " + extraInfo;
        }
        CatLog::logMessage(finalMessage, CatLog::Severity::Debug, CatLog::Category::Operational);
    }

    void ParameterSelectionSystem::updateData()
    {
        CatLog::logMessage(std::string(__FUNCTION__) + " starts: " + MakeBoolLogStringUpdate(), CatLog::Severity::Debug, CatLog::Category::Operational);
        if(dataNeedsUpdate())
        {
            dataNeedsUpdate(false);
            updateQueryData();
            updateImageData();
            updateWmsData(WmsStr, NFmiInfoData::kWmsData);
            updateMacroParamData(MacroParametersStr, NFmiInfoData::kMacroParam);
            CatLog::logMessage(std::string(__FUNCTION__) + " ends: " + MakeBoolLogStringUpdate(), CatLog::Severity::Debug, CatLog::Category::Operational);
        }
        updatePending(false);
    }

    void ParameterSelectionSystem::updateQueryData()
    {
        if(queryDataNeedsUpdate())
        {
            queryDataNeedsUpdate(false);
            updateOperationalData(OperationalDataStr, NFmiInfoData::kEditable);
            updateData(ModellDataStr, *modelProducerSystem_, NFmiInfoData::kViewable);
            updateData(ObservationDataStr, *obsProducerSystem_, NFmiInfoData::kObservations);
            updateCustomCategories();
            updateHelpData();
        }
    }

    void ParameterSelectionSystem::updateImageData()
    {
        if(imageDataNeedsUpdate())
        {
            imageDataNeedsUpdate(false);
            updateData(SatelliteImagesStr, *satelImageProducerSystem_, NFmiInfoData::kSatelData);
        }
    }

    void ParameterSelectionSystem::updateHelpData()
    {
        updateData(HelpDataStr, *modelProducerSystem_, NFmiInfoData::kModelHelpData);
        updateData(HelpDataStr, *obsProducerSystem_, NFmiInfoData::kModelHelpData);
        updateData(HelpDataStr, *obsProducerSystem_, NFmiInfoData::kStationary);
    }

    void ParameterSelectionSystem::dataNeedsUpdate(bool value)
    {
        dataNeedsUpdate_ = value;
    }

    void ParameterSelectionSystem::queryDataNeedsUpdate(bool value) 
    { 
        queryDataNeedsUpdate_ = value; 
        setMainDataFlagDirtyIfSubDataIsSet(value);
    }

    void ParameterSelectionSystem::setMainDataFlagDirtyIfSubDataIsSet(bool value)
    {
        if(value)
        {
            // Jos jokin ali-data-lippu menee likaiseksi, pitää myös 'pää'-lippu liata
            dataNeedsUpdate(true);
        }
    }

    void ParameterSelectionSystem::setDialogDataFlagDirtyIfSubDataNeedsUpdate(bool value)
    {
        if(value)
        {
            // Jos jonkin ali-datan jokin kohta on muuttunut, pitää myös dialogiDatan-lippu liata
            dialogDataNeedsUpdate(true);
        }
    }

    void ParameterSelectionSystem::imageDataNeedsUpdate(bool value) 
    { 
        imageDataNeedsUpdate_ = value; 
        setMainDataFlagDirtyIfSubDataIsSet(value);
    }

    void ParameterSelectionSystem::wmsDataNeedsUpdate(bool value) 
    { 
        wmsDataNeedsUpdate_ = value; 
        setMainDataFlagDirtyIfSubDataIsSet(value);
    }

    void ParameterSelectionSystem::macroParamDataNeedsUpdate(bool value) 
    { 
        macroParamDataNeedsUpdate_ = value; 
        setMainDataFlagDirtyIfSubDataIsSet(value);
    }

    void ParameterSelectionSystem::updateData(const std::string &catName, NFmiProducerSystem &producerSystem, NFmiInfoData::Type dataCategory, bool customCategory)
    {
        std::string categoryName = ::GetDictionaryString(catName.c_str());
        MakeUpdateLogging(__FUNCTION__, "starts", categoryName, dataCategory);
        auto iter = std::find_if(categoryDataVector_.begin(), categoryDataVector_.end(), [categoryName](const auto &categoryData) {return categoryName == categoryData->categoryName(); });
        if(iter != categoryDataVector_.end())
        {
            auto needsUpdate = (*iter)->updateData(producerSystem, *infoOrganizer_, *helpDataInfoSystem_, dataCategory, helpDataIDs_, customCategory);
            setDialogDataFlagDirtyIfSubDataNeedsUpdate(needsUpdate);
            MakeUpdateLogging(__FUNCTION__, "update-section", categoryName, dataCategory, MakeBoolLogString("needsUpdate", needsUpdate));
        }
        else
        {
            if(addNewCategoryData(categoryName, producerSystem, *infoOrganizer_, *helpDataInfoSystem_, dataCategory, customCategory))
            {
                MakeUpdateLogging(__FUNCTION__, "add-new-category-section", categoryName, dataCategory, MakeBoolLogString("customCategory", customCategory));
            }
        }

        updatePending(false);
    }

    void ParameterSelectionSystem::updateOperationalData(const std::string &categoryName, NFmiInfoData::Type dataCategory)
    {
        MakeUpdateLogging(__FUNCTION__, "starts", categoryName, dataCategory);
        auto iter = std::find_if(categoryDataVector_.begin(), categoryDataVector_.end(), [categoryName](const auto &categoryData) {return categoryName == categoryData->categoryName(); });
        if(iter != categoryDataVector_.end())
        {
            auto needsUpdate = (*iter)->updateOperationalData(*infoOrganizer_, *helpDataInfoSystem_, dataCategory);
            setDialogDataFlagDirtyIfSubDataNeedsUpdate(needsUpdate);
            MakeUpdateLogging(__FUNCTION__, "update-section", categoryName, dataCategory, MakeBoolLogString("needsUpdate", needsUpdate));
        }
        else
        {
            auto categoryDataPtr = std::make_unique<CategoryData>(categoryName, dataCategory);
            categoryDataPtr->updateOperationalData(*infoOrganizer_, *helpDataInfoSystem_, dataCategory);
            categoryDataVector_.push_back(std::move(categoryDataPtr));
            dialogDataNeedsUpdate(true);
            MakeUpdateLogging(__FUNCTION__, "new-category", categoryName, dataCategory);
        }
    }

    void ParameterSelectionSystem::updateMacroParamData(const std::string &categoryName, NFmiInfoData::Type dataCategory)
    {
        if(getMacroParamSystemCallback_)
        {
            if(macroParamDataNeedsUpdate())
            {
                macroParamDataNeedsUpdate(false);
                MakeUpdateLogging(__FUNCTION__, "starts", categoryName, dataCategory);
                auto macroParamSystemPtr = getMacroParamSystemCallback_();
                auto& macroParamTree = macroParamSystemPtr->MacroParamItemTree();

                auto iter = std::find_if(categoryDataVector_.begin(), categoryDataVector_.end(), [categoryName](const auto& categoryData) {return categoryName == categoryData->categoryName(); });
                if(iter != categoryDataVector_.end())
                {
                    auto needsUpdate = (*iter)->updateMacroParamData(macroParamTree, dataCategory);
                    setDialogDataFlagDirtyIfSubDataNeedsUpdate(needsUpdate);
                    MakeUpdateLogging(__FUNCTION__, "update-section", categoryName, dataCategory, MakeBoolLogString("needsUpdate", needsUpdate));
                }
                else
                {
                    // Add macro params as a new category
                    auto categoryDataPtr = std::make_unique<CategoryData>(categoryName, dataCategory);
                    categoryDataPtr->updateMacroParamData(macroParamTree, dataCategory);
                    categoryDataVector_.push_back(std::move(categoryDataPtr));
                    dialogDataNeedsUpdate(true);
                    MakeUpdateLogging(__FUNCTION__, "new-category", categoryName, dataCategory);
                }
            }
        }
    }

	void ParameterSelectionSystem::updateWmsData(const std::string &categoryName, NFmiInfoData::Type dataCategory)
	{
	#ifndef DISABLE_CPPRESTSDK

		try
		{
            if(wmsDataNeedsUpdate())
            {
                wmsDataNeedsUpdate(false);
                auto wmsSupport = getWmsCallback_();
                if(!wmsSupport->isCapabilityTreeAvailable())
                    return;
                auto layerTree = wmsSupport->getCapabilityTree();
                if(layerTree)
                {
                    MakeUpdateLogging(__FUNCTION__, "starts", categoryName, dataCategory);
                    const auto& wmsLayerTree = dynamic_cast<const Wms::CapabilityNode&>(*layerTree);

                    auto iter = std::find_if(categoryDataVector_.begin(), categoryDataVector_.end(), [categoryName](const auto& categoryData) {return categoryName == categoryData->categoryName(); });
                    if(iter != categoryDataVector_.end())
                    {
                        auto needsUpdate = (*iter)->updateWmsData(wmsLayerTree, dataCategory);
                        setDialogDataFlagDirtyIfSubDataNeedsUpdate(needsUpdate);
                        MakeUpdateLogging(__FUNCTION__, "update-section", categoryName, dataCategory, MakeBoolLogString("needsUpdate", needsUpdate));
                    }
                    else
                    {
                        // Add wms layers as a new category
                        auto categoryDataPtr = std::make_unique<CategoryData>(categoryName, dataCategory);
                        categoryDataPtr->updateWmsData(wmsLayerTree, dataCategory);
                        categoryDataVector_.push_back(std::move(categoryDataPtr));
                        dialogDataNeedsUpdate(true);
                        MakeUpdateLogging(__FUNCTION__, "new-category", categoryName, dataCategory);
                    }
                }
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

    bool ParameterSelectionSystem::addNewCategoryData(const std::string &categoryName, NFmiProducerSystem &producerSystem, NFmiInfoOrganizer &infoOrganizer, NFmiHelpDataInfoSystem &helpDataInfoSystem, NFmiInfoData::Type dataCategory, bool customCategory)
    {
        auto categoryDataPtr = std::make_unique<CategoryData>(categoryName, dataCategory);
        if(dataCategory == NFmiInfoData::kObservations) 
            categoryDataPtr->setSoungindLevels(*soundingLevels_);
        categoryDataPtr->updateData(producerSystem, infoOrganizer, helpDataInfoSystem, dataCategory, helpDataIDs_, customCategory);
        if(categoryDataPtr->producerDataVector().size() > 0) 
        { 
            //Hide categories that don't have data
            categoryDataVector_.push_back(std::move(categoryDataPtr));
            dialogDataNeedsUpdate(true);
            return true;
        }
        return false;
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
        if(dialogDataNeedsUpdate() || dataNeedsUpdate())
        {
            updateData();
            updateDialogRowData();
            updateDialogTreePatternData();
            dialogDataNeedsUpdate(false);
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
            dialogDataNeedsUpdate(true);
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
        dialogDataNeedsUpdate(true);
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

    // Palautetaan true, jos molemmat annetut indeksit ovat karttanäyttö tyyppisiä
    bool BothViewIndexWereMapViewType(unsigned int desktopIndex1, unsigned int desktopIndex2)
    {
        return (desktopIndex1 <= CtrlViewUtils::kFmiMaxMapDescTopIndex) && (desktopIndex2 <= CtrlViewUtils::kFmiMaxMapDescTopIndex);
    }

    void ParameterSelectionSystem::SetLastActiveIndexes(unsigned int desktopIndex, int rowIndex)
    {
        bool desktopIndexChanged = (desktopIndex != itsLastActivatedDesktopIndex);
        auto mapViewTypeRemains = BothViewIndexWereMapViewType(desktopIndex, itsLastActivatedDesktopIndex);
        if(desktopIndexChanged && !mapViewTypeRemains)
        {
            // Jos muutetaan näyttötyyppiä niin että ei pysytä karttanäyttötyypissä,
            // tällöin pitää Parameter-selection dialogia päivittää. On siis sallittua 
            // vaihtaa kartta1:stä kartta2:een ilman että pitää päivitellä.
            dataNeedsUpdate(true);
            CatLog::logMessage(std::string(__FUNCTION__) + ": active view type changed, need to update selection dialog", CatLog::Severity::Debug, CatLog::Category::Operational);
        }

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
