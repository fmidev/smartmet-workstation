// OptionsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "OptionsDlg.h"
#include "SmartMetDocumentInterface.h"
#include "FmiDataValidationDlg.h"
#include "NFmiMetEditorOptionsData.h"
#include "NFmiValueString.h"
#include "NFmiDictionaryFunction.h"
#include "NFmiMapViewDescTop.h"
#include "FmiWin32Helpers.h"
#include "CloneBitmap.h"
#include "NFmiHelpDataInfo.h"
#include "NFmiApplicationDataBase.h"
#include "NFmiApplicationWinRegistry.h"
#include "Q2ServerInfo.h"
#include "NFmiParameterInterpolationFixer.h"
#include "boost\math\special_functions\round.hpp"
#include "XFolderDialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// COptionsDlg dialog

COptionsDlg::COptionsDlg(CWnd* pParent /*=NULL*/)
	: CDialog(COptionsDlg::IDD, pParent)
	, fUseViewMode(false)
    , itsQ2ServerUrlStrU_(_T(""))
    , itsQ3ServerUrlStrU_(_T(""))
	, fQ2ServerUsed(FALSE)
	, fQ2ServerZipUsed(FALSE)
	, fLogQ2Requests(FALSE)
	, itsQ2ServerGridsizeX(0)
	, itsQ2ServerGridsizeY(0)
	, itsStationDataGridsizeX(0)
	, itsStationDataGridsizeY(0)
	, itsQ2ServerDecimalCount(0)
	, fDoAutoLoadDataAtStartUp(FALSE)
	, itsDrawObjectScaleFactor(0)
	, fSmartOrientationPrint(FALSE)
	, fFitToPagePrint(FALSE)
	, fLowMemoryPrint(FALSE)
	, itsMaxRangeInPrint(0)
	, fUseLocalCache(FALSE)
	, fDoCleanLocalCache(FALSE)
	, itsCacheKeepFilesMaxDays(0)
	, itsSatelDataUpdateFrequenceInMinutes(0)
	, fAllowSending(FALSE)
    , itsSysInfoDbUrlU_(_T(""))
    , fAutoLoadNewCacheData(TRUE)
    , itsLocationFinderTimeoutInSeconds(0)
    , fShowLastSendTimeOnMapView(FALSE)
    , fUseCombinedMapMode(FALSE)
    , fDroppedDataEditable(FALSE)
    , itsIsolineMinimumLengthFactor(1)
    , fGenerateTimeCombinationData(FALSE)
    , fUseForcedLinearInterpolationOption(FALSE)
    , itsLocalCacheDirectoryPath(_T(""))
    , fUseLedLightStatusSystem(FALSE)
{
	//{{AFX_DATA_INIT(COptionsDlg)
	fStationPlot = FALSE;
    itsTimeStepStrU_ = _T("");
	fShowToolTip = FALSE;
	fUseSpacingOut = FALSE;
	fAutoZoom = FALSE;
    itsViewCacheSizeStrU_ = _T("");
    itsUndoRedoDepthStrU_ = _T("");
	fUseAutoSave = FALSE;
    itsAnimationFrameDelayInMSecStrU_ = _T("");
    itsAutoSaveFrequensInMinutesStrU_ = _T("");
	//}}AFX_DATA_INIT
}


void COptionsDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(COptionsDlg)
    DDX_Check(pDX, IDC_CHECK_STATION_PLOT, fStationPlot);
    DDX_Text(pDX, IDC_EDIT_TIME_STEP, itsTimeStepStrU_);
    DDX_Check(pDX, IDC_CHECK_SHOW_TOOLTIP, fShowToolTip);
    DDX_Check(pDX, IDC_CHECK_USE_SPACINGOUT, fUseSpacingOut);
    DDX_Check(pDX, IDC_CHECK_USE_AUTO_ZOOM, fAutoZoom);
    DDX_Text(pDX, IDC_EDIT_VIEW_CACHE_SIZE, itsViewCacheSizeStrU_);
    DDX_Text(pDX, IDC_EDIT_UNDO_REDO_DEPTH, itsUndoRedoDepthStrU_);
    DDX_Check(pDX, IDC_CHECK_USE_AUTO_SAVE, fUseAutoSave);
    DDX_Check(pDX, IDC_CHECK_USE_VIEW_MODE, fUseViewMode);
    DDX_Text(pDX, IDC_EDIT_ANIMATION_FRAME_LENGTH_IN_MILLISECONDS, itsAnimationFrameDelayInMSecStrU_);
    DDX_Text(pDX, IDC_EDIT_AUTO_SAVE_FREQUENSSI_IN_MINUTES, itsAutoSaveFrequensInMinutesStrU_);
    //}}AFX_DATA_MAP
    DDX_Text(pDX, IDC_EDIT_OPTIONS_Q2SERVER_URL, itsQ2ServerUrlStrU_);
    DDX_Text(pDX, IDC_EDIT_OPTIONS_Q3SERVER_URL, itsQ3ServerUrlStrU_);
    DDX_Check(pDX, IDC_CHECK_OPTIONS_Q2SERVER_USE, fQ2ServerUsed);
    DDX_Check(pDX, IDC_CHECK_OPTIONS_Q2SERVER_ZIP, fQ2ServerZipUsed);
    DDX_Check(pDX, IDC_CHECK_OPTIONS_Q2SERVER_LOG, fLogQ2Requests);
    DDX_Text(pDX, IDC_EDIT_OPTIONS_Q2SERVER_GRID_SIZE_X, itsQ2ServerGridsizeX);
    DDV_MinMaxInt(pDX, itsQ2ServerGridsizeX, 10, 1000);
    DDX_Text(pDX, IDC_EDIT_OPTIONS_Q2SERVER_GRID_SIZE_Y, itsQ2ServerGridsizeY);
    DDV_MinMaxInt(pDX, itsQ2ServerGridsizeY, 10, 1000);
    DDX_Text(pDX, IDC_EDIT_OPTIONS_STATION_PARAM_GRID_SIZE_X, itsStationDataGridsizeX);
    DDV_MinMaxInt(pDX, itsStationDataGridsizeX, 10, 500);
    DDX_Text(pDX, IDC_EDIT_OPTIONS_STATION_PARAM_GRID_SIZE_Y, itsStationDataGridsizeY);
    DDV_MinMaxInt(pDX, itsStationDataGridsizeY, 10, 500);
    DDX_Text(pDX, IDC_EDIT_OPTIONS_Q2SERVER_GRID_SIZE_X2, itsQ2ServerDecimalCount);
    DDV_MinMaxInt(pDX, itsQ2ServerDecimalCount, 0, 7);
    DDX_Check(pDX, IDC_CHECK_DO_AUTO_LOAD_DATA_AT_STARTUP, fDoAutoLoadDataAtStartUp);
    DDX_Text(pDX, IDC_EDIT_DRAW_OBJECT_SCALE_FACTOR, itsDrawObjectScaleFactor);
    DDV_MinMaxDouble(pDX, itsDrawObjectScaleFactor, 0, 5);
    DDX_Check(pDX, IDC_CHECK_SMART_ORIENTATION_PRINT, fSmartOrientationPrint);
    DDX_Check(pDX, IDC_CHECK_FIT_TO_PAPER_PRINT, fFitToPagePrint);
    DDX_Check(pDX, IDC_CHECK_LOW_MEMORY_PRINTING, fLowMemoryPrint);
    DDX_Text(pDX, IDC_EDIT_OPTIONS_MAX_PAGE_RANGE, itsMaxRangeInPrint);
    DDX_Check(pDX, IDC_CHECK_CACHE_USE, fUseLocalCache);
    DDX_Check(pDX, IDC_CHECK_CLEAN_CACHE, fDoCleanLocalCache);
    DDX_Text(pDX, IDC_EDIT_CACHE_KEEP_FILES_DAYS, itsCacheKeepFilesMaxDays);
    DDX_Text(pDX, IDC_EDIT_SATEL_DATA_UPDATE_FREQ_IN_MINUTES, itsSatelDataUpdateFrequenceInMinutes);
    DDX_Check(pDX, IDC_CHECK_ALLOW_SYSTEM_INFO_SEND_TO_DB, fAllowSending);
    DDX_Text(pDX, IDC_EDIT_FMI_SYS_INFO_DB_URL, itsSysInfoDbUrlU_);
    DDX_Check(pDX, IDC_CHECK_DO_AUTO_LOAD_NEW_CACHE_DATA, fAutoLoadNewCacheData);
    DDX_Text(pDX, IDC_EDIT_OPTIONS_LOCATION_FINDER_TIMEOUT_IN_SECONDS, itsLocationFinderTimeoutInSeconds);
    DDX_Check(pDX, IDC_CHECK_SHOW_LAST_SEND_TIME_ON_MAP_VIEW, fShowLastSendTimeOnMapView);
    DDX_Check(pDX, IDC_CHECK_USE_COMBINED_MAP_MODE, fUseCombinedMapMode);
    DDX_Control(pDX, IDC_COMBO_OPTIONS_LOG_LEVEL, itsLogLevelComboBox);
    DDX_Check(pDX, IDC_CHECK_DROPPED_DATA_EDITABLE, fDroppedDataEditable);
    DDX_Text(pDX, IDC_EDIT_ISOLINE_MINIMUM_LENGTH_FACTOR, itsIsolineMinimumLengthFactor);
    DDV_MinMaxDouble(pDX, itsIsolineMinimumLengthFactor, 0, 100);
    DDX_Check(pDX, IDC_CHECK_MAKE_COMBINATION_DATA, fGenerateTimeCombinationData);
    DDX_Check(pDX, IDC_CHECK_USE_FORCED_LINEAR_INTERPOLATION_OPTION, fUseForcedLinearInterpolationOption);
    DDX_Text(pDX, IDC_EDIT_HATCHING_EPSILON_FACTOR, itsHatchingToolmasterEpsilonFactor);
    DDX_Text(pDX, IDC_EDIT_LOCAL_CACHE_DIRECTORY, itsLocalCacheDirectoryPath);
    DDX_Control(pDX, IDC_STATIC_RESTART_SMARTMET_STR, itsRestartSmartMetStr);
    DDX_Check(pDX, IDC_CHECK_USE_LED_LIGHT_STATUS_SYSTEM, fUseLedLightStatusSystem);
}


BEGIN_MESSAGE_MAP(COptionsDlg, CDialog)
	//{{AFX_MSG_MAP(COptionsDlg)
	ON_BN_CLICKED(ID_BUTTON_VALIDATION_DIALOG, OnButtonValidationDialog)
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_CHECK_USE_VIEW_MODE, OnBnClickedCheckUseViewMode)
    ON_WM_CTLCOLOR()
    ON_BN_CLICKED(IDC_BUTTON_BROWSE_LOCAL_CACHE_DIRECTORY, &COptionsDlg::OnBnClickedButtonBrowseLocalCacheDirectory)
    ON_EN_CHANGE(IDC_EDIT_LOCAL_CACHE_DIRECTORY, &COptionsDlg::OnEnChangeEditLocalCacheDirectory)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// COptionsDlg message handlers

BOOL COptionsDlg::OnInitDialog()
{

	CDialog::OnInitDialog();

    CFmiWin32Helpers::SetUsedWindowIconDynamically(this);
    auto &applicationWinRegistry = itsSmartMetDocumentInterface->ApplicationWinRegistry();
	InitDialogTexts();
    fStationPlot = applicationWinRegistry.ConfigurationRelatedWinRegistry().MapView(0)->ShowStationPlot();
	NFmiValueString str(itsSmartMetDocumentInterface->MapViewDescTop(0)->TimeControlTimeStep(), "%0.4f");
	itsTimeStepStrU_ = CA2T(str);
    auto &metEditorOptionsData = itsSmartMetDocumentInterface->MetEditorOptionsData();
	fShowToolTip = metEditorOptionsData.ShowToolTipsOnMapView();
    fUseSpacingOut = applicationWinRegistry.ConfigurationRelatedWinRegistry().MapView(0)->SpacingOutFactor() > 0;
    fAutoZoom = applicationWinRegistry.KeepMapAspectRatio();
	NFmiValueString str2(applicationWinRegistry.MapViewCacheMaxSizeInMB(), "%0.f");
    itsViewCacheSizeStrU_ = CA2T(str2);
	NFmiValueString str3(metEditorOptionsData.UndoRedoDepth(), "%d");
    itsUndoRedoDepthStrU_ = CA2T(str3);
	NFmiValueString str4(200, "%d");
    itsAnimationFrameDelayInMSecStrU_ = CA2T(str4);
	NFmiValueString str5(metEditorOptionsData.AutoSaveFrequensInMinutes(), "%d");
    itsAutoSaveFrequensInMinutesStrU_ = CA2T(str5);
	itsSatelDataUpdateFrequenceInMinutes = itsSmartMetDocumentInterface->SatelDataRefreshTimerInMinutes();
	fUseAutoSave = metEditorOptionsData.UseAutoSave();
	fUseViewMode = itsSmartMetDocumentInterface->SmartMetEditingMode() != CtrlViewUtils::kFmiEditingModeNormal;

    auto &q2ServerInfo = itsSmartMetDocumentInterface->GetQ2ServerInfo();
    itsQ2ServerUrlStrU_ = CA2T(q2ServerInfo.Q2ServerURLStr().c_str());
    itsQ3ServerUrlStrU_ = CA2T(q2ServerInfo.Q3ServerUrl().c_str());
	itsQ2ServerDecimalCount = q2ServerInfo.Q2ServerDecimalCount();
	fQ2ServerUsed = q2ServerInfo.UseQ2Server();
	fQ2ServerZipUsed = q2ServerInfo.Q2ServerUsedZipMethod() != 0;
	fLogQ2Requests = q2ServerInfo.LogQ2Requests();
    auto &q2GridSize = q2ServerInfo.Q2ServerGridSize();
	itsQ2ServerGridsizeX = static_cast<int>(q2GridSize.X());
	itsQ2ServerGridsizeY = static_cast<int>(q2GridSize.Y());
    auto &stationGridSize = itsSmartMetDocumentInterface->StationDataGridSize();
	itsStationDataGridsizeX = static_cast<int>(stationGridSize.X());
	itsStationDataGridsizeY = static_cast<int>(stationGridSize.Y());

	fDoAutoLoadDataAtStartUp = itsSmartMetDocumentInterface->DoAutoLoadDataAtStartUp();
	itsDrawObjectScaleFactor = itsSmartMetDocumentInterface->DrawObjectScaleFactor();
    fSmartOrientationPrint = applicationWinRegistry.SmartOrientationPrint();
	fFitToPagePrint = applicationWinRegistry.FitToPagePrint();
	fLowMemoryPrint = applicationWinRegistry.LowMemoryPrint();
	itsMaxRangeInPrint = applicationWinRegistry.MaxRangeInPrint();

	fUseLocalCache = itsSmartMetDocumentInterface->HelpDataInfoSystem()->UseQueryDataCache();
	fDoCleanLocalCache = itsSmartMetDocumentInterface->HelpDataInfoSystem()->DoCleanCache();
	itsCacheKeepFilesMaxDays = itsSmartMetDocumentInterface->HelpDataInfoSystem()->CacheFileKeepMaxDays();
	fAllowSending = itsSmartMetDocumentInterface->ApplicationDataBase().UseDataSending();
    itsSysInfoDbUrlU_ = CA2T(itsSmartMetDocumentInterface->ApplicationDataBase().BaseUrlString().c_str());
    fAutoLoadNewCacheData = applicationWinRegistry.ConfigurationRelatedWinRegistry().AutoLoadNewCacheData();
    itsLocationFinderTimeoutInSeconds = applicationWinRegistry.LocationFinderThreadTimeOutInMS() / 1000.;
    fShowLastSendTimeOnMapView = applicationWinRegistry.ConfigurationRelatedWinRegistry().ShowLastSendTimeOnMapView();
    fUseCombinedMapMode = applicationWinRegistry.ConfigurationRelatedWinRegistry().UseCombinedMapMode();
    InitLogLevelComboBox();
    fDroppedDataEditable = applicationWinRegistry.ConfigurationRelatedWinRegistry().DroppedDataEditable();
    itsIsolineMinimumLengthFactor = applicationWinRegistry.IsolineMinLengthFactor();
    fGenerateTimeCombinationData = applicationWinRegistry.GenerateTimeCombinationData();
    fUseForcedLinearInterpolationOption = itsSmartMetDocumentInterface->ParameterInterpolationFixer().doForcedParameterInterpolationChanges();
    itsHatchingToolmasterEpsilonFactor = applicationWinRegistry.HatchingToolmasterEpsilonFactor();
    itsLocalCacheDirectoryPathOriginal = itsLocalCacheDirectoryPath = CA2T(itsSmartMetDocumentInterface->HelpDataInfoSystem()->LocalDataBaseDirectory().c_str());
    fUseLedLightStatusSystem = applicationWinRegistry.UseLedLightStatusSystem();
    CFmiWin32Helpers::SetDialogItemVisibility(this, IDC_STATIC_RESTART_SMARTMET_STR, false);
	DisableControls();

	UpdateData(FALSE);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void COptionsDlg::DisableControls(void)
{

	if(itsSmartMetDocumentInterface->SmartMetEditingMode() != CtrlViewUtils::kFmiEditingModeNormal) // jos ollaan ns. view-moodissa, disabloidaan muutamia kontrolleja
	{
		CWnd *win = GetDlgItem(ID_BUTTON_VALIDATION_DIALOG);
		if(win)
			win->EnableWindow(FALSE);

		win = GetDlgItem(IDC_EDIT_UNDO_REDO_DEPTH);
		if(win)
			win->EnableWindow(FALSE);
		win = GetDlgItem(IDC_STATIC_UNDO_REDO_1);
		if(win)
			win->EnableWindow(FALSE);

		win = GetDlgItem(IDC_CHECK_USE_AUTO_SAVE);
		if(win)
			win->EnableWindow(FALSE);
		win = GetDlgItem(IDC_EDIT_AUTO_SAVE_FREQUENSSI_IN_MINUTES);
		if(win)
			win->EnableWindow(FALSE);
	}
	else
	{
		CWnd *win = GetDlgItem(ID_BUTTON_VALIDATION_DIALOG);
		if(win)
			win->EnableWindow(TRUE);

		win = GetDlgItem(IDC_EDIT_UNDO_REDO_DEPTH);
		if(win)
			win->EnableWindow(TRUE);
		win = GetDlgItem(IDC_STATIC_UNDO_REDO_1);
		if(win)
			win->EnableWindow(TRUE);

		win = GetDlgItem(IDC_CHECK_USE_AUTO_SAVE);
		if(win)
			win->EnableWindow(TRUE);
		win = GetDlgItem(IDC_EDIT_AUTO_SAVE_FREQUENSSI_IN_MINUTES);
		if(win)
			win->EnableWindow(TRUE);
	}

	CWnd *win2 = GetDlgItem(ID_BUTTON_VALIDATION_DIALOG);
	if(win2)
		win2->EnableWindow(itsSmartMetDocumentInterface->MetEditorOptionsData().DataValidationEnabled());
}

void COptionsDlg::OnOK()
{
	UpdateData(TRUE);
    auto &applicationWinRegistry = itsSmartMetDocumentInterface->ApplicationWinRegistry();
    auto& configurationRelatedWinRegistry = applicationWinRegistry.ConfigurationRelatedWinRegistry();
    configurationRelatedWinRegistry.MapView(0)->ShowStationPlot(fStationPlot == TRUE);
    NFmiValueString valueStr = CT2A(itsTimeStepStrU_);
    auto* mainMapViewDescTop = itsSmartMetDocumentInterface->MapViewDescTop(0);
	float stepValue = float(valueStr);
	if(stepValue <= 0.0001f) // 0.0001-raja hatusta tarkoittaisi alle sekunnin aika-askelta valuestring palauttaa roskalle ep�m��r�isen arvon, joka on alle t�m�n rajan
        mainMapViewDescTop->TimeControlTimeStep(1.f);
	else
        mainMapViewDescTop->TimeControlTimeStep(stepValue);

    auto &metEditorOptionsData = itsSmartMetDocumentInterface->MetEditorOptionsData();
    metEditorOptionsData.ShowToolTipsOnMapView(fShowToolTip == TRUE);
    configurationRelatedWinRegistry.MapView(0)->SpacingOutFactor(fUseSpacingOut == TRUE ? 1 : 0); // nyt vain on/off s��t�
    applicationWinRegistry.KeepMapAspectRatio(fAutoZoom == TRUE);
    itsSmartMetDocumentInterface->SmartMetEditingMode(fUseViewMode ? CtrlViewUtils::kFmiEditingModeViewOnly : CtrlViewUtils::kFmiEditingModeNormal, true);

    NFmiValueString cacheSizeStr = CT2A(itsViewCacheSizeStrU_);
	float cacheSize = float(cacheSizeStr);
    // Must call this SetMapViewCacheSize -function to store new setting AND also set actual map-view-cache-sizes
    itsSmartMetDocumentInterface->SetMapViewCacheSize(cacheSize);

    NFmiValueString animationDelayStr = CT2A(itsAnimationFrameDelayInMSecStrU_);
	float animationDelay = float(animationDelayStr);

	if(itsSmartMetDocumentInterface->SmartMetEditingMode() == CtrlViewUtils::kFmiEditingModeNormal) 
	{	
        // Jos ollaan ns. edit-moodissa, tehd��n undo/redo syvyys ja autosave asetuksia
        // Muuten on vaara, ett� OnInit:iss� nollattuja arvoja laitetaan takaisin oikeisiin asetuksiin
        NFmiValueString autoSaveFreqStr = CT2A(itsAutoSaveFrequensInMinutesStrU_);
		float autoSaveFreq = float(autoSaveFreqStr);
        metEditorOptionsData.AutoSaveFrequensInMinutes(static_cast<int>(autoSaveFreq));

        metEditorOptionsData.UseAutoSave(fUseAutoSave == TRUE);

        NFmiValueString undoRedoDepthStr = CT2A(itsUndoRedoDepthStrU_);
		int undoRedoDepth = int(undoRedoDepthStr);
        metEditorOptionsData.UndoRedoDepth(undoRedoDepth);
	}

    auto &q2ServerInfo = itsSmartMetDocumentInterface->GetQ2ServerInfo();
    std::string tmpStr = CT2A(itsQ2ServerUrlStrU_);
    q2ServerInfo.Q2ServerURLStr(tmpStr);
    tmpStr = CT2A(itsQ3ServerUrlStrU_);
    q2ServerInfo.Q3ServerUrl(tmpStr);
    q2ServerInfo.Q2ServerDecimalCount(itsQ2ServerDecimalCount);
    q2ServerInfo.UseQ2Server(fQ2ServerUsed == TRUE);
    q2ServerInfo.Q2ServerUsedZipMethod(fQ2ServerZipUsed ? 1 : 0);
    q2ServerInfo.LogQ2Requests(fLogQ2Requests == TRUE);
    q2ServerInfo.Q2ServerGridSize(NFmiPoint(itsQ2ServerGridsizeX, itsQ2ServerGridsizeY));
    itsSmartMetDocumentInterface->StationDataGridSize(NFmiPoint(itsStationDataGridsizeX, itsStationDataGridsizeY));

    itsSmartMetDocumentInterface->DoAutoLoadDataAtStartUp(fDoAutoLoadDataAtStartUp == TRUE);
    itsSmartMetDocumentInterface->DrawObjectScaleFactor(itsDrawObjectScaleFactor);
    applicationWinRegistry.SmartOrientationPrint(fSmartOrientationPrint == TRUE);
    applicationWinRegistry.FitToPagePrint(fFitToPagePrint == TRUE);
    applicationWinRegistry.LowMemoryPrint(fLowMemoryPrint == TRUE);
    applicationWinRegistry.MaxRangeInPrint(itsMaxRangeInPrint);

    auto* helpDataInfoSystem = itsSmartMetDocumentInterface->HelpDataInfoSystem();
    helpDataInfoSystem->UseQueryDataCache(fUseLocalCache == TRUE);
    helpDataInfoSystem->DoCleanCache(fDoCleanLocalCache == TRUE);
    helpDataInfoSystem->CacheFileKeepMaxDays(static_cast<float>(itsCacheKeepFilesMaxDays));
    itsSmartMetDocumentInterface->SatelDataRefreshTimerInMinutes(itsSatelDataUpdateFrequenceInMinutes);
    itsSmartMetDocumentInterface->ApplicationDataBase().UseDataSending(fAllowSending == TRUE);
    tmpStr = CT2A(itsSysInfoDbUrlU_);
    itsSmartMetDocumentInterface->ApplicationDataBase().BaseUrlString(tmpStr);

    configurationRelatedWinRegistry.AutoLoadNewCacheData(fAutoLoadNewCacheData == TRUE);
    applicationWinRegistry.LocationFinderThreadTimeOutInMS(boost::math::iround(itsLocationFinderTimeoutInSeconds * 1000.));
    configurationRelatedWinRegistry.ShowLastSendTimeOnMapView(fShowLastSendTimeOnMapView == TRUE);
    configurationRelatedWinRegistry.UseCombinedMapMode(fUseCombinedMapMode == TRUE);
    SetLogLevelOnOk();
    configurationRelatedWinRegistry.DroppedDataEditable(fDroppedDataEditable == TRUE);
    applicationWinRegistry.IsolineMinLengthFactor(itsIsolineMinimumLengthFactor);
    applicationWinRegistry.GenerateTimeCombinationData(fGenerateTimeCombinationData == TRUE);
    itsSmartMetDocumentInterface->ParameterInterpolationFixer().doForcedParameterInterpolationChanges(fUseForcedLinearInterpolationOption == TRUE);
    // HatchingToolmasterEpsilonFactor:in asetus pit�� tehd� n�in, koska arvo asetetaan t�ss� useampaan paikkaan.
    itsSmartMetDocumentInterface->SetHatchingToolmasterEpsilonFactor(itsHatchingToolmasterEpsilonFactor);
    tmpStr = CT2A(itsLocalCacheDirectoryPath);
    itsSmartMetDocumentInterface->HelpDataInfoSystem()->SetLocalDataBaseDirectory(tmpStr);
    applicationWinRegistry.UseLedLightStatusSystem(fUseLedLightStatusSystem);

	CDialog::OnOK();
}

void COptionsDlg::OnButtonValidationDialog()
{
	CFmiDataValidationDlg dlg(&itsSmartMetDocumentInterface->MetEditorOptionsData());
	dlg.DoModal();
}


void COptionsDlg::OnBnClickedCheckUseViewMode()
{
	UpdateData(TRUE);
    itsSmartMetDocumentInterface->SmartMetEditingMode(fUseViewMode ? CtrlViewUtils::kFmiEditingModeViewOnly : CtrlViewUtils::kFmiEditingModeNormal, true);
	DisableControls();
	UpdateData(FALSE);
}

// T�m� funktio alustaa kaikki dialogin tekstit editoriin valitulla kielell�.
// T�m� on ik�v� kyll� teht�v� erikseen dialogin muokkaus ty�kalusta, eli
// tekij�n pit�� lis�t� erikseen t�nne kaikki dialogin osat, joihin
// kieli valinta voi vaikuttaa.
void COptionsDlg::InitDialogTexts(void)
{
    SetWindowText(CA2T(::GetDictionaryString("OptionSettingDlgTitle").c_str()));
	CFmiWin32Helpers::SetDialogItemText(this, IDOK, "IDOK");
	CFmiWin32Helpers::SetDialogItemText(this, IDCANCEL, "IDCANCEL");
	CFmiWin32Helpers::SetDialogItemText(this, ID_BUTTON_VALIDATION_DIALOG, "ID_BUTTON_VALIDATION_DIALOG");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_CHECK_STATION_PLOT, "IDC_CHECK_STATION_PLOT");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_CHECK_SHOW_TOOLTIP, "IDC_CHECK_SHOW_TOOLTIP");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_CHECK_USE_SPACINGOUT, "IDC_CHECK_USE_SPACINGOUT");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_CHECK_USE_AUTO_ZOOM, "IDC_CHECK_USE_AUTO_ZOOM");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_CHECK_USE_VIEW_MODE, "IDC_CHECK_USE_VIEW_MODE");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_OPTIONS_CACHE_SIZE_STR, "IDC_STATIC_OPTIONS_CACHE_SIZE_STR");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_OPTIONS_CACHE_SIZE_STR2, "IDC_STATIC_OPTIONS_CACHE_SIZE_STR2");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_UNDO_REDO_1, "IDC_STATIC_UNDO_REDO_1");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_CHECK_USE_AUTO_SAVE, "IDC_CHECK_USE_AUTO_SAVE");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_OPTIONS_ANIMATION_DELAY_STR, "IDC_STATIC_OPTIONS_ANIMATION_DELAY_STR");
    CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_OPTIONS_TIME_STEP_STR, "IDC_STATIC_OPTIONS_TIME_STEP_STR");
    CFmiWin32Helpers::SetDialogItemText(this, IDC_CHECK_DO_AUTO_LOAD_NEW_CACHE_DATA, "Auto load new cache data");
    CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_OPTIONS_LOCATION_FINDER_TIMEOUT_IN_SECONDS, "Location Finder (x key) Timeout [s]");
    CFmiWin32Helpers::SetDialogItemText(this, IDC_CHECK_SHOW_LAST_SEND_TIME_ON_MAP_VIEW, "Show last send time on main map view");
    
    CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_GENERAL_OPTIONS1, "General Options 1");
    CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_GENERAL_OPTIONS2, "General Options 2");
    CFmiWin32Helpers::SetDialogItemText(this, IDC_CHECK_DO_AUTO_LOAD_DATA_AT_STARTUP, "Auto load data at startup");
    CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_OPTIONS_SATEL_DATA_UPDATE_FREQ_IN_MINUTES_STR, "Conceptual-data update freq in minutes");
    CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_OPTIONS_Q2SERVER_GROUP, "Q2 Server");
    CFmiWin32Helpers::SetDialogItemText(this, IDC_CHECK_OPTIONS_Q2SERVER_USE, "Use");
    CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_Q2SERVER_DECIMAL_COUNT, "Decim.");
    CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_Q2SERVER_GRID_SIZE_STR, "Used data grid size (X x Y)");
    CFmiWin32Helpers::SetDialogItemText(this, IDC_CHECK_OPTIONS_Q2SERVER_ZIP, "Zip");
    CFmiWin32Helpers::SetDialogItemText(this, IDC_CHECK_OPTIONS_Q2SERVER_LOG, "Log");
    CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_Q3_URL_STR, "Q3 url");
    CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_OPTIONS_Q2SERVER_GROUP2, "Graphics");
    CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_STATION_DATA_GRID_SIZE_STR, "Station Data Grid Size");
    CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_OPTIONS_DRAW_OBJECT_SCALE_FACTOR_STR, "Draw Object Scale Factor");
    CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_PRINTIN_OPTIONS_STR, "Printing options");
    CFmiWin32Helpers::SetDialogItemText(this, IDC_CHECK_SMART_ORIENTATION_PRINT, "Use smart paper orientation");
    CFmiWin32Helpers::SetDialogItemText(this, IDC_CHECK_FIT_TO_PAPER_PRINT, "Fit to paper printing");
    CFmiWin32Helpers::SetDialogItemText(this, IDC_CHECK_LOW_MEMORY_PRINTING, "Low memory (and cruder image) printing");
    CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_OPTIONS_MAX_PAGE_RANGE_STR, "Max page range in printing");
    CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_LOCAL_CACHE_OPTIONS_GROUP, "Local cache options");
    CFmiWin32Helpers::SetDialogItemText(this, IDC_CHECK_CACHE_USE, "Use local data cache");
    CFmiWin32Helpers::SetDialogItemText(this, IDC_CHECK_CLEAN_CACHE, "Clean local data cache");
    CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_OPTIONS_CACHE_KEEP_FILES_DAYS, "Keep cache files max [days]");
    CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_OPTIONS_CACHE_KEEP_FILES_MAX, "Local cache data file cleaning has changed. Each\ndata has now LC CNT setting in CaseStudy-dlg.");
    CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_SYSTEM_INFO_TO_DB_GROUP, "System info sending to FMI database");
    CFmiWin32Helpers::SetDialogItemText(this, IDC_CHECK_ALLOW_SYSTEM_INFO_SEND_TO_DB, "Allow SmartMet to collect and send system information to the FMI's database for stability studies");
    CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_URL1_STR, "Url");
    CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_URL2_STR, "Url");
    CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_OPTIONS_LOG_GROUP, "Logging");
    CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_OPTIONS_LOG_LEVEL_TEXT, "Log level");
    CFmiWin32Helpers::SetDialogItemText(this, IDC_CHECK_DROPPED_DATA_EDITABLE, "Dropped data editable (slower to drop)");
    CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_ISOLINE_MINIMUM_LENGTH_FACTOR_TEXT, "Isoline min length factor (0-100)");
    CFmiWin32Helpers::SetDialogItemText(this, IDC_CHECK_MAKE_COMBINATION_DATA, "Generate time combination data");
    CFmiWin32Helpers::SetDialogItemText(this, IDC_CHECK_USE_FORCED_LINEAR_INTERPOLATION_OPTION, "Use forced linear interpolation option");
    CFmiWin32Helpers::SetDialogItemText(this, IDC_CHECK_USE_COMBINED_MAP_MODE, "Use combined map mode(local + wms)");
    CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_HATCHING_EPSILON_FACTOR_TEXT, "Hatching calculation epsilon factor (~1)");
    CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_LOCAL_CACHE_DIRECTORY_STR, "SmartMet's local querydata file cache base directory. Change only if you really know what you are doing!");
    CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_RESTART_SMARTMET_STR, "Directory changed, to apply restart SmartMet");
    CFmiWin32Helpers::SetDialogItemText(this, IDC_CHECK_USE_LED_LIGHT_STATUS_SYSTEM, "Use led light status system on statusbar");
}

void COptionsDlg::InitLogLevelComboBox()
{
    CFmiWin32Helpers::InitLogLevelComboBox(itsLogLevelComboBox);
    itsLogLevelComboBox.SetCurSel(static_cast<int>(CatLog::logLevel()));
}

void COptionsDlg::SetLogLevelOnOk()
{
    itsSmartMetDocumentInterface->ApplicationWinRegistry().ConfigurationRelatedWinRegistry().LogLevel(itsLogLevelComboBox.GetCurSel());
    CatLog::logLevel(static_cast<CatLog::Severity>(itsLogLevelComboBox.GetCurSel()));
}

HBRUSH COptionsDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
    HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

    if(pWnd->GetDlgCtrlID() == IDC_STATIC_OPTIONS_CACHE_KEEP_FILES_MAX)
        CFmiWin32Helpers::SetErrorColorForTextControl(pDC, false);
    if(pWnd->GetDlgCtrlID() == IDC_STATIC_RESTART_SMARTMET_STR)
        CFmiWin32Helpers::SetErrorColorForTextControl(pDC, false);
    return hbr;
}

void COptionsDlg::OnBnClickedButtonBrowseLocalCacheDirectory()
{
    CXFolderDialog dlg(itsLocalCacheDirectoryPath);
    dlg.SetTitle(CA2T(::GetDictionaryString("Select folder").c_str()));
    if(dlg.DoModal() == IDOK)
    {
        itsLocalCacheDirectoryPath = dlg.GetPath();
        AdjustRestartTextControl();
        UpdateData(FALSE);
    }
}

void COptionsDlg::OnEnChangeEditLocalCacheDirectory()
{
    UpdateData(TRUE);
    AdjustRestartTextControl();
}

void COptionsDlg::AdjustRestartTextControl()
{
    fLocalCacheDirectoryChanged = itsLocalCacheDirectoryPath != itsLocalCacheDirectoryPathOriginal;
    CFmiWin32Helpers::SetDialogItemVisibility(this, IDC_STATIC_RESTART_SMARTMET_STR, fLocalCacheDirectoryChanged);
    Invalidate();
}
