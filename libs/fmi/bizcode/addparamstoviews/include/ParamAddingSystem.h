#pragma once

#include "SingleRowItem.h"
#include "boost/shared_ptr.hpp"
#include <vector>

class NFmiHelpDataInfoSystem;
class NFmiInfoOrganizer;
class NFmiProducerSystem;

namespace AddParams
{
    class CategoryData;

    class ParamAddingSystem
    {
        std::vector<std::unique_ptr<CategoryData>> categoryDataVector_;

        // We don't want to do updateModelData everytime when SmartMet reads new queryData from file. 
        // So when any data is read in GeneralDoc following happens:
        // 1. Checks if update is pending andif it is, do nothing.
        // 2. If it's not, startup updateTimer (in CMainFrm) with updateWaitTimeoutInSeconds_ and set on updatePending_ flag.
        // 3. When update timer starts, call updateModelData, which also sets updatePending_ flag off.
        int updateWaitTimeoutInSeconds_;
        bool updatePending_;

        // T‰h‰n vektoriin talletetaan lista ParamAdding-olioita, joiden avulla on tarkoitus t‰ytt‰‰ ParamAdding-dialogin Grid Control.
        std::vector<SingleRowItem> dialogRowData_;
        // T‰ss‰ on grid-controlliin laitettavan puurakenteen syvyys eli category (taso) = 1, producer = 2 ja fileData = 3, param = 4 ja level = 5
        std::vector<unsigned char> dialogTreePatternArray_;
        // If data has been updated, this flag is set. Calling updateDialogData method will set flag off.
        bool dialogDataNeedsUpdate_;

        // These are general helper data sources that are given from GeneralDataDoc
        NFmiProducerSystem *modelProducerSystem_;
        NFmiInfoOrganizer *infoOrganizer_;
        NFmiHelpDataInfoSystem *helpDataInfoSystem_;

        // Here is information about last activated view and its row. these are
        // used to insert selected parameters to right places.
        unsigned int itsLastAcivatedDescTopIndex;
        int itsLastActivatedRowIndex;

    public:
        ParamAddingSystem();
        ~ParamAddingSystem();
        void initialize(NFmiProducerSystem &modelProducerSystem, NFmiInfoOrganizer &infoOrganizer, NFmiHelpDataInfoSystem &helpDataInfoSystem);

        void updateModelData();
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
    private:
        void addNewCategoryData(const std::string &categoryName, NFmiProducerSystem &modelProducerSystem, NFmiInfoOrganizer &infoOrganizer, NFmiHelpDataInfoSystem &helpDataInfoSystem);
        void updateDialogRowData();
        void updateDialogTreePatternData();
    };
}
