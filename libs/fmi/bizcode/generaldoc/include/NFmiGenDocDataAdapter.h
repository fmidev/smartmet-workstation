#pragma once

#include "TimeSerialModificationDataInterface.h"

class NFmiEditMapGeneralDataDoc;

// Tämä luokka toteuttaa uuden editoidun datan muokkaus rajapinnan joka on tehty
// ModifyEditedData-kirjastoon. Siellä on määritelty abstrakti TimeSerialModificationDataAdapterBase
// -luokka ja tämä on sen toteutus kun käytössä on NFmiEditMapGeneralDataDoc -olio.
class NFmiGenDocDataAdapter : public TimeSerialModificationDataInterface
{
public:

	NFmiGenDocDataAdapter(NFmiEditMapGeneralDataDoc *theDoc);
	~NFmiGenDocDataAdapter(void);

	bool CheckValidationFromSettings(void) override;
	boost::shared_ptr<NFmiFastQueryInfo> EditedInfo(void) override;
	boost::shared_ptr<NFmiAreaMaskList> ParamMaskList(void) override;
	bool UseMasksInTimeSerialViews(void) override;
	NFmiAnalyzeToolData& AnalyzeToolData(void) override;
	NFmiInfoOrganizer* InfoOrganizer(void) override;
	bool IsEditedDataInReadOnlyMode() override;
	int DoMessageBox(const std::string & theMessage, const std::string &theTitle, unsigned int theMessageBoxType) override;
    void MapViewDirty(unsigned int theDescTopIndex, bool makeNewBackgroundBitmap, bool clearMapViewBitmapCacheRows, bool redrawMapView, bool clearMacroParamDataCache, bool clearEditedDataDependentCaches, bool updateMapViewDrawingLayers) override;
    void WindTableSystemMustaUpdateTable(bool newState) override;
	boost::shared_ptr<NFmiDrawParam> GetUsedDrawParamForEditedData(const NFmiDataIdent &theDataIdent) override;
	NFmiMetEditorOptionsData& MetEditorOptionsData(void) override;
	boost::shared_ptr<NFmiEditorControlPointManager> CPManager(bool getOldSchoolCPManager = false) override;
	CtrlViewUtils::FmiSmartMetEditingMode SmartMetEditingMode(void) override;
	void SmartMetEditingMode(CtrlViewUtils::FmiSmartMetEditingMode newValue, bool modifySettings) override;
	int FilteringParameterUsageState(void) override;
	void FilteringParameterUsageState(int newValue) override;
	boost::shared_ptr<NFmiDrawParam> ActiveDrawParamFromActiveRow(unsigned int theDescTopIndex) override;
	int AbsoluteActiveViewRow(unsigned int theDescTopIndex) override;
	NFmiParamBag& FilteringParamBag(void) override;
	const NFmiTimeDescriptor& EditedDataTimeDescriptor(void) override;
	const NFmiMetTime& ActiveViewTime(void) override;
	boost::shared_ptr<NFmiTimeDescriptor> CreateDataFilteringTimeDescriptor(boost::shared_ptr<NFmiFastQueryInfo> &theEditedData) override;
	NFmiSmartToolInfo* SmartToolInfo(void) override;
	std::string& SmartToolEditingErrorText(void) override;
    NFmiGriddingHelperInterface* GetGriddingHelper(void) override;
	void LastBrushedViewRealRowIndex(int newRealRowIndex) override;
	bool UseMasksWithFilterTool(void) override;
	int TestFilterUsedMask(void) override;
	NFmiGrid* ClipBoardData(void) override;
	int FilterFunction(void) override; // 0 = avg, 1 = max ja 2 = min filtterit
	const NFmiMetTime& TimeFilterStartTime(void) override;
	const NFmiMetTime& TimeFilterEndTime(void) override;
	const NFmiRect& AreaFilterRange(int index) override; // 1 = alku laatikko ja 2 on loppu laatikko
	bool UseTimeInterpolation(void) override;
	const NFmiPoint& TimeFilterRangeStart(void) override;
	const NFmiPoint& TimeFilterRangeEnd(void) override;
	bool IsSmoothTimeShiftPossible(void) override;
	int DataToDBCheckMethod(void) override; // 1=datavalidation, 2= niin kuin asetuksissa sanotaan
	NFmiDataLoadingInfo& GetUsedDataLoadingInfo(void) override;
	NFmiHelpEditorSystem& HelpEditorSystem(void) override;
	bool StoreData(const std::string& theFileName, boost::shared_ptr<NFmiFastQueryInfo> &theSmartInfo, bool askForSave) override;
	bool StoreData(bool fDoSaveTmpRename, const std::string& theFileName, NFmiQueryData *theData) override;
	NFmiHelpDataInfoSystem* HelpDataInfoSystem(void) override;
	bool DataLoadingOK(bool noError) override;
	void AddQueryData(NFmiQueryData* theData, const std::string& theDataFileName, const std::string& theDataFilePattern, NFmiInfoData::Type theType, const std::string& theNotificationStr, bool loadFromFileState, bool& dataWasDeleted) override;
	bool WarnIfCantSaveWorkingFile(void) override;
	NFmiHPlaceDescriptor* PossibleUsedDataLoadingGrid(void) override;
	bool UseEditedDataParamDescriptor(void) override;
	NFmiParamDescriptor& EditedDataParamDescriptor(void) override;
	std::vector<int>& DataLoadingProducerIndexVector(void) override;
	void EditedDataNeedsToBeLoaded(bool newValue) override;
	void PutWarningFlagTimerOn(void) override;
	NFmiModelDataBlender& ModelDataBlender(void) override;
	NFmiProducerIdLister& ProducerIdLister(void) override;
	void DoDataLoadingProblemsDlg(const std::string &theMessage) override;
	void DataModificationInProgress(bool newValue) override;
	bool DataModificationInProgress(void) override;
	const std::string& GetCurrentSmartToolMacro(void) override;
	std::shared_ptr<NFmiMacroParamSystem> MacroParamSystem(void) override;
	void SetLatestMacroParamErrorText(const std::string& theErrorText) override;
	void SetMacroErrorText(const std::string &theErrorStr) override;
	boost::shared_ptr<NFmiArea> MapHandlerArea(bool fGetZoomedArea) override;
	FmiLanguage Language(void) override;
	const NFmiRect& CPGridCropRect(void) override;
	bool UseCPGridCrop(void) override;
	const NFmiPoint& CPGridCropMargin(void) override;
    MultiProcessClientData& GetMultiProcessClientData(void) override;
    const std::string& GetSmartMetGuid(void) override;
    bool UseMultiProcessCpCalc(void) override;
    bool MakeSureToolMasterPoolIsRunning(void) override;
    bool IsWorkingDataSaved() override;
    void LogAndWarnUser(const std::string &theMessageStr, const std::string &theDialogTitleStr, CatLog::Severity severity, CatLog::Category category, bool justLog, bool addAbortOption = false) override;
	boost::shared_ptr<NFmiArea> GetUsedMapViewArea(int theMapViewDescTopIndex) override;

protected:
	NFmiEditMapGeneralDataDoc *itsDoc; // ei omista, ei tuhoa
};
