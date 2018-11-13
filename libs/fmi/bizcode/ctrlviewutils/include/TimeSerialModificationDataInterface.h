#pragma once

#include "NFmiInfoData.h"
#include "NFmiTimeDescriptor.h"
#include "FmiSmartMetEditingMode.h"
#include "NFmiDataMatrix.h"
#include "boost/shared_ptr.hpp"

class NFmiFastQueryInfo;
class NFmiAreaMaskList;
class NFmiAnalyzeToolData;
class NFmiInfoOrganizer;
class NFmiMetTime;
class NFmiDrawParam;
class NFmiDataIdent;
class NFmiMetEditorOptionsData;
class NFmiEditorControlPointManager;
class NFmiParamBag;
class NFmiGrid;
class NFmiSmartToolInfo;
class NFmiGriddingHelperInterface;
class NFmiRect;
class NFmiPoint;
class NFmiDataLoadingInfo;
class NFmiHelpEditorSystem;
class NFmiHelpDataInfoSystem;
class NFmiQueryData;
class NFmiHPlaceDescriptor;
class NFmiParamDescriptor;
class NFmiModelDataBlender;
class NFmiProducerIdLister;
class NFmiMacroParamSystem;
class NFmiArea;
class MultiProcessClientData;

namespace CatLog
{
    enum class Severity;
    enum class Category;
}

// Joudun siirt�m��n kaikki SmartMetin datan editointi rutiinit erilleen pois NFmiEditMapGeneralDataDoc-luokasta.
// Jotta saan funktiot irti GenDoc-hirvi�st�, teen t�nne adapteri perusluokan, joka toteuttaa kaikki tarvittavat
// datan pyynt�kutsut virtuaalisesti. T�m� luokka on abstrakti perusluokka ja kaiki sen metodit ovat abstrakteja.
// T�st� on tarkoitus peria kaksi lapsiluokkaa:
// 1. TimeSerialModificationDataAdapter, joka on ns. konsoli testaukseen sopiva luokka joka takaa tarvittavat 
// datat ja toiminnat, joilla saan mahd. yksinkertaisesti kaiken editoinnin konsoli pohjaiseen testipenkkiin 
// (t�m� tapahtuu ModifyEditedDataExe.cpp -tiedostossa ModifyEditedDataExe-projektissa). 
// 2. TimeSerialModificationGenDocAdapter, joka ottaa sis��ns� oikean GenDoci SmartMet applikaatiolta ja toteuttaa
// kaiken toiminnan t�m�n oikean GenDoc-olion avustuksella.
class TimeSerialModificationDataInterface
{
public:
    virtual ~TimeSerialModificationDataInterface(void) = default;

    virtual bool CheckValidationFromSettings(void) = 0; // jos t�m� on true, p��tell��n validaatioasetuksista, ett� tehd��nk� validaatio vai ei. Jos t�m� on false, ei tehd� validaatioita
    virtual boost::shared_ptr<NFmiFastQueryInfo> EditedInfo(void) = 0;
    virtual boost::shared_ptr<NFmiAreaMaskList> ParamMaskList(void) = 0;
    virtual bool UseMasksInTimeSerialViews(void) = 0;
    virtual NFmiAnalyzeToolData& AnalyzeToolData(void) = 0;
    virtual NFmiInfoOrganizer* InfoOrganizer(void) = 0;
    virtual bool EditedDataNotInPreferredState(void) = 0;
    virtual void EditedDataNotInPreferredState(bool newValue) = 0;
    virtual int DoMessageBox(const std::string & theMessage, const std::string & theTitle, unsigned int theMessageBoxType) = 0;
    virtual void AllMapViewDescTopsTimeDirty(const NFmiMetTime &theTime) = 0;
    virtual void AreaViewDirty(unsigned int theDescTopIndex, bool areaViewDirty, bool clearCache, bool clearMacroParamDataCache) = 0;
    virtual void MapDirty(unsigned int theDescTopIndex, bool mapDirty, bool clearCache) = 0;
    virtual void WindTableSystemMustaUpdateTable(bool newState) = 0;
    virtual boost::shared_ptr<NFmiDrawParam> GetUsedDrawParam(const NFmiDataIdent &theDataIdent, NFmiInfoData::Type theDataType) = 0;
    virtual NFmiMetEditorOptionsData& MetEditorOptionsData(void) = 0;
    virtual boost::shared_ptr<NFmiEditorControlPointManager> CPManager(bool getOldSchoolCPManager = false) = 0;
    virtual int TimeEditSmootherValue(void) = 0;
    virtual CtrlViewUtils::FmiSmartMetEditingMode SmartMetEditingMode(void) = 0;
    virtual void SmartMetEditingMode(CtrlViewUtils::FmiSmartMetEditingMode newValue, bool modifySettings) = 0;
    virtual int FilteringParameterUsageState(void) = 0;
    virtual void FilteringParameterUsageState(int newValue) = 0;
    virtual boost::shared_ptr<NFmiDrawParam> ActiveDrawParam(unsigned int theDescTopIndex, int theRowIndex) = 0;
    virtual int ActiveViewRow(unsigned int theDescTopIndex) = 0;
    virtual NFmiParamBag& FilteringParamBag(void) = 0;
    virtual const NFmiTimeDescriptor& EditedDataTimeDescriptor(void) = 0;
    virtual const NFmiMetTime& ActiveViewTime(void) = 0;
    virtual boost::shared_ptr<NFmiTimeDescriptor> CreateDataFilteringTimeDescriptor(boost::shared_ptr<NFmiFastQueryInfo> &theEditedData, bool fPasteClipBoardData = false) = 0;
    virtual NFmiSmartToolInfo* SmartToolInfo(void) = 0;
    virtual std::string& SmartToolEditingErrorText(void) = 0;
    virtual NFmiGriddingHelperInterface* GetGriddingHelper(void) = 0;
    virtual void LastBrushedViewRow(int newRow) = 0;
    virtual bool UseMasksWithFilterTool(void) = 0;
    virtual int TestFilterUsedMask(void) = 0;
    virtual NFmiGrid* ClipBoardData(void) = 0;
    virtual int FilterFunction(void) = 0; // 0 = avg, 1 = max ja 2 = min filtterit
    virtual const NFmiMetTime& TimeFilterStartTime(void) = 0;
    virtual const NFmiMetTime& TimeFilterEndTime(void) = 0;
    virtual const NFmiRect& AreaFilterRange(int index) = 0;
    virtual bool UseTimeInterpolation(void) = 0;
    virtual const NFmiPoint& TimeFilterRangeStart(void) = 0;
    virtual const NFmiPoint& TimeFilterRangeEnd(void) = 0;
    virtual bool IsSmoothTimeShiftPossible(void) = 0;
    virtual int DataToDBCheckMethod(void) = 0;
    virtual NFmiDataLoadingInfo& GetUsedDataLoadingInfo(void) = 0;
    virtual NFmiHelpEditorSystem& HelpEditorSystem(void) = 0;
    virtual bool StoreData(const std::string& theFileName, boost::shared_ptr<NFmiFastQueryInfo> &theSmartInfo, bool askForSave) = 0;
    virtual bool StoreData(bool fDoSaveTmpRename, const std::string& theFileName, NFmiQueryData *theData) = 0;
    virtual NFmiHelpDataInfoSystem* HelpDataInfoSystem(void) = 0;
    virtual bool DataLoadingOK(bool noError) = 0;
    virtual void AddQueryData(NFmiQueryData* theData, const std::string& theDataFileName, const std::string& theDataFilePattern, NFmiInfoData::Type theType, const std::string& theNotificationStr, bool loadFromFileState = false) = 0;
    virtual bool WarnIfCantSaveWorkingFile(void) = 0;
    virtual NFmiHPlaceDescriptor* PossibleUsedDataLoadingGrid(void) = 0;
    virtual bool UseEditedDataParamDescriptor(void) = 0;
    virtual NFmiParamDescriptor& EditedDataParamDescriptor(void) = 0;
    virtual checkedVector<int>& DataLoadingProducerIndexVector(void) = 0;
    virtual void EditedDataNeedsToBeLoaded(bool newValue) = 0;
    virtual bool CheckEditedDataAfterAutoLoad(void) = 0;
    virtual void PutWarningFlagTimerOn(void) = 0;
    virtual NFmiModelDataBlender& ModelDataBlender(void) = 0;
    virtual NFmiProducerIdLister& ProducerIdLister(void) = 0;
    virtual void DoDataLoadingProblemsDlg(const std::string &theMessage) = 0;
    virtual void DataModificationInProgress(bool newValue) = 0;
    virtual bool DataModificationInProgress(void) = 0;
    virtual const std::string& GetCurrentSmartToolMacro(void) = 0;
    virtual NFmiMacroParamSystem& MacroParamSystem(void) = 0;
    virtual void SetLatestMacroParamErrorText(const std::string& theErrorText) = 0;
    virtual void SetMacroErrorText(const std::string &theErrorStr) = 0;
    virtual boost::shared_ptr<NFmiArea> MapHandlerArea(bool fGetZoomedArea) = 0;
    virtual FmiLanguage Language(void) = 0;
    virtual const NFmiRect& CPGridCropRect(void) = 0;
    virtual bool UseCPGridCrop(void) = 0;
    virtual const NFmiPoint& CPGridCropMargin(void) = 0;
    virtual MultiProcessClientData& GetMultiProcessClientData(void) = 0;
    virtual const std::string& GetSmartMetGuid(void) = 0;
    virtual bool UseMultiProcessCpCalc(void) = 0;
    virtual bool MakeSureToolMasterPoolIsRunning(void) = 0;
    virtual bool IsWorkingDataSaved() = 0;
    virtual void LogAndWarnUser(const std::string &theMessageStr, const std::string &theDialogTitleStr, CatLog::Severity severity, CatLog::Category category, bool justLog, bool addAbortOption = false) = 0;
};
