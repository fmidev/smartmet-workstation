#pragma once

// SmartMetDoc.h : interface of the CSmartMetDoc class
//

#include <vector>
#include "NFmiInfoData.h"
#include "boost/shared_ptr.hpp"
#include "SmartMetViewId.h"
#include "ControlPointAcceleratorActions.h"

class NFmiRect;
class NFmiEditMapGeneralDataDoc;
class NFmiFastQueryInfo;
class NFmiString;
class CTimeEditValuesDlg;
class CFmiLocationSelectionToolDlg2;
class CZoomDlg;
class CFmiFilterDataDlg;
class CFmiBrushToolDlg;
class CFmiSmartToolDlg;
class CFmiSmarttoolsTabControlDlg;
class NFmiQueryData;
class CFmiTempDlg;
class CFmiShortCutsDlg2;
class CFmiViewMacroDlg;
class CFmiCrossSectionDlg;
class CFmiSynopPlotSettingsDlg;
class CFmiSynopDataGridViewDlg;
class CFmiTrajectoryDlg;
class CFmiWarningCenterDlg;
class CFmiExtraMapViewDlg;
class CFmiWindTableDlg;
class NFmiViewSettingMacro;
class CFmiLocationFinderDlg;
class CFmiDataQualityCheckerDialog;
class CFmiIgnoreStationsDlg;
class CFmiCaseStudyDlg;
class NFmiMetTime;
class CFmiBetaProductTabControlDialog;
class CFmiLogViewer;
class CFmiParameterSelectionDlg;
class SmartMetDocumentInterface;
class CFmiGriddingOptionsDlg;
class NFmiApplicationWinRegistry;
class CFmiSoundingDataServerConfigurationsDlg;
class CFmiVisualizationSettings;
enum class BetaProductViewIndex;

class CSmartMetDoc : public CDocument
{
protected: // create from serialization only
	CSmartMetDoc();
	DECLARE_DYNCREATE(CSmartMetDoc)

// Operations
public:
	BOOL StoreData(bool newFile = false, bool askForSave=true);
	BOOL StoreDataBaseDataMarko(boost::shared_ptr<NFmiFastQueryInfo> &smart);
	BOOL PrepareForLoadingDataFromFile(void);
	BOOL CheckEditedDataAndStoreIfNeeded(void);
	void LoadDataFromFileAndAdd(const std::string &theFileName, NFmiInfoData::Type theDatatype, bool useMemoryMapping);
	void LoadDataFromFilesAndAdd(std::vector<std::string> &theFileList);
	void ActivateZoomDialog(int theWantedDescTopIndex);
	void LoadDataOnStartUp(void);
	void PostMessageToDialog(SmartMetViewId theDestDlg, UINT theMessage);
	void DoOnClose(void);
    static std::map<std::string, std::string> MakeMapWindowPosMap(void);
    static std::map<std::string, std::string> MakeOtherWindowPosMap(void);
    void SaveViewPositionsToRegistry(void);
    void DoOffScreenDraw(BetaProductViewIndex selectedViewRadioButtonIndex, CBitmap &theDrawedScreenBitmapOut);
    void UpdateViewForOffScreenDraw(BetaProductViewIndex selectedViewRadioButtonIndex);
    CWnd* GetView(int theWantedDescTopIndex);
    NFmiApplicationWinRegistry& ApplicationWinRegistry();

// Overrides
public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);

// Implementation
public:
	void DoAutoSave(void);
	void ActivateFilterDlg(void);
	void UpdateAllDialogsButtons(void);
	void CreateZoomDlg(NFmiEditMapGeneralDataDoc * theDoc);
	void CreateFilterDlg(NFmiEditMapGeneralDataDoc * theDoc);
    bool UpdateAllViewsAndDialogsIsAllowed();
    void UpdateAllViewsAndDialogs(const std::string &reasonForUpdate, bool fUpdateOnlyMapViews = false);
    void UpdateAllViewsAndDialogs(const std::string &reasonForUpdate, SmartMetViewId updatedViewsFlag);
	void UpdateOnlyExtraMapViews(bool updateMap1, bool updateMap2);
    void CreateTimeEditor(bool callUpdate);
	void CreateLocationSelectionToolDialog(NFmiEditMapGeneralDataDoc * theDoc);
	void CreateBrushToolDlg(NFmiEditMapGeneralDataDoc* theDoc);
	void CreateLocationFinderDlg(NFmiEditMapGeneralDataDoc* theDoc);
	void CreateSmartToolDlg(NFmiEditMapGeneralDataDoc* theDoc);
    void CreateSmartToolsTabControlDlg(NFmiEditMapGeneralDataDoc* theDoc);
    void CreateParameterSelectionDlg(NFmiEditMapGeneralDataDoc *theDoc);
	void CreateTempDlg();
	void CreateShortCutsDlg(void);
	void CreateViewMacroDlg(NFmiEditMapGeneralDataDoc * theDoc);
	void CreateCrossSectionDlg(bool callUpdate = false);
	void CreateSynopPlotSettingsDlg(NFmiEditMapGeneralDataDoc * theDoc);
	void CreateSynopDataGridViewDlg(NFmiEditMapGeneralDataDoc *theDoc);
	void CreateTrajectoryDlg(NFmiEditMapGeneralDataDoc *theDoc);
	void CreateWarningCenterDlg(NFmiEditMapGeneralDataDoc *theDoc);
	void CreateWindTableDlg(NFmiEditMapGeneralDataDoc *theDoc);
	void CreateDataQualityCheckerDialog(NFmiEditMapGeneralDataDoc *theDoc);
	void CreateIgnoreStationsDlg(NFmiEditMapGeneralDataDoc *theDoc);
    void CreateCaseStudyDlg(NFmiEditMapGeneralDataDoc *theDoc);
    void CreateBetaProductDialog(SmartMetDocumentInterface *smartMetDocumentInterface);
    void CreateLogViewer(NFmiEditMapGeneralDataDoc *theDoc);
	void CreateSoundingDataServerConfigurationsDlg();
	void CreateVisualizationSettingsDlg();
    void CreateGriddingOptionsDialog(SmartMetDocumentInterface *smartMetDocumentInterface);
    void UpdateBetaProductDialog();
    CFmiExtraMapViewDlg* CreateExtraMapViewDlg(NFmiEditMapGeneralDataDoc *theDoc, unsigned int theMapViewDescTopIndex);
	void StoreViewMacroWindowsSettings(NFmiViewSettingMacro &theViewMacro);
	void LoadViewMacroWindowsSettings(NFmiViewSettingMacro &theViewMacro);
	void SetMacroErrorText(const std::string &theErrorStr);
	void CaseStudyLoadingActions(const NFmiMetTime &theUsedTime, const std::string &updateReasonText);
	void CaseStudyToNormalModeActions(void);
	void SetAllViewIconsDynamically(void);
    void ActivateParameterSelectionDlg(unsigned int theDescTopIndex = 0);
	CFmiSynopDataGridViewDlg* SynopDataGridViewDlg(void){return itsSynopDataGridViewDlg; }
    void UpdateCrossSectionView(void);
    void UpdateTempView(void);
    void UpdateTimeSerialView(void);
    void UpdateTrajectorySystem(void);
    void ApplyUpdatedViewsFlag(SmartMetViewId updatedViewsFlag);
    SmartMetViewId GetUpdatedViewsFlag() const { return itsUpdatedViewsFlag; }
    SmartMetViewId GetAndResetUpdatedViewsFlag();
    bool HasUpdatedViewsFlagSignificantValue() const { return itsUpdatedViewsFlag != SmartMetViewId::NoViews; }
	void OpenLogViewer();
	void SetAllMapViewTooltipDelays(bool doRestoreAction, int delayInMilliSeconds);

	NFmiEditMapGeneralDataDoc* GetData(void);
	virtual ~CSmartMetDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

// Generated message map functions
public:
	afx_msg void OnButtonTimeEditValuesDlg();
	afx_msg void OnButtonOpenSmartToolDlg();
	afx_msg void OnButtonTempDlg();
	afx_msg void OnButtonShowCrossSection();
	afx_msg void OnExtraMapView1();
	afx_msg void OnExtraMapView2();
protected:
	afx_msg void OnButtonZoomDialog();
	afx_msg void OnButtonDataArea();
	afx_msg void OnButtonLoadData();
	afx_msg void OnButtonStoreData();
	afx_msg void OnMenuitemOptiot();
	afx_msg void OnSelectAll();
	afx_msg void OnDeselectAll();
	afx_msg void OnEditUndo();
	afx_msg void OnEditRedo();
	afx_msg void OnButtonSelectionToolDlg();
	afx_msg void OnMenuitemSaveAs();
	afx_msg void OnButtonSelectEuropeMap();
	afx_msg void OnButtonSelectFinlandMap();
	afx_msg void OnButtonDataToDatabase();
	afx_msg void OnDataLoadFromFile();
	afx_msg void OnUpdateEditUndo(CCmdUI* pCmdUI);
	afx_msg void OnUpdateEditRedo(CCmdUI* pCmdUI);
	afx_msg void OnChangeParamWindowPositionForward();
	afx_msg void OnChangeParamWindowPositionBackward();
	afx_msg void OnMakeGridFile();
	afx_msg void OnButtonMakeEditedDataCopy();
	afx_msg void OnButtonFilterDialog();
	afx_msg void OnButtonBrushToolDlg();
	afx_msg void OnUpdateButtonBrushToolDlg(CCmdUI* pCmdUI);
	afx_msg void OnButtonRefresh();
	afx_msg void OnButtonReloadAllDynamicHelpData();
	afx_msg void OnMenuitemViewGridSelectionDlg2();
	afx_msg void OnEditSpaceOut();
	afx_msg void OnButtonValidateData();
	afx_msg void OnUpdateButtonValidateData(CCmdUI* pCmdUI);
	afx_msg void OnChangeMapType();
	afx_msg void OnUpdateButtonSelectEuropeMap(CCmdUI* pCmdUI);
	afx_msg void OnUpdateButtonSelectFinlandMap(CCmdUI* pCmdUI);
	afx_msg void OnShowTimeString();
	afx_msg void OnShowGridPoints();
	afx_msg void OnToggleGridPointColor();
	afx_msg void OnToggleGridPointSize();
	afx_msg void OnToggleShowNamesOnMap();
	afx_msg void OnButtonGlobe();
	afx_msg void OnUpdateButtonGlobe(CCmdUI* pCmdUI);
	afx_msg void OnShowMasksOnMap();
	afx_msg void OnViewSetTimeEditorPlaceToDefault();
	afx_msg void OnFileOpen();
	afx_msg void OnFileNew();
	afx_msg void OnButtonAnimation();
	afx_msg void OnButtonEditorControlPointMode();
	afx_msg void OnUpdateButtonEditorControlPointMode(CCmdUI* pCmdUI);
	afx_msg void OnButtonDelete();
	afx_msg void OnDataStoreViewToPictureFile();
	afx_msg void OnUpdateDataStoreViewToPictureFile(CCmdUI* pCmdUI);
	afx_msg void OnDataStoreMapView2ToPictureFile();
	afx_msg void OnDataStoreMapView3ToPictureFile();
	afx_msg void OnHelpShortCuts();
	afx_msg void OnShowProjectionLines();
	afx_msg void OnMenuitemProjectionLineSetup();
	afx_msg void OnButtonViewSelectParamDialog();
	afx_msg void OnDataStoreTimeserialviewToPictureFile();
	afx_msg void OnDataStoreTrajectoryViewToPictureFile();
	afx_msg void OnMenuitemGriddingOptions();
	afx_msg void OnUpdateButtonFilterDialog(CCmdUI* pCmdUI);
	afx_msg void OnDataStoreTempviewToPictureFile();
	afx_msg void OnButtonObservationComparisonMode();
	afx_msg void OnUpdateButtonObservationComparisonMode(CCmdUI* pCmdUI);
	afx_msg void OnButtonSynopDataGridView();
	afx_msg void OnUpdateSynopDataGridView(CCmdUI* pCmdUI);
	afx_msg void OnUpdateOnExtraMapView1(CCmdUI *pCmdUI);
	afx_msg void OnUpdateOnExtraMapView2(CCmdUI *pCmdUI);
	DECLARE_MESSAGE_MAP()

	// Generated OLE dispatch map functions

	DECLARE_DISPATCH_MAP()
	DECLARE_INTERFACE_MAP()

private:
	void OnDataStoreExtraMapViewToPictureFile(CFmiExtraMapViewDlg *theExtraMapViewDlg);
	void OnExtraMapView(unsigned int theMapViewDescTopIndex, CFmiExtraMapViewDlg **theExtraMapViewDlg);
	void DoCrashTest(void);
	BOOL LoadDataFromFile(const char* theFileName);
	BOOL StoreEditedData(void);
	BOOL StoreData(const NFmiString& theFileName, boost::shared_ptr<NFmiFastQueryInfo> &theSmartInfo, bool askForSave=true); //laura lisäsi booleanin 10051999
	BOOL StoreWorkingData(boost::shared_ptr<NFmiFastQueryInfo> &smart, bool askForSave = true);
	BOOL SaveViewToFilePicture(const std::string &theCallingFunctionName, CBitmap* bm, const NFmiRect *theRelativeOutputArea);
	BOOL StoreData(const NFmiString& theFileName, boost::shared_ptr<NFmiQueryData> &theData);
    bool IsMapSelected(int theMapIndex);
    void MakeVisualizationImages();
    void LoadViewMacroSettingsForWarningCenterView(NFmiViewSettingMacro &theViewMacro, int &theStartCornerCounter);
    void LoadViewMacroSettingsForTrajectoryDlg(NFmiViewSettingMacro &theViewMacro, int &theStartCornerCounter);
    void LoadViewMacroSettingsForSynopDataGridView(NFmiViewSettingMacro &theViewMacro, int &theStartCornerCounter);
    void LoadViewMacroSettingsForTempDialog(NFmiViewSettingMacro &theViewMacro, int &theStartCornerCounter);
    void LoadViewMacroSettingsForCrossSectionDlg(NFmiViewSettingMacro &theViewMacro, int &theStartCornerCounter);
    void LoadViewMacroSettingsForExtraMapViewDlg(CFmiExtraMapViewDlg **extraMapViewDlg, unsigned int theMapViewDescTopIndex, NFmiViewSettingMacro &theViewMacro, int &theStartCornerCounter);
    void LoadViewMacroSettingsForTimeSerialDataView(NFmiViewSettingMacro &theViewMacro, int &theStartCornerCounter);
    void MakeViewActivationAfterLoadingViewMacro();
    void HandleCpAccelerator(ControlPointAcceleratorActions action, const std::string &updateMessage);
    void StoreBitmapImageToFile(const std::string &callingFunctionName, const std::string &imageViewName, CBitmap *bitmap, const NFmiRect *theRelativeOutputArea);
    void MakeNextUpdateOnSynopDataGridViewDlgForced();

	CTimeEditValuesDlg *itsTimeSerialDataEditorDlg;
	NFmiEditMapGeneralDataDoc* itsData;
	CFmiLocationSelectionToolDlg2* itsLocationSelectionToolDlg;
	CZoomDlg* itsZoomDlg;
	CFmiFilterDataDlg* itsFilterDlg;
	CFmiBrushToolDlg* itsBrushToolDlg;
	CFmiLocationFinderDlg *itsLocationFinderDlg;
	CFmiSmartToolDlg* itsSmartToolDlg;
    CFmiSmarttoolsTabControlDlg* itsSmarttoolsTabControlDlg;
	CFmiTempDlg* itsTempDialog;  // Luotaus dialogi Viljolta
	CFmiShortCutsDlg2* itsShortCutsDialog;  // pikänäppäin dialogi
	CFmiViewMacroDlg* itsViewMacroDlg;
	CFmiCrossSectionDlg* itsCrossSectionDlg; // poikkileikkaus-dialogi
	CFmiSynopPlotSettingsDlg *itsSynopPlotSettingsDlg; // synop asetusten säätö dialogi
	CFmiSynopDataGridViewDlg* itsSynopDataGridViewDlg;  // asema datan katselu taulukossa
	CFmiTrajectoryDlg *itsTrajectoryDlg;
	CFmiWarningCenterDlg *itsWarningCenterDlg;
	CFmiExtraMapViewDlg *itsExtraMapViewDlg1;
	CFmiExtraMapViewDlg *itsExtraMapViewDlg2;
	CFmiWindTableDlg *itsWindTableDlg;
	CFmiDataQualityCheckerDialog *itsDataQualityCheckerDialog;
	CFmiIgnoreStationsDlg *itsIgnoreStationsDlg;
	CFmiCaseStudyDlg *itsCaseStudyDlg;
    CFmiBetaProductTabControlDialog *itsBetaProductDialog;
    CFmiLogViewer *itsLogViewer;
    CFmiParameterSelectionDlg *itsParameterSelectionDlg;
    CFmiGriddingOptionsDlg *itsGriddingOptionsDlg;
    CFmiSoundingDataServerConfigurationsDlg *itsSoundingDataServerConfigurationsDlg;
	CFmiVisualizationSettings* itsVisualizationSettings;
    unsigned int itsMapViewDescTopIndex;
    // Joskus halutaan sallia päivitys vain osalle näyttöjä (optimointia), vaikka lopulta 
    // käytetäänkin päivityksen aloitukseen yleistä CSmartMetDoc::UpdateAllViewsAndDialogs -metodia, jonka 
    // tarkoitus on päivittää kaikkia mahdollisia näyttöjä (jotka ovat auki).
    // Jos itsUpdatedViewsFlag:in arvoksi on laitettu jotain muuta kuin NoViews, tällöin toteutetaankin
    // se CSmartMetDoc::UpdateAllViewsAndDialogs -metodi, jolle annetaan updatedViewsFlag -parametri.
    // Lopuksi aina nollataan itsUpdatedViewsFlag:in arvo.
    SmartMetViewId itsUpdatedViewsFlag = SmartMetViewId::NoViews;
public:
	afx_msg void OnToggleLandBorderDrawColor();
	afx_msg void OnToggleLandBorderPenSize();
	afx_msg void OnAcceleratorStoreViewMacro();
	afx_msg void OnUpdateButtonDataToDatabase(CCmdUI *pCmdUI);
	afx_msg void OnUpdateButtonStoreData(CCmdUI *pCmdUI);
	afx_msg void OnUpdateButtonSelectionToolDlg(CCmdUI *pCmdUI);
	afx_msg void OnUpdateSelectAll(CCmdUI *pCmdUI);
	afx_msg void OnUpdateMenuitemSaveAs(CCmdUI *pCmdUI);
	afx_msg void OnUpdateMenuitemGriddingOptions(CCmdUI *pCmdUI);
	afx_msg void OnUpdateDeselectAll(CCmdUI *pCmdUI);
	afx_msg void OnUpdateButtonMakeEditedDataCopy(CCmdUI *pCmdUI);
	afx_msg void OnUpdateMakeGridFile(CCmdUI *pCmdUI);
	afx_msg void OnUpdateEditPaste(CCmdUI *pCmdUI);
	afx_msg void OnUpdateEditCopy(CCmdUI *pCmdUI);
	afx_msg void OnAcceleratorCrossSectionMode();
	afx_msg void OnButtonSelectScandinaviaMap();
	afx_msg void OnUpdateButtonSelectScandinaviaMap(CCmdUI *pCmdUI);
	afx_msg void OnAcceleratorMapRow1();
	afx_msg void OnAcceleratorMapRow2();
	afx_msg void OnAcceleratorMapRow3();
	afx_msg void OnAcceleratorMapRow4();
	afx_msg void OnAcceleratorMapRow5();
    afx_msg void OnAcceleratorMapRow6();
    afx_msg void OnAcceleratorMapRow7();
    afx_msg void OnAcceleratorMapRow8();
    afx_msg void OnAcceleratorMapRow9();
    afx_msg void OnAcceleratorMapRow10();
    afx_msg void OnButtonShowSynopPlotSettings();
	afx_msg void OnAcceleratorObsComparisonChangeSymbol();
	afx_msg void OnAcceleratorObsComparisonChangeSymbolSize();
	afx_msg void OnAcceleratorObsComparisonToggleBorderDraw();
	afx_msg void OnUpdateButtonTempDlg(CCmdUI *pCmdUI);
	afx_msg void OnUpdateButtonShowCrossSection(CCmdUI *pCmdUI);
	afx_msg void OnUpdateButtonTimeEditValuesDlg(CCmdUI *pCmdUI);
	afx_msg void OnDataStoreCrosssectionviewToPictureFile();
	void OnViewSetMapView2PlaceToDefault(CFmiExtraMapViewDlg *theExtraMapViewDlg, const std::string &updateMessage);
	afx_msg void OnViewSetMapView2PlaceToDefault();
	afx_msg void OnViewSetMapView3PlaceToDefault();
	afx_msg void OnViewSetTempViewPlaceToDefault();
	afx_msg void OnViewSetCrosssectionViewPlaceToDefault();
	virtual void OnCloseDocument();
	afx_msg void OnViewSetZoomViewPlaceToDefault();
	afx_msg void OnViewSetViewMacroDlgPlaceToDefault();
	afx_msg void OnViewSetSynopTextViewPlaceToDefault();
	afx_msg void OnAcceleratorToggleHelpCursorOnMap();
	afx_msg void OnOhjeVirhetesti();
	afx_msg void OnMenuitemMuokkaaKieli();
	afx_msg void OnButtonTrajectory();
	afx_msg void OnUpdateButtonTrajectory(CCmdUI *pCmdUI);
	afx_msg void OnViewSetTrajectoryViewPlaceToDefault();
	afx_msg void OnButtonWarningCenterDlg();
	afx_msg void OnViewEditorLog();
	afx_msg void OnToggleOverMapBackForeGround();
	afx_msg void OnAcceleratorToggleTooltip();
	afx_msg void OnAcceleratorToggleKeepMapRatio();
	afx_msg void OnMenuitemHelpEditorModeSettings();
	afx_msg void OnButtonHelpEditorMode();
	afx_msg void OnUpdateButtonHelpEditorMode(CCmdUI *pCmdUI);
	afx_msg void OnViewSetWarningCenterDlgPlaceToDefault();

	afx_msg void OnAcceleratorBorrowParams1();
	afx_msg void OnAcceleratorBorrowParams2();
	afx_msg void OnAcceleratorBorrowParams3();
	afx_msg void OnAcceleratorBorrowParams4();
	afx_msg void OnAcceleratorBorrowParams5();
	afx_msg void OnAcceleratorBorrowParams6();
	afx_msg void OnAcceleratorBorrowParams7();
	afx_msg void OnAcceleratorBorrowParams8();
	afx_msg void OnAcceleratorBorrowParams9();
	afx_msg void OnAcceleratorBorrowParams10();
	afx_msg void OnButtonWindTableDlg();
	afx_msg void OnViewSetWindTableDlgPlaceToDefault();
	afx_msg void OnChangePreviousMapType();
	afx_msg void OnToggleShowPreviousNamesOnMap();
	afx_msg void OnAcceleratorLocationFinderTool();
	afx_msg void OnButtonDataQualityChecker();
	afx_msg void OnViewSetDataQualityCheckerDlgPlaceToDefault();
	afx_msg void OnUpdateButtonDataQualityChecker(CCmdUI *pCmdUI);
	afx_msg void OnAcceleratorIgnoreStationsDlg();
	afx_msg void OnViewCaseStudyDialog();
    afx_msg void OnAcceleratorAreaInfoToClipboard();
    afx_msg void OnViewSetSmarttoolViewPlaceToDefault();
    afx_msg void OnTooltipstatusCrosssectionview();
    afx_msg void OnUpdateTooltipstatusCrosssectionview(CCmdUI *pCmdUI);
    afx_msg void OnTooltipstatusMapviews();
    afx_msg void OnUpdateTooltipstatusMapviews(CCmdUI *pCmdUI);
    afx_msg void OnTooltipstatusSoundingview();
    afx_msg void OnUpdateTooltipstatusSoundingview(CCmdUI *pCmdUI);
    afx_msg void OnTooltipstatusTimeserialview();
    afx_msg void OnUpdateTooltipstatusTimeserialview(CCmdUI *pCmdUI);
    afx_msg void OnAcceleratorApplyBackupViewMacro();
    afx_msg void OnAcceleratorApplyCrashBackupViewMacro();
    afx_msg void OnReloaddataReloadfailedsatelliteimages();
    afx_msg void OnAcceleratorDevelopedFunctionTesting();
    afx_msg void OnViewBetaproduction();
    afx_msg void OnAcceleratorResetTimeFilterTimes();
    afx_msg void OnAcceleratorLogViewer();
    afx_msg void OnAcceleratorCalculationpointInfoToClipboard();
    afx_msg void OnAcceleratorDataFilepathsToClipboard();
    afx_msg void OnAcceleratorMoveManyMapRowsUp();
    afx_msg void OnAcceleratorMoveManyMapRowsDown();
    afx_msg void OnAcceleratorApplyStartupViewMacro();
    afx_msg void OnAcceleratorToggleWmsMapMode();
    afx_msg void OnSetParameterSelectionDlgPlaceToDefault();
    afx_msg void OnHelpExceptiontest();
    afx_msg void OnAcceleratorCpSelectNext();
    afx_msg void OnAcceleratorCpSelectPrevious();
    afx_msg void OnAcceleratorCpSelectLeft();
    afx_msg void OnAcceleratorCpSelectRight();
    afx_msg void OnAcceleratorCpSelectUp();
    afx_msg void OnAcceleratorCpSelectDown();
    void OpenLocationFinderTool(CWnd *parentView);
    afx_msg void OnEditSoundingDataFromServerSettings();
    afx_msg void OnViewSetSoundingsFromServerSettingsPlaceToDefault();
    afx_msg void OnButtonBetaProductDialog();
    afx_msg void OnAcceleratorDoVisualizationProfiling();
	afx_msg void OnViewSetLogViewerDlgPlaceToDefault();
	afx_msg void OnViewSetCaseStudyDlgPlaceToDefault();
	afx_msg void OnViewSetBetaProductionDlgPlaceToDefault();
	afx_msg void OnEditVisualizationsettings();
	afx_msg void OnMoveviewsvisibleVisualizationsettingsposition();
	afx_msg void OnAcceleratorMoveTimeBoxLocation();
};


