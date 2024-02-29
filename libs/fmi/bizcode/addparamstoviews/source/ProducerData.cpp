#include "ProducerData.h"
#include "SingleData.h"
#include "ParameterSelectionUtils.h"
#include "NFmiInfoOrganizer.h"
#include "NFmiFastQueryInfo.h"
#include "NFmiHelpDataInfo.h"
#include "NFmiMacroParamSystem.h"
#include "NFmiMacroParam.h"
#include "NFmiDrawParam.h"
#include "NFmiPathUtils.h"
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
    // SmartMetin CaseStudy moodi laitetaan tähän talteen, jotta voidaan jättää
    // normi moodissa tietyt datat (satelliittikuva datat, jotka on muuten käytöstä poistetut) 
    // pois dialogista. Jos taas ollaan CaseStudy moodissa, silloin ne lisätään käyttöön, 
    // koska talletetussa CaseStudydatapaketissa saattaa olla sellaisia vielä käytössä.
    bool gCaseStudyModeOn = false;

    bool isDataOnlyOnOneLevel(const boost::shared_ptr<NFmiFastQueryInfo> &info)
    {
        if(info)
        {
            if(info->IsGrid())
            {
                if(info->SizeLevels() == 1) //SizeLevels == 1 for surface data.
                {
                    return true;
                }
            }
        }
        return false;
    }

    void FixMissingDataItemName(AddParams::SingleRowItem& rowItem)
    {
        if(rowItem.itemName().empty())
        {
            rowItem.itemName(PathUtils::getFilename(rowItem.totalFilePath()));
        }
    }

    AddParams::SingleRowItem makeRowItem(const AddParams::SingleData &data, const std::string &uniqueId, const AddParams::SingleRowItem *rowItemMemory)
    {
        // If there is memory for this data's rowItem, use it, otherwise put row in collapsed mode
        bool nodeCollapsed = rowItemMemory ? rowItemMemory->dialogTreeNodeCollapsed() : true;

        auto singleRowItem = AddParams::SingleRowItem(AddParams::kDataType, data.dataName(), data.producerId(), nodeCollapsed, uniqueId, NFmiInfoData::kNoDataType);
        singleRowItem.origTime(data.OrigOrLastTime());
        singleRowItem.totalFilePath(data.totalLocalPath());
        FixMissingDataItemName(singleRowItem);
        return singleRowItem;
    }

    std::unique_ptr<AddParams::SingleRowItem> makeSatelliteRowItem(const NFmiProducer &producer, unsigned long paramId, const std::string &paramName, const std::string &uniqueId, NFmiInfoData::Type dataCategory)
    {
        return std::make_unique<AddParams::SingleRowItem>(AddParams::kParamType, paramName, paramId, true, uniqueId, dataCategory, producer.GetIdent(), std::string(producer.GetName()), true, nullptr, 3);
    }

    std::unique_ptr<AddParams::SingleRowItem> makeDataRowItem(const NFmiProducer &producer, unsigned long paramId, const std::string &paramName, const std::string &uniqueId, NFmiInfoData::Type dataCategory, bool leafNode, int treeDepth)
    {
        AddParams::RowType paramType = (leafNode) ? AddParams::kParamType : AddParams::kDataType;
        return std::make_unique<AddParams::SingleRowItem>(paramType, paramName, paramId, true, uniqueId, dataCategory, producer.GetIdent(), std::string(producer.GetName()), leafNode, nullptr, treeDepth);
    }

    bool areDataVectorsDifferent(std::vector<std::unique_ptr<AddParams::SingleRowItem>> &newDataVector, std::vector<std::unique_ptr<AddParams::SingleRowItem>> &oldDataVector_)
    {
        if(newDataVector.size() != oldDataVector_.size())
            return true;
        for(int i = 0; i < newDataVector.size(); i++)
        {
            if((newDataVector[i]->uniqueDataId() != oldDataVector_[i]->uniqueDataId()))
                return true;
        }
        return false;
    }
}

namespace AddParams
{
    ProducerData::ProducerData(const NFmiProducer &producer, NFmiInfoData::Type dataCategory)
    :producer_(producer)
    ,dataCategory_(dataCategory)
    ,dataVector_()
    ,satelliteDataVector_()
    ,macroParamDataVector_()
	,wmsDataVector_()
    {
    }

    ProducerData::~ProducerData() = default;

    // Returns true, if some new producer data is added or data's param or level structure is changed
    bool ProducerData::updateData(NFmiInfoOrganizer &infoOrganizer, NFmiHelpDataInfoSystem &helpDataInfoSystem, NFmiInfoData::Type dataType)
    {
        bool dataStructuresChanged = false;
        
        if(dataCategory_ == NFmiInfoData::kSatelData)
        {
            dataStructuresChanged |= updateSatelliteData(helpDataInfoSystem);
        } 
        else 
        {
            auto producerData = infoOrganizer.GetInfos(producer_.GetIdent());

            for(auto &info : producerData)
            {
                if(::isDataOnlyOnOneLevel(info))
                {
                    dataStructuresChanged |= updateData(info, infoOrganizer, helpDataInfoSystem);
                }
                else
                {
                    dataStructuresChanged |= updateData(info, infoOrganizer, helpDataInfoSystem);
                }
            }
        }

        return dataStructuresChanged;
    }

    bool ProducerData::updateData(const boost::shared_ptr<NFmiFastQueryInfo> &info, NFmiInfoOrganizer &infoOrganizer, NFmiHelpDataInfoSystem &helpDataInfoSystem)
    {
        auto fileFilter = info->DataFilePattern();
        auto iter = std::find_if(dataVector_.begin(), dataVector_.end(),
            [fileFilter] (const auto &singleData) { return fileFilter == singleData->uniqueDataId(); });
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

    bool ProducerData::updateSatelliteData(NFmiHelpDataInfoSystem &helpDataInfoSystem)
    {
        if(!satelliteDataVector_.empty()) // No need to update after initialization
            return false;

        for(int i = 0; i < helpDataInfoSystem.DynamicCount(); i++)
        {
            const auto &helpDataInfo = helpDataInfoSystem.DynamicHelpDataInfo(i);
            if(helpDataInfo.IsEnabled() && helpDataInfo.IsDataUsedCaseStudyChecks(gCaseStudyModeOn) && helpDataInfo.DataType() == NFmiInfoData::kSatelData)
            {
                int prodId = static_cast<int>(producer_.GetIdent());
                if(prodId > 0 && prodId == helpDataInfo.FakeProducerId()) //Checks that this actually is correct helpdata
                {
                    const auto &fileFilter = helpDataInfo.FileNameFilter();
                    satelliteDataVector_.push_back(::makeSatelliteRowItem(producer_, helpDataInfo.ImageDataIdent().GetParamIdent(), std::string(helpDataInfo.ImageDataIdent().GetParamName()), fileFilter, dataCategory_));
                }
            }
        }
        return !satelliteDataVector_.empty();
    }

    bool ProducerData::updateOperationalData(const boost::shared_ptr<NFmiFastQueryInfo> &info, NFmiHelpDataInfoSystem &helpDataInfoSystem)
    {
        bool dataStructuresChanged = false;
        const auto &fileFilter = info->DataFilePattern();
        if(!dataVector_.empty() && info->DataType() == NFmiInfoData::kEditable) //Comparison for editable data
        {
            if(dataVector_.front()->uniqueDataId() == info->DataFileName())
                return dataStructuresChanged;
        }
        else if(!dataVector_.empty() && (info->DataType() == NFmiInfoData::kEditingHelpData || info->DataType() == NFmiInfoData::kKepaData)) //Comparison for operational/ help data
        {
            if(dataVector_.front()->latestDataFilePath() == info->DataFileName())
                return dataStructuresChanged;
        }
        
        auto helpDataInfo = helpDataInfoSystem.FindHelpDataInfo(fileFilter);
        auto singleDataPtr = std::make_unique<SingleData>();
        dataStructuresChanged = singleDataPtr->updateOperationalData(info, helpDataInfo);
        if(dataStructuresChanged)
        {
            dataVector_.clear();
            dataVector_.push_back(std::move(singleDataPtr));
        }
        return dataStructuresChanged;
    }

    bool ProducerData::updateMacroParamData(std::vector<NFmiMacroParamItem> &macroParamTree)
    {
        int treeDepth = 3;
        std::vector<std::unique_ptr<SingleRowItem>> newDataVector = createMacroParamVector(macroParamTree, treeDepth);
        //Compare vectors and update if different
        if(::areDataVectorsDifferent(newDataVector, macroParamDataVector_))
        {
            macroParamDataVector_.clear();
            macroParamDataVector_ = std::move(newDataVector);
            return true;
        }
        return false;;
    }

	bool ProducerData::updateWmsData(const cppext::Node<Wms::Capability>& wmsLayerTree)
	{
#ifndef DISABLE_CPPRESTSDK
		int treeDepth = 3;
		std::vector<std::unique_ptr<SingleRowItem>> newDataVector = createWmsDataVector(wmsLayerTree, treeDepth);
		//Compare vectors and update if different
		if (::areDataVectorsDifferent(newDataVector, wmsDataVector_))
		{
			wmsDataVector_.clear();
			wmsDataVector_ = std::move(newDataVector);
			return true;
		}
		return false;;
#endif
	}

    std::vector<std::unique_ptr<SingleRowItem>> ProducerData::createMacroParamVector(const std::vector<NFmiMacroParamItem> &macroParamTree, int treeDepth)
    {
        std::vector<std::unique_ptr<SingleRowItem>> macroParamVector;
        macroParamsToVector(macroParamVector, macroParamTree, treeDepth);
        return macroParamVector;
    }

	std::vector<std::unique_ptr<SingleRowItem>> ProducerData::createWmsDataVector(const cppext::Node<Wms::Capability>& wmsLayerTree, int treeDepth)
	{
		std::vector<std::unique_ptr<SingleRowItem>> wmsVector;
#ifndef DISABLE_CPPRESTSDK
		wmsDataToVector(wmsVector, wmsLayerTree, treeDepth);
#endif
		return wmsVector;
	}

    int ProducerData::macroParamsToVector(std::vector<std::unique_ptr<SingleRowItem>> &macroParamVector, const std::vector<NFmiMacroParamItem> &macroParamTree, int treeDepth)
    {
        //Recursively loop through macroParamTree and create SingleRowItems
        for(auto const &macroParamRow : macroParamTree)
        {
            const NFmiMacroParamItem &macroParamItem = macroParamRow;

            const auto &param = macroParamItem.itsMacroParam;
            bool isDirectory = param->IsMacroParamDirectory();
            bool leafNode = !isDirectory;

            if(!isDirectory) //Leaf node
            {
                macroParamVector.push_back(::makeDataRowItem(producer_, producer_.GetIdent(), param->Name(), param->DrawParam()->InitFileName(), dataCategory_, leafNode, treeDepth));
            }
            else // Directory, increase treeDepth
            {
                std::string paramName = macroParamItem.itsMacroParam->Name();
                if(macroParamItem.itsMacroParam->ErrorInMacro())
                    paramName += " (ERROR)";
                auto &subtree = macroParamRow.itsDirectoryItems;
                macroParamVector.push_back(::makeDataRowItem(producer_, producer_.GetIdent(), paramName, param->MacroParamDirectoryPath(), dataCategory_, leafNode, treeDepth));
                macroParamsToVector(macroParamVector, subtree, ++treeDepth);
                treeDepth--;
            }
        }
        return treeDepth;
    }

	int ProducerData::wmsDataToVector(std::vector<std::unique_ptr<SingleRowItem>>& wmsVector, const cppext::Node<Wms::Capability>& wmsLayerTree, int treeDepth)
	{
#ifndef DISABLE_CPPRESTSDK
		//Recursively loop through wmsLayerTree and create SingleRowItems
		for (auto const& wmsChild : wmsLayerTree.children)
		{
			try
			{
				auto paramName = wmsChild->value.name;
				auto paramId = wmsChild->value.paramId;
				auto producer = wmsChild->value.producer;
				const std::string& uniqueId = paramName + "-" + std::to_string(paramId) + "-" + std::to_string(producer.GetIdent());
				bool leafNode = true;

				try
				{
					const auto& wmsNode = dynamic_cast<const Wms::CapabilityNode&>(*wmsChild);
					leafNode = wmsNode.children.size() > 0 ? false : true;
					if (!leafNode)
					{
						wmsVector.push_back(::makeDataRowItem(producer, paramId, paramName, uniqueId, dataCategory_, leafNode, treeDepth));
						wmsDataToVector(wmsVector, wmsNode, ++treeDepth);
						treeDepth--;
					}
				}
				catch (const std::exception&)
				{		
				}

				if (leafNode)
				{
                    auto tmpDataRowItem = ::makeDataRowItem(producer, paramId, paramName, uniqueId, dataCategory_, leafNode, treeDepth);
                    tmpDataRowItem->wmsLayerHasTimeDimension(wmsChild->value.hasTimeDimension);
					wmsVector.push_back(std::move(tmpDataRowItem));
				}				
			}
			catch (...)
			{
			}
		}
#endif
		return treeDepth;
	}

    void ProducerData::addNewSingleData(const boost::shared_ptr<NFmiFastQueryInfo> &info, NFmiHelpDataInfoSystem &helpDataInfoSystem)
    {
        auto helpDataInfo = helpDataInfoSystem.FindHelpDataInfo(info->DataFilePattern());
        auto singleDataPtr = std::make_unique<SingleData>();
        if(helpDataInfo)
        {
            singleDataPtr->updateData(info, helpDataInfo);
        }
        else
        {
            singleDataPtr->updateData(info);
        }
        dataVector_.push_back(std::move(singleDataPtr));
        if(dataVector_.size() > 1)
        {
            std::sort(dataVector_.begin(), dataVector_.end(), ([](const auto &a, const auto &b)
                { return boost::algorithm::ilexicographical_compare(std::string(a->dataName()), std::string(b->dataName())); }));
        }
    }

    std::vector<SingleRowItem> ProducerData::makeDialogRowData(const std::vector<SingleRowItem> &dialogRowDataMemory) const
    {
        if(dataCategory_ == NFmiInfoData::kSatelData)
            return makeDialogRowData(dialogRowDataMemory, satelliteDataVector_);

        if(dataCategory_ == NFmiInfoData::kMacroParam)
            return makeDialogRowData(dialogRowDataMemory, macroParamDataVector_);

		if (dataCategory_ == NFmiInfoData::kWmsData)
			return makeDialogRowData(dialogRowDataMemory, wmsDataVector_);

        std::vector<SingleRowItem> dialogRowData;
        for(const auto &singleData : dataVector_)
        {
            const std::string &uniqueId = singleData->uniqueDataId();
            auto *rowDataMemory = findDataRowItem(uniqueId, dialogRowDataMemory);
            dialogRowData.push_back(::makeRowItem(*singleData, uniqueId, rowDataMemory));
            auto rowData = singleData->makeDialogRowData(dialogRowDataMemory);
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

    std::vector<SingleRowItem> ProducerData::makeDialogRowData(const std::vector<SingleRowItem> &dialogRowDataMemory, const std::vector<std::unique_ptr<SingleRowItem>> &thisDataVector) const
    {
        std::vector<SingleRowItem> dialogRowData;
        dialogRowData.reserve(thisDataVector.size());
        for(const auto &singleRowData : thisDataVector)
        {
            dialogRowData.push_back(*singleRowData);
            auto& rowItem = dialogRowData.back();
            const auto* rowItemMemory = findDataRowItem(rowItem.uniqueDataId(), dialogRowDataMemory);
            if(rowItemMemory)
                rowItem.dialogTreeNodeCollapsed(rowItemMemory->dialogTreeNodeCollapsed());
        }
        return dialogRowData;
    }

    void ProducerData::SetCaseStudyMode(bool newState)
    {
        gCaseStudyModeOn = newState;
    }
}
