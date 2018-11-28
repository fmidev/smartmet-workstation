#pragma once

#include "SingleRowItem.h"
#include "boost/shared_ptr.hpp"
#include "NFmiProducer.h"
#include <vector>
#include <functional>

class NFmiHelpDataInfoSystem;
class NFmiInfoOrganizer;
class NFmiProducerSystem;
class NFmiMacroParamSystem;

namespace AddParams
{
    class CategoryData;

    class ParamAddingSystem
    {
        std::vector<std::unique_ptr<CategoryData>> categoryDataVector_;

        // We don't want to do updateModelData every time when SmartMet reads new queryData from file. 
        // So when any data is read in GeneralDoc following happens:
        // 1. Checks if update is pending and if it is, do nothing.
        // 2. If it's not, startup updateTimer (in CMainFrm) with updateWaitTimeoutInSeconds_ and set on updatePending_ flag.
        // 3. When update timer starts, call updateModelData, which also sets updatePending_ flag off.
        int updateWaitTimeoutInSeconds_;
        bool updatePending_;

        // List of ParamAdding-objects (SingleRowItems), which are used to fill ParamAdding-dialogs Grid Control (tree structure).
        std::vector<SingleRowItem> dialogRowData_;
        // TreeDepth for grid-control is saved here. Uses either treeDepth or category info (category = 1, producer = 2, fileData = 3, param = 4 and level = 5...)
        std::vector<unsigned char> dialogTreePatternArray_;
        // If data has been updated, this flag is set. Calling updateDialogData method will set flag off.
        bool dialogDataNeedsUpdate_;

        // These are general helper data sources that are given from GeneralDataDoc
        NFmiProducerSystem *modelProducerSystem_;
        NFmiProducerSystem *obsProducerSystem_;
        NFmiProducerSystem *satelImageProducerSystem_;
        NFmiInfoOrganizer *infoOrganizer_;
        NFmiHelpDataInfoSystem *helpDataInfoSystem_;

        // Here is information about last activated view and its row. these are
        // used to insert selected parameters to right places.
        // 0 = main map, 1 = map-view-2, 2 = map-view-3
        // In future these will be added: 98 = cross-section-view, 99 = time-serial-view
        unsigned int itsLastAcivatedDescTopIndex;
        // The view row that has been last clicked with mouse.
        // This index starts from 1 !!
        int itsLastActivatedRowIndex;

        // Help data's producer id's
        std::vector<int> helpDataIDs_;
        std::vector<SingleRowItem> otherHelpData_;
        std::vector<std::string> customCategories_;

        std::function<NFmiMacroParamSystem&()> getMacroParamSystemCallback_;

    public:
        ParamAddingSystem();
        ~ParamAddingSystem();
        void initialize(NFmiProducerSystem &modelProducerSystem, NFmiProducerSystem &obsProducerSystem, NFmiProducerSystem &satelImageProducerSystem,
            NFmiInfoOrganizer &infoOrganizer, NFmiHelpDataInfoSystem &helpDataInfoSystem, std::vector<int> idVector, std::vector<std::string> customCategories);
        void addHelpData(NFmiProducer &producer, const std::string &menuString, NFmiInfoData::Type dataType);
        void addHelpData(NFmiProducer &producer, const std::string &menuString, NFmiInfoData::Type dataType, std::string &displayName);
        void updateData();
        void updateData(std::string catName, NFmiProducerSystem &producerSystem, NFmiInfoData::Type dataCategory, bool customCategory = false);
        int updateWaitTimeoutInSeconds() const { return updateWaitTimeoutInSeconds_; }
        bool updatePending() const { return updatePending_; }
        void updatePending(bool newValue) { updatePending_ = newValue; }
        bool dialogDataNeedsUpdate() const { return dialogDataNeedsUpdate_; }
        void updateDialogData();
        std::vector<SingleRowItem>& dialogRowData();
        const std::vector<SingleRowItem>& dialogRowData() const;
        const std::vector<unsigned char>& dialogTreePatternArray() const;
        unsigned int LastAcivatedDescTopIndex() const { return itsLastAcivatedDescTopIndex; }
        void LastAcivatedDescTopIndex(unsigned int newValue) { itsLastAcivatedDescTopIndex = newValue; }
        int LastActivatedRowIndex() const { return itsLastActivatedRowIndex; }
        void LastActivatedRowIndex(int newValue) { itsLastActivatedRowIndex = newValue; }
        void setMacroParamSystemCallback(std::function<NFmiMacroParamSystem&()> macroParamSystemCallback) { getMacroParamSystemCallback_ = macroParamSystemCallback; }

    private:
        void addNewCategoryData(const std::string &categoryName, NFmiProducerSystem &producerSystem, NFmiInfoOrganizer &infoOrganizer, NFmiHelpDataInfoSystem &helpDataInfoSystem, NFmiInfoData::Type dataCategory, bool customCategory = false);
        void updateDialogRowData();
        void updateDialogTreePatternData();
        void updateMacroParamData(std::string catName, NFmiInfoData::Type dataCategory);
        void updateCustomCategories();
        void updateCustomCategoryData(std::string catName, NFmiProducerSystem &producerSystem, NFmiInfoData::Type dataCategory);
    };
}
