#pragma once

#include <stdafx.h>
#include <WinDef.h>

#include "NFmiTimeEditMode.h"
#include "NFmiDataMatrix.h"
#include "NFmiMetTime.h"
#include "NFmiParameterName.h"
#include "NFmiProducer.h"
#include "NFmiColor.h"
#include "NFmiInfoData.h"
#include "NFmiMetEditorTypes.h"
#include "FmiSmartMetEditingMode.h"
#include "FmiEditorModifyToolMode.h"
#include "catlog/catlog.h"
#include "ControlPointAcceleratorActions.h"

#include <list>
#include "boost/shared_ptr.hpp"

class NFmiColor;
class NFmiString;
class NFmiGrid;
class NFmiInfoOrganizer;
class NFmiDrawParamList;
class NFmiQueryData;
class NFmiDrawParam;
class NFmiMenuItem;
class NFmiMenuItemList;
class NFmiDataIdent;
class NFmiSmartInfo;
class NFmiFastQueryInfo;
class NFmiAreaMaskList;
class NFmiDataLoadingInfo;
class NFmiTimeDescriptor;
class NFmiHPlaceDescriptor;
class NFmiDataLoadingInfo;
class NFmiLocationSelectionTool;
class NFmiLevel;
class NFmiArea;
class NFmiEditMapDataListHandler; //laura lisäsi 30081999
class CDC; // toivottavasti vain väliaikaisesti/Marko
class CBitmap;
class NFmiMilliSecondTimer;
class NFmiDataModifier;
class NFmiStringList;
class NFmiProducerList;
class NFmiMetEditorModeDataWCTR;
class NFmiMetEditorOptionsData;
class NFmiMetEditorBaseMacro;
class NFmiEditorControlPointManager;
class NFmiProjectionCurvatureInfo;
class NFmiSmartToolInfo;
class NFmiPoint;
class NFmiRect;
class NFmiViewSettingMacro;
class NFmiPolyline;
class NFmiMapViewCache;
class NFmiMacroParamSystem;
class NFmiCrossSectionSystem;
class NFmiHelpDataInfoSystem;
class NFmiSynopStationPrioritySystem;
class NFmiSynopPlotSettings;
class NFmiObsComparisonInfo;
class NFmiMTATempSystem;
class NFmiTrajectorySystem;
class NFmiFileCleanerSystem;
class NFmiProducerSystem;
class NFmiHelpEditorSystem;
class NFmiVPlaceDescriptor;
class NFmiMapViewDescTop;
namespace CtrlViewUtils
{
    struct GraphicalInfo;
    struct ParamInfoStatus;
}
class NFmiConceptualModelData;
class NFmiQ2Client;
class NFmiSeaIcingWarningSystem;
class CWnd;
class NFmiMapViewTimeLabelInfo;
class NFmiWindTableSystem;
class CSize;
class NFmiAutoComplete;
class NFmiModelDataBlender;
class NFmiParamBag;
class NFmiAnalyzeToolData;
class NFmiDataQualityChecker;
class NFmiToolBox;
class NFmiIgnoreStationsData;
class NFmiMacroPathSettings;
class NFmiApplicationDataBase;
class NFmiCaseStudySystem;
class NFmiParamDescriptor;
class NFmiProducerIdLister;
class NFmiGenDocDataAdapter;
class NFmiBasicSmartMetConfigurations;
class NFmiCPManagerSet;
class NFmiApplicationWinRegistry;
class NFmiDataNotificationSettingsWinRegistry;
class MultiProcessClientData;
class NFmiMultiProcessPoolOptions;
class NFmiFixedDrawParamSystem;
class NFmiSatelliteImageCacheSystem;
class NFmiBetaProductionSystem;
class NFmiBetaProduct;
class NFmiLightWeightViewSettingMacro;
class NFmiEditDataUserList;
class TimeSerialModificationDataInterface;
class ToolboxViewsInterface;
class CtrlViewDocumentInterface;
class SmartMetDocumentInterface;
class Q2ServerInfo;
class NFmiMacroParamDataCache;

namespace Wms
{
    class WmsSupport;
}

namespace HakeMessage
{
    class Main;
}

namespace Imagine
{
	class NFmiGeoShape;
}

namespace AddParams
{
    class ParamAddingSystem;
}

namespace Warnings
{
    class CapDataSystem;
}

class GeneralDocImpl;

using LogAndWarnFunctionType = std::function<void(const std::string &, const std::string&, CatLog::Severity, CatLog::Category, bool)>;

class NFmiEditMapGeneralDataDoc
{
public:
    NFmiMacroParamDataCache& MacroParamDataCache();
    void InitGriddingProperties();
    bool MakeControlPointAcceleratorAction(ControlPointAcceleratorActions action, const std::string &updateMessage);
    int GetTimeRangeForWarningMessagesOnMapViewInMinutes();
    void UpdateRowInLockedDescTops(unsigned int theOrigDescTopIndex);
    Warnings::CapDataSystem& GetCapDataSystem();
    Q2ServerInfo& GetQ2ServerInfo();
    void UpdateViewForOffScreenDraw(unsigned int theMapViewDescTopIndex);
    void DoOffScreenDraw(unsigned int theMapViewDescTopIndex, CBitmap &theDrawedScreenBitmapOut);
    NFmiBasicSmartMetConfigurations& BasicSmartMetConfigurations();
    boost::shared_ptr<NFmiFastQueryInfo> GetMosTemperatureMinAndMaxData();
    void SetLastActiveDescTopAndViewRow(unsigned int theDescTopIndex, int theActiveRowIndex);
    bool LoadStaticHelpData(void);
    AddParams::ParamAddingSystem& ParamAddingSystem();
    void UpdateParamAddingSystem();
#ifndef DISABLE_CPPRESTSDK
    Wms::WmsSupport& WmsSupport();
#endif // DISABLE_CPPRESTSDK
    bool UseWmsMaps();
    void UseWmsMaps(bool newValue);
    boost::shared_ptr<NFmiFastQueryInfo> GetFavoriteSurfaceModelFractileData();
    boost::shared_ptr<NFmiFastQueryInfo> GetModelClimatologyData();
    bool IsWorkingDataSaved();
    SmartMetDocumentInterface& GetSmartMetDocumentInterface();
    ToolboxViewsInterface& GetToolboxViewsInterface();
    CtrlViewDocumentInterface& GetCtrlViewDocumentInterface();
    void MakeHTTPRequest(const std::string &theUrlStr, std::string &theResponseStrOut, bool fDoGet);
    void ApplyStartupViewMacro();
    void FillStartupViewMacro();
    void CheckForNewWarningMessageData();
    NFmiEditDataUserList& EditDataUserList();
    const std::vector<std::string>& CorruptedViewMacroFileList() const;
    const std::vector<std::string>& SmartToolFileNames(bool updateList);
    std::string GetRelativeViewMacroPath();
    std::vector<NFmiLightWeightViewSettingMacro>& ViewMacroDescriptionList();
    const std::vector<std::string>& ViewMacroFileNames(bool updateList);
    const NFmiMetTime& LastEditedDataSendTime();
    bool IsLastEditedDataSendHasComeBack();
    void ResetTimeFilterTimes();
    void SetAllViewIconsDynamically(void);
    void LoadViewMacroInfo(NFmiViewSettingMacro &theViewMacro, bool fTreatAsViewMacro);
    void FillViewMacroInfo(NFmiViewSettingMacro &theViewMacro, const std::string &theName, const std::string &theDescription);
    void SetCurrentGeneratedBetaProduct(const NFmiBetaProduct *theBetaProduct);
    const NFmiBetaProduct* GetCurrentGeneratedBetaProduct();
    void DoGenerateBetaProductsChecks();
    LogAndWarnFunctionType GetLogAndWarnFunction();
    NFmiBetaProductionSystem& BetaProductionSystem();
    bool LoadViewMacroFromBetaProduct(const std::string &theAbsoluteVieMacroPath, std::string &theErrorStringOut, bool justLogMessages);
    const std::string& RootViewMacroPath();
    bool BetaProductGenerationRunning();
    void BetaProductGenerationRunning(bool newValue);
    void OnAcceleratorSetHomeTime(int theMapViewDescTopIndex);
    void ReloadFailedSatelliteImages();
    NFmiSatelliteImageCacheSystem& SatelliteImageCacheSystem();
    int DoImageCacheUpdates(void);
    const NFmiPoint& PreciseTimeSerialLatlonPoint() const;
    void PreciseTimeSerialLatlonPoint(const NFmiPoint &newValue);
    bool IsPreciseTimeSerialLatlonPointUsed();
    void StoreBackUpViewMacro(bool fUseNormalBackup);
    void ApplyBackupViewMacro(bool fUseNormalBackup);
    void ReloadFixedDrawParams();
    NFmiFixedDrawParamSystem& FixedDrawParamSystem();
    bool ChangeParamSettingsToNextFixedDrawParam(unsigned int theDescTopIndex, int theMapRow, int theParamIndex, bool fNext, bool fUseCrossSectionParams);
    bool AllowRightClickDisplaySelection(void);
    void AllowRightClickDisplaySelection(bool newValue);
    NFmiMultiProcessPoolOptions& MultiProcessPoolOptions(void);
    bool MakeSureToolMasterPoolIsRunning(void);
    bool UseMultiProcessCpCalc(void);
    void UseMultiProcessCpCalc(bool newValue);
    const std::string& GetSmartMetGuid(void);
    MultiProcessClientData& GetMultiProcessClientData(void);
    void UpdateEnableDataChangesToWinReg(void);
    const NFmiMetTime& ActiveMapTime(void);
    unsigned int ActiveMapDescTopIndex(void);
    void ActiveMapDescTopIndex(unsigned int newValue);
    void StoreSettingsToWinRegistry(void);
    void PanMap(int theMapViewDescTopIndex, boost::shared_ptr<NFmiArea> &theMapArea, const NFmiPoint &theMousePoint, const NFmiPoint &theZoomDragUpPoint);
    void ZoomMapInOrOut(int theMapViewDescTopIndex, boost::shared_ptr<NFmiArea> &theMapArea, const NFmiPoint &theMousePoint, double theZoomFactor);
    void ZoomMapWithRelativeRect(int theMapViewDescTopIndex, boost::shared_ptr<NFmiArea> &theMapArea, const NFmiRect &theZoomedRect);
    NFmiApplicationWinRegistry& ApplicationWinRegistry();
	NFmiCPManagerSet& CPManagerSet(void);
	bool IsCPGridCropNotPlausible(void);
	boost::shared_ptr<NFmiArea> CPGridCropInnerLatlonArea(void);
	const NFmiPoint& CPGridCropMargin(void);
	const boost::shared_ptr<NFmiArea> CPGridCropLatlonArea(void);
	const NFmiRect& CPGridCropRect(void);
	bool UseCPGridCrop(void);
	void UseCPGridCrop(bool newValue);
	bool IsCPGridCropInAction(void);
	bool DrawSelectionOnThisView(void);
	void DrawSelectionOnThisView(bool newValue);
	size_t SelectedGridPointLimit(void);
	void SelectedGridPointLimit(size_t newValue);
	bool EditedPointsSelectionChanged(void);
	bool EditedPointsSelectionChanged(bool newValue);
	bool UseMultithreaddingWithModifyingFunctions(void);
	bool DataModificationInProgress(void);
	void DataModificationInProgress(bool newValue);
	void InitGenDocDataAdapter(NFmiEditMapGeneralDataDoc *theDoc);
    TimeSerialModificationDataInterface& GenDocDataAdapter(void);
	void DoDataLoadingProblemsDlg(const std::string &theMessage);
	NFmiProducerIdLister& ProducerIdLister(void);
	void PutWarningFlagTimerOn(void);
	checkedVector<int>& DataLoadingProducerIndexVector(void);
	NFmiParamDescriptor& EditedDataParamDescriptor(void);
	bool UseEditedDataParamDescriptor(void);
	NFmiHPlaceDescriptor* PossibleUsedDataLoadingGrid(void);
	bool WarnIfCantSaveWorkingFile(void);
	bool DataLoadingOK(bool noError);
    bool StoreData(bool fDoSaveTmpRename, const NFmiString& theFileName, NFmiQueryData *theData, bool writeLocalDiskThenCopyToDestination);
	bool StoreData(const std::string& theFileName, boost::shared_ptr<NFmiFastQueryInfo> &theSmartInfo, bool askForSave);
	bool IsSmoothTimeShiftPossible(void);
	const NFmiPoint& TimeFilterRangeStart(void);
	const NFmiPoint& TimeFilterRangeEnd(void);
	boost::shared_ptr<NFmiDrawParam> GetUsedDrawParam(const NFmiDataIdent &theDataIdent, NFmiInfoData::Type theDataType);
	void AllMapViewDescTopsTimeDirty(const NFmiMetTime &theTime);
	int DoMessageBox(const std::string & theMessage, const std::string &theTitle, unsigned int theMessageBoxType);
	NFmiDataLoadingInfo& GetUsedDataLoadingInfo(void);
	bool StoreCaseStudyMemory(void);
	void CaseStudyToNormalMode(void);
	NFmiCaseStudySystem& LoadedCaseStudySystem(void);
	bool CaseStudyModeOn(void);
	bool LoadCaseStudyData(const std::string &theCaseStudyMetaFile);
	NFmiCaseStudySystem& CaseStudySystem(void);
	void DoMacroDirectoriesSyncronization(void);
	void DoUnisonDirectorySync(const std::string &theRoot1, const std::string &theRoot2, bool preferRoot1, WORD theShowWindow, bool waitExecutionToStop, DWORD dwCreationFlags);
	NFmiApplicationDataBase& ApplicationDataBase(void);
	int RunningTimeInSeconds(void);
	bool EditedDataNotInPreferredState(void);
	void EditedDataNotInPreferredState(bool newValue);
	bool CheckEditedDataAfterAutoLoad(void);
	NFmiMilliSecondTimer& EditedDataNeedsToBeLoadedTimer(void);
	bool TryAutoStartUpLoad(void);
	bool EditedDataNeedsToBeLoaded(void);
	void EditedDataNeedsToBeLoaded(bool newValue);
	NFmiMacroPathSettings& MacroPathSettings(void);
	int CleanUnusedDataFromMemory(void);
	void OnButtonRedo(void); 
	void OnButtonUndo(void); 
	NFmiIgnoreStationsData& IgnoreStationsData(void);
	int SatelDataRefreshTimerInMinutes(void);
	void SatelDataRefreshTimerInMinutes(int newValue);
	int MachineThreadCount(void);
	void ReportProcessMemoryUsage(void);
	void SetMacroErrorText(const std::string &theErrorStr);
	void InvalidateMapView(bool bErase = true);
	void ForceDrawOverBitmapThings(unsigned int originalCallerDescTopIndex, bool doOriginalView, bool doAllOtherMapViews);
	void ActivateZoomDialog(int theWantedDescTopIndex);
	std::string GetToolTipString(unsigned int commandID, std::string &theMagickWord);
	void ActivateViewParamSelectorDlg(int theMapViewDescTopIndex);
	void UpdateTempView(void);
	void UpdateCrossSectionView(void);
	void DrawOverBitmapThings(NFmiToolBox * theGTB); // tämä on kirjastojen pilkkomiseen vaadittuja funktioita
	NFmiDataQualityChecker& DataQualityChecker(void);
	NFmiAnalyzeToolData& AnalyzeToolData(void);
	NFmiModelDataBlender& ModelDataBlender(void);
	bool ChangeTime(int theTypeOfChange, FmiDirection theDirection, int theViewType, unsigned long theMapViewIndex, double theAmountOfChange);
	std::string AutoGeneratedSoundingIndexBasePath(void);
	bool SetNearestBeforeModelOrigTimeRunoff(boost::shared_ptr<NFmiDrawParam> &theDrawParam ,const NFmiMetTime &theTime, unsigned int theDescTopIndex, int theViewRowIndex);
	void SetModelRunOffset(boost::shared_ptr<NFmiDrawParam> &theDrawParam, int theMoveByValue, unsigned int theDescTopIndex, int theViewRowIndex);
	const NFmiColor& GeneralColor(int theIndex);
	void SetMapViewCacheSize(double theNewSizeInMB);
	NFmiAutoComplete& AutoComplete(void);
	void UndoViewMacro(void);
	void RedoViewMacro(void);
	bool IsUndoableViewMacro(void);
	bool IsRedoableViewMacro(void);
	std::string GetModelOrigTimeString(boost::shared_ptr<NFmiDrawParam> &theDrawParam, int theIndex = 0);
	NFmiMetTime GetModelOrigTime(boost::shared_ptr<NFmiDrawParam> &theDrawParam, int theIndex = 0);
	void ChangeDriveLetterInSettings(const std::string &newLetter);
	bool UseTimeSerialAxisAutoAdjust(void);
	void UseTimeSerialAxisAutoAdjust(bool newValue);
	NFmiDataNotificationSettingsWinRegistry& DataNotificationSettings(void);
	bool DeveloperModePath(void);
	void DeveloperModePath(bool newValue);
    NFmiPoint GetPrintedMapAreaOnScreenSizeInPixels(unsigned int theDescTopIndex);
	bool CheckEditedDataForStartUpLoadErrors(int theMessageBoxButtunOptions);
	void CenterTimeControlView(unsigned int theDescTopIndex, const NFmiMetTime &theTime, bool fUpdateSelectedTime);
	const NFmiPoint& TimeSerialViewSizeInPixels(void) const;
	void TimeSerialViewSizeInPixels(const NFmiPoint &newValue);
    const std::string& WorkingDirectory(void) const;
    const std::string& ControlDirectory(void) const;
    const std::string& HelpDataPath(void) const;
	void MakeDrawedInfoVectorForMapView(checkedVector<boost::shared_ptr<NFmiFastQueryInfo> > &theInfoVector, boost::shared_ptr<NFmiDrawParam> &theDrawParam, const boost::shared_ptr<NFmiArea> &theArea);
	void ToggleTimeControlAnimationView(unsigned int theDescTopIndex);
    void CheckForNewConceptualModelData(void);
	bool DoAutoLoadDataAtStartUp(void) const;
	void DoAutoLoadDataAtStartUp(bool newValue);
	void CheckAnimationLockedModeTimeBags(unsigned int theDescTopIndex, bool ignoreSatelImages);
	bool ShowWaitCursorWhileDrawingView(void); // tarkasta CView-näyttöluokissa, onko mahdollisesti animaatiota käynnissä. Jos on, älä laita odota-cursoria näkyviin, koska se vilkuttaa ikävästi
	int DoAllAnimations(void);
	void CopyMapViewDescTopParams(unsigned int theDescTopIndex);
	void PasteMapViewDescTopParams(unsigned int theDescTopIndex);
	void CopyDrawParamsFromMapViewRow(unsigned int theDescTopIndex);
	void PasteDrawParamsToMapViewRow(unsigned int theDescTopIndex);
	void TimeSerialViewTimeBag(const NFmiTimeBag &theTimeBag);
	const NFmiTimeBag& TimeSerialViewTimeBag(void) const;
	void TimeControlViewTimes(unsigned int theDescTopIndex, const NFmiTimeDescriptor &newTimeDescriptor);
	const NFmiTimeDescriptor& TimeControlViewTimes(unsigned int theDescTopIndex);
	NFmiMapViewTimeLabelInfo& MapViewTimeLabelInfo();
	void ReloadAllDynamicHelpData();
	bool ShowToolTipTimeView(void);
	void ShowToolTipTimeView(bool newValue);
	bool ShowToolTipTempView(void);
	void ShowToolTipTempView(bool newValue);
	bool ShowToolTipTrajectoryView(void);
	void ShowToolTipTrajectoryView(bool newValue);
	void ToggleShowHelperDatasInTimeView(int theCommand);
	void MakeSwapBaseArea(unsigned int theDescTopIndex);
	void SwapArea(unsigned int theDescTopIndex);
	checkedVector<boost::shared_ptr<NFmiFastQueryInfo> > GetSortedSynopInfoVector(int theProducerId, int theProducerId2 = -1, int theProducerId3 = -1, int theProducerId4 = -1);
	void TransparencyContourDrawView(CWnd *theView);
	CWnd* TransparencyContourDrawView(void);
	int SoundingViewWindBarbSpaceOutFactor(void);
	void SoundingViewWindBarbSpaceOutFactor(int newValue);
	NFmiSeaIcingWarningSystem& SeaIcingWarningSystem(void);
	NFmiWindTableSystem& WindTableSystem(void);
	NFmiQ2Client& Q2Client(void);
	void GetDataFromQ2Server(const std::string &theURLStr,
							 const std::string &theParamsStr,
							 bool fUseBinaryData,
							 int theUsedCompression,
							 NFmiDataMatrix<float> &theDataMatrixOut,
							 std::string &theExtraInfoStrOut);
	bool UseQ2Server(void);
	NFmiConceptualModelData& ConceptualModelData(void);
	double DrawObjectScaleFactor(void);
	void DrawObjectScaleFactor(double newValue);
	void OnButtonDataArea(unsigned int theDescTopIndex);
	void OnAcceleratorToggleKeepMapRatio(void);
	void OnToggleOverMapBackForeGround(unsigned int theDescTopIndex);
	void OnAcceleratorMapRow(unsigned int theDescTopIndex, int theStartingRow);
	void OnAcceleratorBorrowParams(unsigned int theDescTopIndex, int row);
	void OnToggleLandBorderPenSize(unsigned int theDescTopIndex);
	void OnToggleLandBorderDrawColor(unsigned int theDescTopIndex);
	void OnShowProjectionLines(void);
	void OnShowMasksOnMap(unsigned int theDescTopIndex);
	void OnToggleShowNamesOnMap(unsigned int theDescTopIndex, bool fForward);
	void OnChangeMapType(unsigned int theDescTopIndex, bool fForward);
	void OnButtonRefresh(void);
	void OnShowTimeString(unsigned int theDescTopIndex);
	void OnHideParamWindow(unsigned int theDescTopIndex);
	void OnShowGridPoints(unsigned int theMapViewIndex);
	void OnToggleGridPointColor(unsigned int theDescTopIndex);
	void OnToggleGridPointSize(unsigned int theDescTopIndex);
	void OnEditSpaceOut(unsigned int theDescTopIndex);
    bool ChangeActiveMapViewParam(unsigned int theDescTopIndex, int theMapRow, int theParamIndex, bool fNext, bool fUseCrossSectionParams);
	boost::shared_ptr<NFmiFastQueryInfo> GetNearestSynopStationInfo(const NFmiLocation &theLocation, const NFmiMetTime &theTime, bool ignoreTime, checkedVector<boost::shared_ptr<NFmiFastQueryInfo> > *thePossibleInfoVector, double maxDistanceInMeters = 1000. * kFloatMissing);
	const NFmiPoint& OutOfEditedAreaTimeSerialPoint(void) const;
	void OutOfEditedAreaTimeSerialPoint(const NFmiPoint &newValue);
	void ResetOutOfEditedAreaTimeSerialPoint(void);
	const NFmiVPlaceDescriptor& SoundingPlotLevels(void);
	const NFmiPoint& StationDataGridSize(void);
	void StationDataGridSize(const NFmiPoint &newValue);

	CtrlViewUtils::GraphicalInfo& GetGraphicalInfo(unsigned int theDescTopIndex);
	bool SetMapViewGrid(unsigned int theDescTopIndex, const NFmiPoint &newValue);
	int DataToDBCheckMethod(void);
	void DataToDBCheckMethod(int newValue);
    std::vector<NFmiProducer>& ExtraSoundingProducerList(void);

	void BorrowParams(unsigned int theDescTopIndex, int row);
	void MustDrawTimeSerialView(bool newValue);
	bool MustDrawTimeSerialView(void);
	void MustDrawCrossSectionView(bool newValue);
	bool MustDrawCrossSectionView(void);
	void MustDrawTempView(bool newValue);
	bool MustDrawTempView(void);
	void CrossSectionDataViewFrame(const NFmiRect &theRect);
	const NFmiRect& CrossSectionDataViewFrame(void);
	NFmiPoint ActualCrossSectionBitmapSizeInPixels(void);
	const NFmiPoint& CrossSectionViewSizeInPixels(void);
	void CrossSectionViewSizeInPixels(const NFmiPoint& newSize);
	NFmiHelpEditorSystem& HelpEditorSystem(void);
	std::vector<NFmiMapViewDescTop*>& MapViewDescTopList(void);
	void TimeControlTimeStep(unsigned int theDescTopIndex, float newValue);
	float TimeControlTimeStep(unsigned int theDescTopIndex);
    void MapViewDirty(unsigned int theDescTopIndex, bool makeNewBackgroundBitmap, bool clearMapViewBitmapCacheRows, bool redrawMapView, bool clearMacroParamDataCache, bool clearEditedDataDependentCaches, bool updateMapViewDrawingLayers);
    void ForceStationViewRowUpdate(unsigned int theDescTopIndex, unsigned int theRealRowIndex);
    bool ActivateParamSelectionDlgAfterLeftDoubleClick(void);
	void ActivateParamSelectionDlgAfterLeftDoubleClick(bool newValue);
	NFmiMapViewDescTop* MapViewDescTop(unsigned int theIndex);
	void TakeDrawParamInUseEveryWhere(boost::shared_ptr<NFmiDrawParam> &theDrawParam, bool useInMap, bool useInTimeSerial, bool useInCrossSection, bool useWithViewMacros);
	void UpdateCrossSectionMacroParamDataSize(void);
	void ReportInfoOrganizerDataConsumption(void);
	NFmiProducerSystem& ProducerSystem(void);
	NFmiProducerSystem& ObsProducerSystem(void);
	NFmiProducerSystem& SatelImageProducerSystem(void);
#ifndef DISABLE_CPPRESTSDK
    HakeMessage::Main& WarningCenterSystem(void);
#endif // DISABLE_CPPRESTSDK
	void SetMacroParamDataGridSize(int xSize, int ySize);
    void LogMessage(const std::string& theMessage, CatLog::Severity severity, CatLog::Category category, bool flushLogger = false);
    void LogAndWarnUser(const std::string &theMessageStr, const std::string &theDialogTitleStr, CatLog::Severity severity, CatLog::Category category, bool justLog, bool addAbortOption = false, bool flushLogger = false);
	NFmiFileCleanerSystem& FileCleanerSystem(void);
	void CleanDataDirectories(void);
	void DoTEMPDataUpdate(const std::string &theTEMPCodeTextStr, std::string &theTEMPCodeCheckReportStr, bool fJustCheckTEMPCode = false);
	void ClearTEMPData(void);
	const std::string& LastTEMPDataStr(void);
	void SelectLocations(unsigned int theDescTopIndex, boost::shared_ptr<NFmiFastQueryInfo> &theInfo, const boost::shared_ptr<NFmiArea> &theMapArea, const NFmiPoint& theLatLon
						,const NFmiMetTime &theTime, int theSelectionCombineFunction, unsigned long theMask
						,bool fMakeMTAModeAdd
						,bool fDoOnlyMTAModeAdd);
	NFmiTrajectorySystem* TrajectorySystem(void);
	void RefreshApplicationViewsAndDialogs(const std::string &reasonForUpdate, bool fMakeAreaViewDirty = false, bool fClearCache = false, int theWantedMapViewDescTop = -1);
	FmiLanguage Language(void);
	bool SelectLanguage(void);
	bool CreateNewViewMacroDirectory(const std::string &thePath);
    void ChangeCurrentViewMacroDirectory(const std::string & theDirectoryName, bool fUseRootPathAsBase);
	void MakeClosingLogMessage(void);
	bool ShowMouseHelpCursorsOnMap(void);
	void ShowMouseHelpCursorsOnMap(bool newState);
	void LastSelectedSynopWmoId(int theWmoId);
	int LastSelectedSynopWmoId(void);
	void MouseOnMapView(bool newState);
	bool MouseOnMapView(void);
	const NFmiPoint& GetSynopHighlightLatlon(void);
	bool ShowSynopHighlight(void);
	void SetHighlightedSynopStation(const NFmiPoint &theLatlon, int theWmoId, bool fShowHighlight);
	bool SynopDataGridViewOn(void);
	void SynopDataGridViewOn(bool newState);
	bool TimeSerialDataViewOn(void);
	void TimeSerialDataViewOn(bool newValue);
	NFmiMTATempSystem& GetMTATempSystem(void);
	bool CreateCrossSectionViewPopup(int theRowIndex);
	NFmiDrawParamList* CrossSectionViewDrawParamList(int theRowIndex);
	NFmiObsComparisonInfo& ObsComparisonInfo(void);
	void SetApplicationTitle(const std::string &theTitle);
	std::string GetApplicationTitle(void);
	bool MakeMacroPathConfigurations(void);
	bool MapMouseDragPanMode(void);
	void MapMouseDragPanMode(bool newState);
	NFmiSynopPlotSettings* SynopPlotSettings(void);
	NFmiSynopStationPrioritySystem* SynopStationPrioritySystem(void);
	bool MoveActiveMapViewParamInDrawingOrderList(unsigned int theDescTopIndex, int theMapRow, bool fRaise, bool fUseCrossSectionParams);
	NFmiHelpDataInfoSystem* HelpDataInfoSystem(void);
	NFmiCrossSectionSystem* CrossSectionSystem(void);
	void SetLatestMacroParamErrorText(const std::string& theErrorText);
	const std::string& GetLatestMacroParamErrorText(void);
	void RemoveMacroParam(const std::string &theName); // poistaa halutun macroparamin dokumentista, tiedostoista ja näytöiltä
	void AddMacroParamToView(unsigned int theDescTopIndex, int theViewRow, const std::string &theName); // lisää halutun nimisen macroParamin halutun karttanäytön riville (1-5)
	void AddMacroParamToCrossSectionView(int theViewRow, const std::string &theName); // lisää halutun nimisen macroParamin halutun karttanäytön riville (1-5)
	NFmiMacroParamSystem& MacroParamSystem(void);
	// Nämä makro tekstin get ja set metodit on makroparam-näyttöä varten tehtyjä virityksiä
	void SetCurrentSmartToolMacro(const std::string& theMacroText);
	const std::string& GetCurrentSmartToolMacro(void);

	bool ChangeToEditorsWorkingDirectory(void); // asettaa editorin hakemiston ja aseman käyttöön
	bool SaveCurrentPath(void); // ottaa nykyisen työhakemiston ja aseman talteen
	bool LoadSavedPath(void); // lataa edellisessä talletetut polut ja asemat käyttöön
	void SnapShotData(boost::shared_ptr<NFmiFastQueryInfo> &theInfo, const NFmiDataIdent &theDataIdent, const std::string &theModificationText
				 , const NFmiMetTime &theStartTime, const NFmiMetTime &theEndTime);
	void SetMapArea(unsigned int theDescTopIndex, const boost::shared_ptr<NFmiArea> &newArea);
	void SetSelectedMapHandler(unsigned int theDescTopIndex, unsigned int newMapIndex);
	CtrlViewUtils::FmiSmartMetEditingMode SmartMetEditingMode(void);
	void SmartMetEditingMode(CtrlViewUtils::FmiSmartMetEditingMode newValue, bool modifySettings);
	bool UseDoAtSendCommandString(void);
	void UseDoAtSendCommandString(bool newValue);
	std::string DoAtSendCommandString(void);
	void DoAtSendCommandString(const std::string &newValue);
	bool RemoveViewMacro(const std::string& theMacroName);
	void RefreshViewMacroList(void);
	bool LoadViewMacro(const std::string &theName);
	void StoreViewMacro(const std::string &theName, const std::string &theDescription);

	NFmiSmartToolInfo* SmartToolInfo(void);
	boost::shared_ptr<NFmiDrawParam> GetDrawDifferenceDrawParam(void);
	boost::shared_ptr<NFmiDrawParam> GetSelectedGridPointDrawParam(void);

	bool DoSmartToolEditing(const std::string &theSmartToolText, const std::string &theRelativePathMacroName, bool fSelectedLocationsOnly);
	std::string& SmartToolEditingErrorText(void);
	bool ExecuteCommand(const NFmiMenuItem &theMenuItem, int theViewIndex, int theViewTypeId);
	const std::string& EditorVersionStr(void);
	bool IsOperationalModeOn(void);

	NFmiInfoOrganizer* InfoOrganizer(void);
    bool ShowHelperData1InTimeSerialView();
    bool ShowHelperData2InTimeSerialView();
    bool ShowHelperData3InTimeSerialView();
    bool ShowHelperData4InTimeSerialView();
    int BrushToolLimitSetting(void);
	void BrushToolLimitSetting(int newValue);
	float BrushToolLimitSettingValue(void);
	void BrushToolLimitSettingValue(float newValue);
	int TimeEditSmootherValue(void);
	void TimeEditSmootherValue(int newValue);
	int TimeEditSmootherMaxValue(void);
	void TimeEditSmootherMaxValue(int newValue);
	bool StoreAllCPDataToFiles(void);
	bool StoreDataToDataBase(const std::string &theForecasterId); // tallettaa datan tietokantaan
	void RemoveAllParamsFromMapViewRow(unsigned int theDescTopIndex, int theRowIndex);
	void RemoveAllParamsFromCrossSectionViewRow(int theRowIndex);

	void SetDataLoadingProducerIndexVector(const checkedVector<int>& theIndexVector);
	NFmiProjectionCurvatureInfo* ProjectionCurvatureInfo(void); // tämä tietää miten lat-lon-viivoja piirretään karttojen ylle

	void StoreSupplementaryData(void); // tallettaa mm. CP pisteet, muutoskäyrät jne.
	void UpdateModifiedDrawParamMarko(boost::shared_ptr<NFmiDrawParam> &theDrawParam); // tällä päivitetään muualla tehdyt muutokset drawparameihin myös modified listoihin (esim. aikasarja muutokset)
	bool IsToolMasterAvailable() const;
	void ToolMasterAvailable(bool newValue);
	bool TimeSerialViewDirty(void); // aikasarja ikkunat asetetaan samalla likaiseksi kuin karttanäyttökin (ainakin toistaiseksi)
	void TimeSerialViewDirty(bool newValue);
	boost::shared_ptr<NFmiEditorControlPointManager> CPManager(bool getOldSchoolCPManager = false);
	void DoAutoSaveData(void);
	NFmiMetEditorOptionsData& MetEditorOptionsData(void);
	// theModificationFactorCurvePoints koko tulee annetusta timebagista
	bool DoTimeSeriesValuesModifying(boost::shared_ptr<NFmiDrawParam> &theModifiedDrawParam, int theUsedMask, NFmiTimeDescriptor& theTimeDescriptor, checkedVector<double> &theModificationFactorCurvePoints, NFmiMetEditorTypes::FmiUsedSmartMetTool theEditorTool, bool fUseSetForDiscreteData, int theUnchangedValue = -1);
	bool DoCombineModelAndKlapse(void);
	CtrlViewUtils::FmiEditorModifyToolMode ModifyToolMode(void);
	void ModifyToolMode(CtrlViewUtils::FmiEditorModifyToolMode newState);
	NFmiMetEditorModeDataWCTR* EditorModeDataWCTR(void);
	const NFmiTimeDescriptor& EditedDataTimeDescriptor(void);
	const NFmiTimeBag& EditedDataTimeBag(void);
	bool LoadData(bool fRemoveThundersOnLoad);
	bool Init(const NFmiBasicSmartMetConfigurations &theBasicConfigurations, std::map<std::string, std::string> &mapViewsPositionMap, std::map<std::string, std::string> &otherViewsPositionPosMap);
	NFmiEditMapGeneralDataDoc(unsigned long thePopupMenuStartId);
	virtual ~NFmiEditMapGeneralDataDoc();
	NFmiEditMapDataListHandler* DataLists(void);

    // Tämä DrawParamList vs DrawParamListWithRealRowNumber -juttu johtuu niistä kammottavista 'suhteellisista' rivinumeroista, joista pitäisi päästä kokonaan eroon.
	NFmiDrawParamList* DrawParamList(int theDescTopIndex, int theIndex);
    NFmiDrawParamList* DrawParamListWithRealRowNumber(unsigned int theDescTopIndex, int theRealRowIndex);
    NFmiDrawParamList* TimeSerialViewDrawParamList(void);
	FmiTimeEditMode TimeEditMode(void)const;
	void TimeEditMode(const int& newMode);
	void AddQueryData(NFmiQueryData* theData, const std::string& theDataFileName, const std::string& theDataFilePattern, int theType, const std::string& theNotificationStr, bool loadFromFileState=false);

	bool OpenPopupMenu(void);
	void OpenPopupMenu(bool newState);
	bool CreateMaskSelectionPopup(void);
	bool CreateParamSelectionPopup(unsigned int theDescTopIndex, int theRowIndex);
	bool CreateViewParamsPopup(unsigned int theDescTopIndex, int theRowIndex, int index);
	bool CreateMaskSelectionPopup(int theRowIndex);
	bool CreateMaskParamsPopup(int theRowIndex, int index);

	bool CreateCPPopup();

	bool CreateTimeSerialDialogPopup(int index);
	bool CreateTimeSerialDialogOnViewPopup(int index);
	NFmiMenuItemList* PopupMenu(void);
	bool MakePopUpCommandUsingRowIndex(unsigned short theCommandID);
	bool SelectAllLocations(bool newState);
	NFmiLocationSelectionTool* LocationSelectionTool2(void);
	boost::shared_ptr<NFmiAreaMaskList> ParamMaskListMT(void);
	const std::string& FileDialogDirectoryMemory(void);
	bool MakeAndStoreFileDialogDirectoryMemory(const std::string& thePathAndFileName);

	const NFmiMetTime& CurrentTime(unsigned int theDescTopIndex);
	bool CurrentTime(unsigned int theDescTopIndex, const NFmiMetTime& newCurrentTime, bool fStayInsideAnimationTimes = false);

	bool IsMasksUsedInTimeSerialViews(void);
	void UseMasksInTimeSerialViews(bool newValue);
	bool SetDataToNextTime(unsigned int theDescTopIndex, bool fStayInsideAnimationTimes = false);
	bool SetDataToPreviousTime(unsigned int theDescTopIndex, bool fStayInsideAnimationTimes = false);
	bool StoreOptionsData(void);

	int ToolTipRowIndex(void);
	void ToolTipRowIndex(int newIndex);
	int ToolTipColumnIndex(void);
	void ToolTipColumnIndex(int newIndex);
	const NFmiPoint& ToolTipLatLonPoint(void) const;
	void ToolTipLatLonPoint(const NFmiPoint& theLatLon);
	void ToolTipTime(const NFmiMetTime& theTime);
	const NFmiMetTime& ToolTipTime(void);
	NFmiParamBag& AllStaticParams(void);
	bool SelectNewParamForSelectionTool(int theParamId);
	boost::shared_ptr<NFmiDrawParam> DefaultEditedDrawParam(void);
	void DoAutoZoom(unsigned int theDescTopIndex);
	NFmiPoint ActualMapBitmapSizeInPixels(unsigned int theDescTopIndex); // laskee näyttöruudukon yhden ruudun koon pikseleissä
	boost::shared_ptr<NFmiDrawParam> ActiveDrawParam(unsigned int theDescTopIndex, int theRowIndex);
	boost::shared_ptr<NFmiFastQueryInfo> EditedSmartInfo(void);
	bool DoAreaFiltering(bool fPasteClipBoardData = false);
	bool DoTimeFiltering(void);
	int FilterFunction(void);
	void FilterFunction(int newFilter);
	bool MakeGridFile(const NFmiString& theFileName);
	void UpdateEditedDataCopy(void);
	const NFmiMetTime& ActiveViewTime(void);
	void ActiveViewTime(const NFmiMetTime& theTime);
	int ActiveViewRow(unsigned int theDescTopIndex);
	void ActiveViewRow(unsigned int theDescTopIndex, int theActiveRowIndex);
	bool UseMasksWithFilterTool(void);
	void UseMasksWithFilterTool(bool newStatus);
	const NFmiRect& AreaFilterRangeLimits(void);
	void AreaFilterRangeLimits(const NFmiRect& theRect);
	const NFmiRect& AreaFilterRange(int index);
	void AreaFilterRange(int index, const NFmiRect& theRect);
	const NFmiPoint& TimeFilterLimits(void);
	void TimeFilterLimits(const NFmiPoint& thePoint);
	const NFmiPoint& TimeFilterRange(int index);
	void TimeFilterRange(int index, const NFmiPoint& thePoint, bool fRoundToNearestHour);
	void ResetFilters(void);
	int FilteringParameterUsageState(void);
	void FilteringParameterUsageState(int newState);
	NFmiParamBag& FilteringParamBag(void);
	void FilteringParamBag(const NFmiParamBag& newBag);
	void SetTestFilterMask(int newValue);
	int GetTestFilterMask(void);
	NFmiGrid* ClipBoardData(void);
	bool CopyActiveDataToClipBoard(void);
	bool PasteClipBoardToActiveData(void);
	bool MouseCapturedInTimeWindow(void);
	void MouseCapturedInTimeWindow(bool newValue);
	bool MouseCaptured(void);
	void MouseCaptured(bool newState);
	const NFmiRect& UpdateRect(void);
	void UpdateRect(const NFmiRect& theRect);
	const NFmiPoint& BrushSize(void);
	void BrushSize(const NFmiPoint& theSize);
	double BrushValue(void);
	void BrushValue(double newValue);
	double BrushSpecialParamValue(void);
	void BrushSpecialParamValue(double newValue);
	bool UseMaskWithBrush(void);
	void UseMaskWithBrush(bool newState);
	bool ViewBrushed(void);
	void ViewBrushed(bool newState);
	const NFmiRect& ActiveViewRect(void);
	void ActiveViewRect(const NFmiRect& theRect);
	bool LeftMouseButtonDown(void);
	void LeftMouseButtonDown(bool newState);
	bool RightMouseButtonDown(void);
	void RightMouseButtonDown(bool newState);
	bool MiddleMouseButtonDown(void);
	void MiddleMouseButtonDown(bool newState);
	const NFmiMetTime& LastBrushedViewTime(void);
	void LastBrushedViewTime(const NFmiMetTime& newTime);
	int LastBrushedViewRow(void);
	void LastBrushedViewRow(int newRow);
	bool HasActiveViewChanged(void);
	const NFmiMetTime& TimeFilterStartTime(void);
	const NFmiMetTime& TimeFilterEndTime(void);
	void SetTimeFilterStartTime(const NFmiMetTime& theTime);
	void SetTimeFilterEndTime(const NFmiMetTime& theTime);
	void SetActiveParamMissingValues(double theValue);
	void ChangeMapType(unsigned int theDescTopIndex, bool fForward);

	int FilterDialogUpdateStatus(void);
	void FilterDialogUpdateStatus(int newState);
	bool UseTimeInterpolation(void);
	void UseTimeInterpolation(bool newState);

	bool MakeDataValiditation(void);
	bool CheckAndValidateAfterModifications(NFmiMetEditorTypes::FmiUsedSmartMetTool theModifyingTool, bool fMakeDataSnapshotAction, unsigned int theLocationMask, FmiParameterName theParam = kFmiLastParameter, bool fPasteAction = false);
	bool ScrollViewRow(unsigned int theDescTopIndex, int theCount); // scrollaa näyttöriveja halutun määrän (negatiivinen skrollaa ylös ja positiivinen count alas)

	bool Printing(void);
	void Printing(bool newStatus);
	int ToggleShowTimeOnMapMode(unsigned int theDescTopIndex); // neljä tilaa: 0 = näytä aikakontrolliikkuna+teksti 1=vain aik.kont.ikkuna, 2=älä näytä kumpaakaan ja 3= näytä vain teksti

private:
    void SetGeneralDataDocInterfaceCallbacks();

	GeneralDocImpl *pimpl; // tähän laitetaan kaikki toiminnot dokumentista!!!

};
