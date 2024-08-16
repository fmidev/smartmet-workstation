#pragma once

#include "SingleRowItem.h"
#include "boost/shared_ptr.hpp"
#include <vector>
#include <functional>

class NFmiHelpDataInfoSystem;
class NFmiInfoOrganizer;
class NFmiProducerSystem;
class NFmiMacroParamSystem;
class WmsSupportInterface;
class NFmiLevelBag;
class NFmiProducer;

namespace AddParams
{   
    class CategoryData;

    class ParameterSelectionSystem
    {
        std::vector<std::unique_ptr<CategoryData>> categoryDataVector_;

        // We don't want to do updateModelData every time when SmartMet reads new queryData from file. 
        // So when any data is read in GeneralDoc following happens:
        // 1. Checks if update is pending and if it is, do nothing.
        // 2. If it's not, startup updateTimer (in CMainFrm) with updateWaitTimeoutInSeconds_ and set on updatePending_ flag.
        // 3. When update timer starts, call updateModelData, which also sets updatePending_ flag off.
        int updateWaitTimeoutInSeconds_;
        bool updatePending_;

		//Static data will be added only once
		bool staticDataAdded_;

        // List of ParameterSelection-objects (SingleRowItems), which are used to fill ParameterSelection-dialogs Grid Control (tree structure).
        std::vector<SingleRowItem> dialogRowData_;
        // TreeDepth for grid-control is saved here. Uses either treeDepth or category info (category = 1, producer = 2, fileData = 3, param = 4 and level = 5...)
        std::vector<unsigned char> dialogTreePatternArray_;
        // If data has been updated and dialogRowData_ needs update, this flag is set. Calling updateDialogData method will set flag off.
        bool dialogDataNeedsUpdate_ = true;
        // If basic data structures need update (any of query, image, wms, macroParam), this is set on
        bool dataNeedsUpdate_ = true;
        // All four dirty flags for different sub-data sections (queryData, imageData, wmsData and macroParamData)
        bool queryDataNeedsUpdate_ = true;
        bool imageDataNeedsUpdate_ = true;
        bool wmsDataNeedsUpdate_ = true;
        bool macroParamDataNeedsUpdate_ = true;

        // These are general helper data sources that are given from GeneralDataDoc
        NFmiProducerSystem *modelProducerSystem_;
        NFmiProducerSystem *obsProducerSystem_;
        NFmiProducerSystem *satelImageProducerSystem_;
        NFmiInfoOrganizer *infoOrganizer_;
        NFmiHelpDataInfoSystem *helpDataInfoSystem_;

        // Here is information about last activated view and its row. these are
        // used to insert selected parameters to right places.
        // 0 = main map, 1 = map-view-2, 2 = map-view-3
        // 98 = cross-section-view, 99 = time-serial-view
        unsigned int itsLastActivatedDesktopIndex;
        // The view row that has been last clicked with mouse.
        // This index starts from 1 and its absolute
        // Map-view's (1-3) have common row-index
        int itsLastActivatedMapRowIndex = 1;
        // Cross-section view have separate active row index
        int itsLastActivatedCrossSectionRowIndex = 1;
        // time-serial view have separate active row index
        int itsLastActivatedTimeSerialRowIndex = 1;

        // Help data's producer id's
        std::vector<int> helpDataIDs_;
        std::vector<SingleRowItem> otherHelpData_;
        std::vector<std::string> customCategories_;

        std::function<std::shared_ptr<NFmiMacroParamSystem>()> getMacroParamSystemCallback_;
        using GetWmsCallbackType = std::function<std::shared_ptr<WmsSupportInterface>()>;
        GetWmsCallbackType getWmsCallback_;
        const NFmiLevelBag *soundingLevels_;

		const std::string OperationalDataStr = "Operational data",
			ModellDataStr = "Model data",
			ObservationDataStr = "Observation data",
			SatelliteImagesStr = "Satellite images",
			MacroParametersStr = "Macro Parameters", 
			WmsStr = "WMS",
			HelpDataStr = "Help data";

    public:
        ParameterSelectionSystem();
        ~ParameterSelectionSystem();
        void initialize(NFmiProducerSystem &modelProducerSystem, NFmiProducerSystem &obsProducerSystem, NFmiProducerSystem &satelImageProducerSystem,
            NFmiInfoOrganizer &infoOrganizer, NFmiHelpDataInfoSystem &helpDataInfoSystem, const std::vector<int> &idVector, const std::vector<std::string> &customCategories);
        void reInitialize(NFmiProducerSystem& modelProducerSystem, NFmiProducerSystem& obsProducerSystem,
            NFmiProducerSystem& satelImageProducerSystem, NFmiHelpDataInfoSystem& helpDataInfoSystem);
        void addHelpData(const NFmiProducer &producer, const std::string &menuString, NFmiInfoData::Type dataType, const std::string &displayName = std::string());
		void updateData();
        void updateData(const std::string &catName, NFmiProducerSystem &producerSystem, NFmiInfoData::Type dataCategory, bool customCategory = false);
        int updateWaitTimeoutInSeconds() const { return updateWaitTimeoutInSeconds_; }
        bool updatePending() const { return updatePending_; }
        void updatePending(bool newValue) { updatePending_ = newValue; }
        bool dialogDataNeedsUpdate() const { return dialogDataNeedsUpdate_; }
		void dialogDataNeedsUpdate(bool value) { dialogDataNeedsUpdate_ = value; }
        bool dataNeedsUpdate() const { return dataNeedsUpdate_; }
        void dataNeedsUpdate(bool value);
        bool queryDataNeedsUpdate() const { return queryDataNeedsUpdate_; }
        void queryDataNeedsUpdate(bool value);
        bool imageDataNeedsUpdate() const { return imageDataNeedsUpdate_; }
        void imageDataNeedsUpdate(bool value);
        bool wmsDataNeedsUpdate() const { return wmsDataNeedsUpdate_; }
        void wmsDataNeedsUpdate(bool value);
        bool macroParamDataNeedsUpdate() const { return macroParamDataNeedsUpdate_; }
        void macroParamDataNeedsUpdate(bool value);
        void setMainDataFlagDirtyIfSubDataIsSet(bool value);
        void setDialogDataFlagDirtyIfSubDataNeedsUpdate(bool value);
        void updateDialogData();
        std::vector<SingleRowItem>& dialogRowData();
        const std::vector<SingleRowItem>& dialogRowData() const;
        const std::vector<unsigned char>& dialogTreePatternArray() const;
        void SetLastActiveIndexes(unsigned int desktopIndex, int rowIndex);
        unsigned int LastActivatedDesktopIndex() const { return itsLastActivatedDesktopIndex; }
        int LastActivatedRowIndex() const;
        int GetLastActivatedRowIndexFromWantedDesktop(unsigned int desktopIndex) const;
		void setMacroParamSystemCallback(std::function< std::shared_ptr<NFmiMacroParamSystem> ()> macroParamSystemCallback) { getMacroParamSystemCallback_ = macroParamSystemCallback; }
		void setWmsCallback(GetWmsCallbackType wmsCallBack) { getWmsCallback_ = wmsCallBack; }
		void setSoundingLevels(const NFmiLevelBag& soundingLevels) { soundingLevels_ = &soundingLevels; }
        void searchItemsThatMatchToSearchWords(const std::string &words); 

	private:
        bool addNewCategoryData(const std::string &categoryName, NFmiProducerSystem &producerSystem, NFmiInfoOrganizer &infoOrganizer, NFmiHelpDataInfoSystem &helpDataInfoSystem, NFmiInfoData::Type dataCategory, bool customCategory = false);
        void updateDialogRowData();
		void otherHelpDataTodialog();
		void updateDialogTreePatternData();
        void updateOperationalData(const std::string &categoryName, NFmiInfoData::Type dataCategory);
        void updateMacroParamData(const std::string &categoryName, NFmiInfoData::Type dataCategory);
        void updateCustomCategories();
		void updateWmsData(const std::string &categoryName, NFmiInfoData::Type dataCategory);
        void updateHelpData();
        void updateImageData();
        void updateQueryData();
        bool hasLeafNodeAsAChild(int index, std::vector<SingleRowItem> &resultRowData);
        void removeNodesThatDontHaveLeafs(std::vector<SingleRowItem> &resultRowData);
		void trimDialogRowDataDependingOnActiveView();
		std::vector<SingleRowItem> crossSectionData();
		std::vector<SingleRowItem> timeSeriesData();
        bool hasActualGridData(const SingleRowItem& row);
        bool isObservationsData(const SingleRowItem& row, int index);
		std::vector<SingleRowItem> addSubmenu(const SingleRowItem& row, int index);
		std::vector<SingleRowItem> addAllChildNodes(const SingleRowItem& row, int index);
        void clearData();
        bool isMapViewCase() const;
        std::string MakeBoolLogStringUpdate() const;
        void MakeUpdateLogging(const std::string& funcName, const std::string& stepName, const std::string& categoryName, NFmiInfoData::Type dataCategory = NFmiInfoData::kNoDataType, std::string extraInfo = "") const;
    };
}
