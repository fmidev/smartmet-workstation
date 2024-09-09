#include "CategoryData.h"
#include "ProducerData.h"
#include "ParameterSelectionUtils.h"
#include "NFmiProducerSystem.h"
#include "NFmiMacroParamSystem.h"
#include "NFmiInfoOrganizer.h"
#include "NFmiHelpDataInfo.h"
#include "NFmiOwnerInfo.h"
#include "SingleRowItem.h"
#include "NFmiDictionaryFunction.h"
#include "ParameterSelectionSystem.h"
#ifndef DISABLE_CPPRESTSDK
#include "CapabilityTree.h"
#endif // DISABLE_CPPRESTSDK
#include "cppext/tree.h"

#include <boost/algorithm/string.hpp>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

namespace
{
    AddParams::SingleRowItem makeRowItem(const AddParams::ProducerData &producerData, const std::string &uniqueId, const AddParams::SingleRowItem *rowItemMemory)
    {
        // If there is memory for this producer's rowItem, use it, otherwise put producerData in collapsed mode
        bool nodeCollapsed = rowItemMemory ? rowItemMemory->dialogTreeNodeCollapsed() : true;
        const auto &producer = producerData.producer();
        return AddParams::SingleRowItem(AddParams::kProducerType, std::string(producer.GetName()), producer.GetIdent(), nodeCollapsed, uniqueId, NFmiInfoData::kNoDataType);
    }

    std::map<std::string, boost::shared_ptr<NFmiFastQueryInfo>> operationalProducers(NFmiInfoOrganizer &infoOrganizer)
    {
        std::map<std::string, boost::shared_ptr<NFmiFastQueryInfo>> operationalData;
        operationalData.emplace(AddParams::CategoryData::GetEditableDataRowName(), infoOrganizer.FindInfo(NFmiInfoData::kEditable));
        //operationalData.emplace("Comparison data", infoOrganizer.FindInfo(NFmiInfoData::kCopyOfEdited));
        operationalData.emplace("Help editor data", infoOrganizer.FindInfo(NFmiInfoData::kEditingHelpData));
        operationalData.emplace("Operational data", infoOrganizer.FindInfo(NFmiInfoData::kKepaData));

        return operationalData;
    }

    AddParams::RowType EnumOfIndex(int i) { return static_cast<AddParams::RowType>(i); }
    int IndexOfEnum(AddParams::RowType e) { return static_cast<int>(e); }

    std::vector<AddParams::SingleRowItem> addSpecificSoundingLevels(const NFmiDataIdent &dataIdent, NFmiInfoData::Type dataType, AddParams::RowType rowType, int parentId, const NFmiLevelBag* soundingLevels, int treeDepth)
    {
        treeDepth++;
        rowType = EnumOfIndex(IndexOfEnum(rowType) - 1);
        std::vector<AddParams::SingleRowItem> items;
        NFmiLevelBag* levels = const_cast<NFmiLevelBag*>(soundingLevels);

        for(levels->Reset(); levels->Next();)
        {
            auto lvl = levels->Level();
            const std::shared_ptr<NFmiLevel> level = std::make_shared<NFmiLevel>(NFmiLevel(lvl->GetIdent(), lvl->GetName(), lvl->LevelValue()));
            std::string levelStr = level->GetName();
            std::string menuString = dataIdent.GetParamName() + " " + levelStr;
            std::string uniqueDataId = "Temp - " + menuString;

            AddParams::SingleRowItem item = AddParams::SingleRowItem(rowType, menuString, dataIdent.GetParamIdent(), true, uniqueDataId, NFmiInfoData::kObservations, parentId, "", true, level, treeDepth, menuString);
            items.push_back(item);
        }
        return items;
    }

    std::vector<AddParams::SingleRowItem> soundingSubMenu(NFmiParamBag &theParamBag, NFmiInfoData::Type theDataType, int parentId, const NFmiLevelBag* soundingLevels, int treeDepth, AddParams::RowType rowType)
    {
        treeDepth++;
        rowType = EnumOfIndex(IndexOfEnum(rowType) - 1);
        std::vector<AddParams::SingleRowItem> subMenuItems;

        if(!theParamBag.ParamsVector().empty())
        {
            //First put params into vector and sort, then create menu items
            std::vector<NFmiDataIdent> paramsVector;
            for(auto &dataIdent : theParamBag.ParamsVector())
            {
                paramsVector.push_back(dataIdent);
            }
            std::sort(paramsVector.begin(), paramsVector.end(), ([](const auto &a, const auto &b) { return boost::algorithm::ilexicographical_compare(std::string(a.GetParamName()), std::string(b.GetParamName())); }));

            for(const auto &dataIdent : paramsVector)
            {
                std::string menuString = dataIdent.GetParamName();
                std::string uniqueDataId = "Temp - " + menuString;

                AddParams::SingleRowItem item = AddParams::SingleRowItem(rowType, menuString, dataIdent.GetParamIdent(), true, uniqueDataId, NFmiInfoData::kObservations, parentId, "", false, nullptr, treeDepth, menuString);
                subMenuItems.push_back(item);
                std::vector<AddParams::SingleRowItem> subItems;
                if(dataIdent.HasDataParams()) //Wind submenu
                {
                    subItems = soundingSubMenu(*(dataIdent.GetDataParams()), theDataType, parentId, soundingLevels, treeDepth, rowType);
                }
                else
                {
                    subItems = addSpecificSoundingLevels(dataIdent, theDataType, rowType, parentId, soundingLevels, treeDepth);
                }
                subMenuItems.insert(subMenuItems.end(), subItems.begin(), subItems.end());
            }
        }
        return subMenuItems;
    }

    bool isObservationCustomMenuFolder(NFmiInfoData::Type dataCategory, const NFmiHelpDataInfo &helpDataInfo)
    {
        // Jos kategoria on havainnot, pit‰‰ tehd‰ poikkeus "observation" nimisist‰ CustomMenuFolder:eista
        if(dataCategory == NFmiInfoData::kObservations && boost::iequals(helpDataInfo.CustomMenuFolder(), "observation"))
            return true;
        else
            return false;
    }
}

namespace AddParams
{
    CategoryData::CategoryData(const std::string &categoryName, NFmiInfoData::Type dataCategory)
    :categoryName_(categoryName)
    ,dataCategory_(dataCategory)
    ,producerDataVector_()
    ,soundingLevels_()
	,staticDataAdded_(false)
    {
    }

    CategoryData::~CategoryData() = default;

    // Returns true, if new producer is added or if some new producer data is added or data's param or level structure is changed
    bool CategoryData::updateData(NFmiProducerSystem &categoryProducerSystem, NFmiInfoOrganizer &infoOrganizer, NFmiHelpDataInfoSystem &helpDataInfoSystem, 
        NFmiInfoData::Type dataCategory, const std::vector<int> &helpDataIDs, bool customCategory)
    {
        bool dataStructuresChanged = false;

		if (dataCategory == NFmiInfoData::kStationary)
		{
			dataStructuresChanged = addStaticData(infoOrganizer, helpDataInfoSystem, dataCategory);
		}
		else
		{
			dataStructuresChanged = customCategory ? updateCustomCategoryData(categoryProducerSystem, infoOrganizer, helpDataInfoSystem, dataCategory) : 
				updateNormalData(categoryProducerSystem, infoOrganizer, helpDataInfoSystem, dataCategory, helpDataIDs);
		}

        return dataStructuresChanged;
    }

    void CategoryData::setSoungindLevels(const NFmiLevelBag &soundingLevels)
    {
        soundingLevels_ = &soundingLevels;
    }

    std::string CategoryData::GetEditableDataRowName()
    {
        static std::string editableDataRowName = "Editable data";
        return editableDataRowName;
    }

    bool CategoryData::updateNormalData(NFmiProducerSystem &categoryProducerSystem, NFmiInfoOrganizer &infoOrganizer, NFmiHelpDataInfoSystem &helpDataInfoSystem,
        NFmiInfoData::Type dataCategory, const std::vector<int> &helpDataIDs)
    {
        bool dataStructuresChanged = false;

        for(const auto &producerInfo : categoryProducerSystem.Producers())
        {
            if(producerInfo.ProducerId() == kFmiTEMP)
                continue;
            NFmiProducer producer(producerInfo.ProducerId(), producerInfo.Name());
            bool helpData = std::find(helpDataIDs.begin(), helpDataIDs.end(), producerInfo.ProducerId()) != helpDataIDs.end();
            auto dataType = getDataType(infoOrganizer, producer);
            if(dataType == NFmiInfoData::kNoDataType)
                continue;

            if(dataCategory == NFmiInfoData::kEditable)
            {
                dataStructuresChanged = addNewOrUpdateData(producer, infoOrganizer, helpDataInfoSystem, dataCategory);
            }
            else if(dataCategory == NFmiInfoData::kViewable && !helpData && dataType != NFmiInfoData::kKepaData)
            {
                dataStructuresChanged = addNewOrUpdateData(producer, infoOrganizer, helpDataInfoSystem, dataCategory);
            }
            else if(dataCategory == NFmiInfoData::kObservations && !helpData)
            {
                dataStructuresChanged = addNewOrUpdateData(producer, infoOrganizer, helpDataInfoSystem, dataCategory);
            }
            else if(dataCategory == NFmiInfoData::kSatelData)
            {
                dataStructuresChanged = addNewOrUpdateData(producer, infoOrganizer, helpDataInfoSystem, dataCategory);
            }
            else if(dataCategory == NFmiInfoData::kMacroParam)
            {
                dataStructuresChanged = addNewOrUpdateData(producer, infoOrganizer, helpDataInfoSystem, dataCategory);
            }
            else if(dataCategory == NFmiInfoData::kModelHelpData && helpData)
            {
                dataStructuresChanged = addNewOrUpdateData(producer, infoOrganizer, helpDataInfoSystem, dataCategory);
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
                auto infoVector = infoOrganizer.GetInfos(info.UsedFileNameFilter(helpDataInfoSystem));
                if(infoVector.size())
                {
                    //A little bit of fiddle to get the producer name that is used in model_producer.conf file.
                    auto queryInfo = infoVector[0];
                    auto index = categoryProducerSystem.FindProducerInfo(NFmiProducer(queryInfo->Producer()->GetIdent()));
                    if(index != 0)
                    {
                        auto producer = categoryProducerSystem.Producer(index).GetProducer();
                        dataStructuresChanged = addNewOrUpdateData(producer, infoOrganizer, helpDataInfoSystem, dataCategory, true);
                    }
                }
            }
        }
        return dataStructuresChanged;
    }

    bool CategoryData::updateOperationalData(NFmiInfoOrganizer &infoOrganizer, NFmiHelpDataInfoSystem &helpDataInfoSystem, NFmiInfoData::Type dataCategory)
    {
        bool dataStructuresChanged = false;
        auto infos = operationalProducers(infoOrganizer);
        for(const auto &info : infos)
        {
            if(!info.second)
                continue;
            auto producer = NFmiProducer(info.second->Producer()->GetIdent(), info.first);
            auto iter = std::find_if(producerDataVector_.begin(), producerDataVector_.end(), [producer](const auto &producerData) {return producer.GetName() == producerData->producer().GetName(); });
            if(iter != producerDataVector_.end())
            {
                dataStructuresChanged |= (*iter)->updateOperationalData(info.second, helpDataInfoSystem);
            }
            else
            {
                auto producerDataPtr = std::make_unique<ProducerData>(producer, dataCategory);
                producerDataPtr->updateOperationalData(info.second, helpDataInfoSystem);
                producerDataVector_.push_back(std::move(producerDataPtr));
                dataStructuresChanged = true;
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
            // Add macro params as a new producer
            auto producerDataPtr = std::make_unique<ProducerData>(producer, dataCategory);
            producerDataPtr->updateMacroParamData(macroParamTree);
            producerDataVector_.push_back(std::move(producerDataPtr));
            dataStructuresChanged = true;
        }
        return dataStructuresChanged;
    }

	// Returns true, if new wms layers are added
	bool CategoryData::updateWmsData(const cppext::Node<Wms::Capability>& wmsLayerTree, NFmiInfoData::Type dataCategory)
	{
#ifndef DISABLE_CPPRESTSDK
		bool dataStructuresChanged = false;
		NFmiProducer producer(wmsLayerTree.value.paramId, "WMS");
		auto iter = std::find_if(producerDataVector_.begin(), producerDataVector_.end(), [producer](const auto& producerData) {return producer == producerData->producer(); });
		if (iter != producerDataVector_.end())
		{
			dataStructuresChanged |= (*iter)->updateWmsData(wmsLayerTree);
		}
		else
		{
			// Add wms as a new producer
			auto producerDataPtr = std::make_unique<ProducerData>(producer, dataCategory);
			producerDataPtr->updateWmsData(wmsLayerTree);
			producerDataVector_.push_back(std::move(producerDataPtr));
			dataStructuresChanged = true;
		}
		return dataStructuresChanged;
#endif
	}

    bool CategoryData::addNewOrUpdateData(const NFmiProducer &producer, NFmiInfoOrganizer &infoOrganizer, NFmiHelpDataInfoSystem &helpDataInfoSystem, NFmiInfoData::Type dataCategory, bool customCategory)
    {
        bool dataStruckturesChanged = false;
        // Add only when handling custom category
        if(!customCategory && skipCustomProducerData(producer, infoOrganizer, helpDataInfoSystem, dataCategory))
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

	bool CategoryData::addStaticData(NFmiInfoOrganizer& infoOrganizer, NFmiHelpDataInfoSystem& helpDataInfoSystem, NFmiInfoData::Type dataCategory)
	{
		bool dataStructuresChanged = false;
		if (staticDataAdded_) return false;

		boost::shared_ptr<NFmiFastQueryInfo> info = infoOrganizer.FindInfo(NFmiInfoData::kStationary);
		if (!info) return false;

		const auto &producer = *info->Producer();
		dataStructuresChanged = addNewOrUpdateData(producer, infoOrganizer, helpDataInfoSystem, dataCategory);
		if (dataStructuresChanged) staticDataAdded_ = true;

		return dataStructuresChanged;
	}

    bool CategoryData::skipCustomProducerData(const NFmiProducer &producer, NFmiInfoOrganizer &infoOrganizer, NFmiHelpDataInfoSystem &helpDataInfoSystem, NFmiInfoData::Type dataCategory)
    {
        auto producerData = infoOrganizer.GetInfos(producer.GetIdent());
        for(auto &info : producerData)
        {
            auto filePattern = info->DataFilePattern();
            NFmiHelpDataInfo *helpDataInfo = helpDataInfoSystem.FindHelpDataInfo(filePattern);
            if(helpDataInfo && !helpDataInfo->CustomMenuFolder().empty())
            {
                if(!::isObservationCustomMenuFolder(dataCategory, *helpDataInfo))
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
        if(producerDataVector_.size() > 1)
        { 
            std::sort(producerDataVector_.begin(), producerDataVector_.end(), ([](const auto &a, const auto &b) 
                { return boost::algorithm::ilexicographical_compare(std::string(a->producer().GetName()), std::string(b->producer().GetName())); }));
        }
    }

    std::vector<SingleRowItem> CategoryData::makeDialogRowData(const std::vector<SingleRowItem> &dialogRowDataMemory, NFmiInfoOrganizer &infoOrganizer, bool mapViewCase) const
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
        if(mapViewCase && categoryName() == "Observation data")
        {
            auto data = customObservationData(infoOrganizer);
            dialogRowData.insert(dialogRowData.end(), data.begin(), data.end());
        }
        return dialogRowData;
    }

    NFmiInfoData::Type CategoryData::getDataType(NFmiInfoOrganizer &infoOrganizer, const NFmiProducer &producer)
    {
        auto fastQueryInfoVector = infoOrganizer.GetInfos(producer.GetIdent());
        auto dataType = !fastQueryInfoVector.empty() ? fastQueryInfoVector.at(0)->DataType() : NFmiInfoData::kNoDataType;
        return dataType;
    }

    std::vector<SingleRowItem> CategoryData::customObservationData(NFmiInfoOrganizer &infoOrganizer) const
    {
        int treeDepth = 2;
        AddParams::RowType rowType = kDataType;
        std::vector<SingleRowItem> customData;
        
        // *** Sounding and sounding plot ***
        boost::shared_ptr<NFmiFastQueryInfo> soundingInfo = infoOrganizer.GetPrioritizedSoundingInfo(NFmiInfoOrganizer::ParamCheckFlags(true));
        if(soundingInfo)
        {
            NFmiInfoData::Type soundingType = soundingInfo->DataType();
            const std::shared_ptr<NFmiLevel> defaultLevel = std::make_shared<NFmiLevel>(NFmiLevel(50, "850xxx", 850));

            std::string menuString = "Sounding";
            std::string uniqueDataId = "Temp - " + menuString;
            SingleRowItem item = SingleRowItem(rowType, menuString, NFmiProducer(kFmiTEMP).GetIdent(), true, uniqueDataId, NFmiInfoData::kObservations, 0, "", false, nullptr, treeDepth, menuString);
            auto paramBag = soundingInfo->ParamBag();
            customData.push_back(item);
            auto subMenuItems = ::soundingSubMenu(paramBag, soundingType, NFmiProducer(kFmiTEMP).GetIdent(), soundingLevels_, treeDepth, kParamType);
            customData.insert(customData.end(), subMenuItems.begin(), subMenuItems.end());
            
            menuString = "Sounding plot";
            auto param = NFmiParam(NFmiInfoData::kFmiSpSoundingPlot, "temp");
            uniqueDataId = "Temp - " + menuString;
            item = SingleRowItem(rowType, menuString, param.GetIdent(), true, uniqueDataId, NFmiInfoData::kObservations, 0, "", true, defaultLevel, treeDepth, menuString);
            customData.push_back(item);
        } 
        // *** Lightning ***
        if(infoOrganizer.FindInfo(NFmiInfoData::kFlashData, 0))
        {
            NFmiProducer producer(*(infoOrganizer.FindInfo(NFmiInfoData::kFlashData, 0)->Producer()));
            std::string menuString = ::GetDictionaryString("MapViewParamPopUpFlashData");
            std::string uniqueDataId = std::string(producer.GetName()) + " - " + menuString;
            SingleRowItem item = SingleRowItem(rowType, menuString, producer.GetIdent(), true, uniqueDataId, NFmiInfoData::kFlashData, 0, "", true, nullptr, treeDepth, menuString);
            customData.push_back(item);
        }
        // *** Synop plot ***
        if(infoOrganizer.FindInfo(NFmiInfoData::kObservations, NFmiProducer(kFmiSYNOP), true) != 0)
        {
            NFmiProducer producer(*(infoOrganizer.FindInfo(NFmiInfoData::kObservations, NFmiProducer(kFmiSYNOP), true)->Producer()));
            std::string menuString = ::GetDictionaryString("MapViewParamPopUpSynopPlot");
            std::string uniqueDataId = std::string(producer.GetName()) + " - " + menuString;
            auto param = NFmiParam(NFmiInfoData::kFmiSpSynoPlot, "synop");
            SingleRowItem item = SingleRowItem(rowType, menuString, param.GetIdent(), true, uniqueDataId, NFmiInfoData::kObservations, 0, "", true, nullptr, treeDepth, menuString);
            customData.push_back(item);

            // Add also a min/max synop plot
            NFmiProducer producer2(*(infoOrganizer.FindInfo(NFmiInfoData::kObservations, NFmiProducer(kFmiSYNOP), true)->Producer()));
            menuString = "Synop min/max";
            uniqueDataId = std::string(producer2.GetName()) + " - " + menuString;
            param = NFmiParam(NFmiInfoData::kFmiSpMinMaxPlot, "min/max");
            item = SingleRowItem(rowType, menuString, param.GetIdent(), true, uniqueDataId, NFmiInfoData::kObservations, 0, "", true, nullptr, treeDepth, menuString);
            customData.push_back(item);
        }
        // *** Metar plot ***
        if(infoOrganizer.FindInfo(NFmiInfoData::kObservations, NFmiProducer(kFmiMETAR), true) != 0)
        {
            NFmiProducer producer(*(infoOrganizer.FindInfo(NFmiInfoData::kObservations, NFmiProducer(kFmiMETAR), true)->Producer()));
            std::string menuString = ::GetDictionaryString("Metar plot");
            std::string uniqueDataId = std::string(producer.GetName()) + " - " + menuString;
            auto param = NFmiParam(NFmiInfoData::kFmiSpMetarPlot, "metar");
            SingleRowItem item = SingleRowItem(rowType, menuString, param.GetIdent(), true, uniqueDataId, NFmiInfoData::kObservations, 0, "", true, nullptr, treeDepth, menuString);
            customData.push_back(item);
        }

        return customData;
    }

}
