
// SmartMetDoc.cpp : implementation of the CSmartMetDoc class
//
#ifdef _MSC_VER
#pragma warning(disable : 4786) // poistaa n kpl VC++ kääntäjän varoitusta
#endif

#include "stdafx.h"
#include "resource.h"
#include "SmartMet.h"
#include "SmartMetDoc.h"

#include "MainFrm.h"
#include "NFmiQueryData.h"
#include "ZoomDlg.h"
#include "NFmiEditMapGeneralDataDoc.h"
#include "NFmiDrawParam.h"
#include "NFmiSmartInfo.h"
#include "NFmiGdiPlusImageMapHandler.h"
#include "TimeEditValuesDlg.h"
#include "TimeEditValuesView.h"
#include "SmartMetView.h"
#include "OptionsDlg.h"
#include "NFmiDataLoadingInfo.h"
#include "FmiLoadDataDialog.h"
#include "SaveToDataBaseDlg.h"
#include "NoSaveInfoDlg.h"
#include "DataLoadingProblemsDlg.h"
#include "SaveDataDlg.h"
#include "FmiLocationSelectionToolDlg2.h"
#include "FmiFilterDataDlg.h"
#include "FmiBrushToolDlg.h"
#include "FmiSmartToolDlg.h"
#include "FmiSmarttoolsTabControlDlg.h"
#include "FmiViewGridSelectorDlg.h"
#include "NFmiEditorControlPointManager.h"
#include "FmiDropFileWarningDlg.h"
#include "NFmiMetEditorModeDataWCTR.h"
#include "FmiShortCutsDlg2.h"
#include "NFmiProjectionCurvatureInfo.h"
#include "FmiProjectionLineSetupDlg.h"
#include "FmiGriddingOptionsDlg.h"
#include "FmiTempDlg.h"
#include "FmiTempView.h"
#include "FmiViewMacroDlg.h"
#include "NFmiSettings.h"
#include "NFmiViewSettingMacro.h"
#include "FmiCrossSectionDlg.h"
#include "FmiCrossSectionView.h"
#include "NFmiCrossSectionSystem.h"
#include "FmiSynopPlotSettingsDlg.h"
#include "NFmiObsComparisonInfo.h"
#include "NFmiMetEditorOptionsData.h"
#include "NFmiMTATempSystem.h"
#include "FmiSynopDataGridViewDlg.h"
#include "FmiTrajectoryDlg.h"
#include "NFmiTrajectorySystem.h"
#include "persist2.h"
#include "FmiWarningCenterDlg.h"
#include "NFmiDictionaryFunction.h"
#include "NFmiArea.h"
#include "FmiHelpEditorSettingsDlg.h"
#include "NFmiHelpEditorSystem.h"
#include "NFmiMapViewDescTop.h"
#include "FmiExtraMapViewDlg.h"
#include "FmiExtraMapView.h"
#include "FmiGdiPlusHelpers.h"
#include "FmiSeaIcingWarningsDlg.h"
#include "FmiWindTableDlg.h"
#include "NFmiWindTableSystem.h"
#include "FmiLocationFinderDlg.h"
#include "NFmiAutoComplete.h"
#include "FmiOperationProgressDlg.h"
#include "NFmiDataQualityChecker.h"
#include "FmiDataQualityCheckerDialog.h"
#include "FmiDataLoadingThread2.h"
#include "NFmiHelpDataInfo.h"
#include "FmiCombineDataThread.h"
#include "FmiIgnoreStationsDlg.h"
#include "NFmiIgnoreStationsData.h"
#include "FmiWin32Helpers.h"
#include "FmiCaseStudyDlg.h"
#include "CloneBitmap.h"
#include "NFmiValueString.h"
#include "NFmiApplicationWinRegistry.h"
#include "FmiQueryDataCacheLoaderThread.h"
#include "FmiBetaProductTabControlDialog.h"
#include "FmiWin32TemplateHelpers.h"
#include "FmiLogViewer.h"
#include "NFmiSeaIcingWarningSystem.h"
#include "CtrlViewFunctions.h"
#include "NFmiFastInfoUtils.h"
#include "HakeMessage/Main.h"
#include "HakeMessage/HakeSystemConfigurations.h"
#include "FmiParamAddingDlg.h"
#include "SpecialDesctopIndex.h"
#include "CtrlViewTimeConsumptionReporter.h"
#include "QueryDataReading.h"
#include "SmartMetDocumentInterface.h"
#include "ApplicationInterface.h"
#include "FmiWarningMessageOptionsDlg.h"

#include <direct.h> // _chdir()
#include <cassert>
#include <fstream>
#include "execute-command-in-separate-process.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace std;

// CSmartMetDoc

IMPLEMENT_DYNCREATE(CSmartMetDoc, CDocument)

BEGIN_MESSAGE_MAP(CSmartMetDoc, CDocument)
	ON_COMMAND(ID_FILE_SEND_MAIL, &CSmartMetDoc::OnFileSendMail)
	ON_UPDATE_COMMAND_UI(ID_FILE_SEND_MAIL, &CSmartMetDoc::OnUpdateFileSendMail)

	ON_COMMAND(ID_BUTTON_ZOOM_DIALOG, OnButtonZoomDialog)
	ON_COMMAND(ID_BUTTON_TIME_EDIT_VALUES_DLG, OnButtonTimeEditValuesDlg)
	ON_COMMAND(ID_BUTTON_DATA_AREA, OnButtonDataArea)
	ON_COMMAND(ID_BUTTON_LOAD_DATA, OnButtonLoadData)
	ON_COMMAND(ID_BUTTON_STORE_DATA, OnButtonStoreData)
	ON_COMMAND(ID_MENUITEM_OPTIOT, OnMenuitemOptiot)
	ON_COMMAND(ID_SELECT_ALL, OnSelectAll)
	ON_COMMAND(ID_DESELECT_ALL, OnDeselectAll)
	ON_COMMAND(ID_EDIT_UNDO, OnEditUndo)
	ON_COMMAND(ID_EDIT_REDO, OnEditRedo)
	ON_COMMAND(ID_BUTTON_SELECTION_TOOL_DLG, OnButtonSelectionToolDlg)
	ON_COMMAND(ID_MENUITEM_SAVE_AS, OnMenuitemSaveAs)
	ON_COMMAND(ID_BUTTON_SELECT_EUROPE_MAP, OnButtonSelectEuropeMap)
	ON_COMMAND(ID_BUTTON_SELECT_FINLAND_MAP, OnButtonSelectFinlandMap)
	ON_COMMAND(ID_BUTTON_DATA_TO_DATABASE, OnButtonDataToDatabase)
	ON_COMMAND(ID_DATA_LOAD_FROM_FILE, OnDataLoadFromFile)
	ON_COMMAND(ID_DATA_STORE_TO_FILE, OnDataStoreToFile)
	ON_UPDATE_COMMAND_UI(ID_EDIT_UNDO, OnUpdateEditUndo)
	ON_UPDATE_COMMAND_UI(ID_EDIT_REDO, OnUpdateEditRedo)
	ON_COMMAND(ID_HIDE_PARAM_WINDOW, OnHideParamWindow)
	ON_COMMAND(ID_MAKE_GRID_FILE, OnMakeGridFile)
	ON_COMMAND(ID_BUTTON_MAKE_EDITED_DATA_COPY, OnButtonMakeEditedDataCopy)
	ON_COMMAND(ID_BUTTON_FILTER_DIALOG, OnButtonFilterDialog)
	ON_COMMAND(ID_BUTTON_BRUSH_TOOL_DLG, OnButtonBrushToolDlg)
	ON_UPDATE_COMMAND_UI(ID_BUTTON_BRUSH_TOOL_DLG, OnUpdateButtonBrushToolDlg)
	ON_COMMAND(ID_BUTTON_REFRESH, OnButtonRefresh)
	ON_COMMAND(ID_ACCELERATOR_RELOAD_ALL_DYNAMIC_DATA, OnButtonReloadAllDynamicHelpData)
	ON_COMMAND(ID_MENUITEM_VIEW_GRID_SELECTION_DLG2, OnMenuitemViewGridSelectionDlg2)
	ON_COMMAND(ID_EDIT_SPACE_OUT, OnEditSpaceOut)
	ON_COMMAND(ID_BUTTON_VALIDATE_DATA, OnButtonValidateData)
	ON_UPDATE_COMMAND_UI(ID_BUTTON_VALIDATE_DATA, OnUpdateButtonValidateData)
	ON_COMMAND(ID_CHANGE_MAP_TYPE, OnChangeMapType)
	ON_UPDATE_COMMAND_UI(ID_BUTTON_SELECT_EUROPE_MAP, OnUpdateButtonSelectEuropeMap)
	ON_UPDATE_COMMAND_UI(ID_BUTTON_SELECT_FINLAND_MAP, OnUpdateButtonSelectFinlandMap)
	ON_COMMAND(ID_SHOW_TIME_STRING, OnShowTimeString)
	ON_COMMAND(ID_SHOW_GRID_POINTS, OnShowGridPoints)
	ON_COMMAND(ID_TOGGLE_GRID_POINT_COLOR, OnToggleGridPointColor)
	ON_COMMAND(ID_TOGGLE_GRID_POINT_SIZE, OnToggleGridPointSize)
	ON_COMMAND(ID_TOGGLE_SHOW_NAMES_ON_MAP, OnToggleShowNamesOnMap)
	ON_COMMAND(ID_BUTTON_GLOBE, OnButtonGlobe)
	ON_UPDATE_COMMAND_UI(ID_BUTTON_GLOBE, OnUpdateButtonGlobe)
	ON_COMMAND(ID_SHOW_MASKS_ON_MAP, OnShowMasksOnMap)
	ON_COMMAND(ID_VIEW_SET_TIME_EDITOR_PLACE_TO_DEFAULT, OnViewSetTimeEditorPlaceToDefault)
	ON_COMMAND(ID_FILE_OPEN, OnFileOpen)
	ON_COMMAND(ID_FILE_NEW, OnFileNew)
	ON_COMMAND(ID_BUTTON_ANIMATION, OnButtonAnimation)
	ON_UPDATE_COMMAND_UI(ID_BUTTON_ANIMATION, OnUpdateButtonAnimation)
	ON_COMMAND(ID_BUTTON_EDITOR_CONTROL_POINT_MODE, OnButtonEditorControlPointMode)
	ON_UPDATE_COMMAND_UI(ID_BUTTON_EDITOR_CONTROL_POINT_MODE, OnUpdateButtonEditorControlPointMode)
	ON_COMMAND(ID_BUTTON_DELETE, OnButtonDelete)
	ON_COMMAND(ID_DATA_STORE_VIEW_TO_PICTURE_FILE, OnDataStoreViewToPictureFile)
	ON_UPDATE_COMMAND_UI(ID_DATA_STORE_VIEW_TO_PICTURE_FILE, OnUpdateDataStoreViewToPictureFile)
	ON_COMMAND(ID_DATA_STORE_MAP_VIEW2_TO_PICTURE_FILE, OnDataStoreMapView2ToPictureFile)
	ON_COMMAND(ID_DATA_STORE_MAP_VIEW3_TO_PICTURE_FILE, OnDataStoreMapView3ToPictureFile)
	ON_COMMAND(ID_HELP_SHORT_CUTS, OnHelpShortCuts)
	ON_COMMAND(ID_SHOW_PROJECTION_LINES, OnShowProjectionLines)
	ON_COMMAND(ID_MENUITEM_PROJECTION_LINE_SETUP, OnMenuitemProjectionLineSetup)
	ON_COMMAND(ID_BUTTON_VIEW_SELECT_PARAM_DIALOG, OnButtonViewSelectParamDialog)
	ON_COMMAND(ID_DATA_STORE_TIMESERIALVIEW_TO_PICTURE_FILE, OnDataStoreTimeserialviewToPictureFile)
	ON_COMMAND(ID_DATA_STORE_TRAJEKTORYVIEW_TO_PICTURE_FILE, OnDataStoreTrajectoryViewToPictureFile)
	ON_COMMAND(ID_MENUITEM_GRIDDING_OPTIONS, OnMenuitemGriddingOptions)
	ON_COMMAND(ID_BUTTON_OPEN_SMART_TOOL_DLG, OnButtonOpenSmartToolDlg)
	ON_UPDATE_COMMAND_UI(ID_BUTTON_FILTER_DIALOG, OnUpdateButtonFilterDialog)
	ON_COMMAND(ID_BUTTON_TEMP_DLG, OnButtonTempDlg)
	ON_COMMAND(ID_BUTTON_SHOW_CROSS_SECTION, OnButtonShowCrossSection)
	ON_COMMAND(ID_DATA_STORE_TEMPVIEW_TO_PICTURE_FILE, OnDataStoreTempviewToPictureFile)
	ON_COMMAND(ID_TOGGLE_LAND_BORDER_DRAW_COLOR, OnToggleLandBorderDrawColor)
	ON_COMMAND(ID_TOGGLE_LAND_BORDER_PEN_SIZE, OnToggleLandBorderPenSize)
	ON_COMMAND(ID_ACCELERATOR_STORE_VIEW_MACRO, OnAcceleratorStoreViewMacro)

	ON_COMMAND(ID_ACCELERATOR_BORROW_PARAMS_1, OnAcceleratorBorrowParams1)
	ON_COMMAND(ID_ACCELERATOR_BORROW_PARAMS_2, OnAcceleratorBorrowParams2)
	ON_COMMAND(ID_ACCELERATOR_BORROW_PARAMS_3, OnAcceleratorBorrowParams3)
	ON_COMMAND(ID_ACCELERATOR_BORROW_PARAMS_4, OnAcceleratorBorrowParams4)
	ON_COMMAND(ID_ACCELERATOR_BORROW_PARAMS_5, OnAcceleratorBorrowParams5)

	ON_UPDATE_COMMAND_UI(ID_BUTTON_DATA_TO_DATABASE, OnUpdateButtonDataToDatabase)
	ON_UPDATE_COMMAND_UI(ID_BUTTON_STORE_DATA, OnUpdateButtonStoreData)
	ON_UPDATE_COMMAND_UI(ID_BUTTON_SELECTION_TOOL_DLG, OnUpdateButtonSelectionToolDlg)
	ON_UPDATE_COMMAND_UI(ID_SELECT_ALL, OnUpdateSelectAll)
	ON_UPDATE_COMMAND_UI(ID_MENUITEM_SAVE_AS, OnUpdateMenuitemSaveAs)
	ON_UPDATE_COMMAND_UI(ID_MENUITEM_GRIDDING_OPTIONS, OnUpdateMenuitemGriddingOptions)
	ON_UPDATE_COMMAND_UI(ID_DESELECT_ALL, OnUpdateDeselectAll)
	ON_UPDATE_COMMAND_UI(ID_DATA_STORE_TO_FILE, OnUpdateDataStoreToFile)
	ON_UPDATE_COMMAND_UI(ID_BUTTON_MAKE_EDITED_DATA_COPY, OnUpdateButtonMakeEditedDataCopy)
	ON_UPDATE_COMMAND_UI(ID_MAKE_GRID_FILE, OnUpdateMakeGridFile)
	ON_UPDATE_COMMAND_UI(ID_EDIT_PASTE, OnUpdateEditPaste)
	ON_UPDATE_COMMAND_UI(ID_EDIT_COPY, OnUpdateEditCopy)
	ON_COMMAND(ID_ACCELERATOR_CROSS_SECTION_MODE, OnAcceleratorCrossSectionMode)
	ON_COMMAND(ID_BUTTON_SELECT_SCANDINAVIA_MAP, OnButtonSelectScandinaviaMap)
	ON_UPDATE_COMMAND_UI(ID_BUTTON_SELECT_SCANDINAVIA_MAP, OnUpdateButtonSelectScandinaviaMap)
	ON_COMMAND(ID_ACCELERATOR_MAP_ROW_1, OnAcceleratorMapRow1)
	ON_COMMAND(ID_ACCELERATOR_MAP_ROW_2, OnAcceleratorMapRow2)
	ON_COMMAND(ID_ACCELERATOR_MAP_ROW_3, OnAcceleratorMapRow3)
	ON_COMMAND(ID_ACCELERATOR_MAP_ROW_4, OnAcceleratorMapRow4)
	ON_COMMAND(ID_ACCELERATOR_MAP_ROW_5, OnAcceleratorMapRow5)
    ON_COMMAND(ID_ACCELERATOR_MAP_ROW_6, OnAcceleratorMapRow6)
    ON_COMMAND(ID_ACCELERATOR_MAP_ROW_7, OnAcceleratorMapRow7)
    ON_COMMAND(ID_ACCELERATOR_MAP_ROW_8, OnAcceleratorMapRow8)
    ON_COMMAND(ID_ACCELERATOR_MAP_ROW_9, OnAcceleratorMapRow9)
    ON_COMMAND(ID_ACCELERATOR_MAP_ROW_10, OnAcceleratorMapRow10)
    ON_COMMAND(ID_BUTTON_SHOW_SYNOP_PLOT_SETTINGS, OnButtonShowSynopPlotSettings)
    ON_COMMAND(ID_MUOKKAA_MAKROPOLKUASETUKSET, OnMuokkaaMakropolkuasetukset)
	ON_COMMAND(ID_BUTTON_OBSERVATION_COMPARISON_MODE, OnButtonObservationComparisonMode)
	ON_UPDATE_COMMAND_UI(ID_BUTTON_OBSERVATION_COMPARISON_MODE, OnUpdateButtonObservationComparisonMode)
	ON_COMMAND(ID_BUTTON_SYNOP_DATA_GRID_VIEW, OnButtonSynopDataGridView)
	ON_UPDATE_COMMAND_UI(ID_BUTTON_SYNOP_DATA_GRID_VIEW, OnUpdateSynopDataGridView)
	ON_COMMAND(ID_ACCELERATOR_OBS_COMPARISON_CHANGE_SYMBOL, OnAcceleratorObsComparisonChangeSymbol)
	ON_COMMAND(ID_ACCELERATOR_OBS_COMPARISON_CHANGE_SYMBOL_SIZE, OnAcceleratorObsComparisonChangeSymbolSize)
	ON_COMMAND(ID_ACCELERATOR_OBS_COMPARISON_TOGGLE_BORDER_DRAW, OnAcceleratorObsComparisonToggleBorderDraw)
	ON_UPDATE_COMMAND_UI(ID_BUTTON_TEMP_DLG, OnUpdateButtonTempDlg)
	ON_UPDATE_COMMAND_UI(ID_BUTTON_SHOW_CROSS_SECTION, OnUpdateButtonShowCrossSection)
	ON_UPDATE_COMMAND_UI(ID_BUTTON_TIME_EDIT_VALUES_DLG, OnUpdateButtonTimeEditValuesDlg)
	ON_COMMAND(ID_DATA_STORE_CROSSSECTIONVIEW_TO_PICTURE_FILE, OnDataStoreCrosssectionviewToPictureFile)
	ON_COMMAND(ID_VIEW_SET_MAP_VIEW2_PLACE_TO_DEFAULT, OnViewSetMapView2PlaceToDefault)
	ON_COMMAND(ID_VIEW_SET_MAP_VIEW3_PLACE_TO_DEFAULT, OnViewSetMapView3PlaceToDefault)
	ON_COMMAND(ID_VIEW_SET_TEMP_VIEW_PLACE_TO_DEFAULT, OnViewSetTempViewPlaceToDefault)
	ON_COMMAND(ID_VIEW_SET_CROSSSECTION_VIEW_PLACE_TO_DEFAULT, OnViewSetCrosssectionViewPlaceToDefault)
	ON_COMMAND(ID_VIEW_SET_ZOOM_VIEW_PLACE_TO_DEFAULT, OnViewSetZoomViewPlaceToDefault)
	ON_COMMAND(ID_VIEW_SET_VIEW_MACRO_DLG_PLACE_TO_DEFAULT, OnViewSetViewMacroDlgPlaceToDefault)
	ON_COMMAND(ID_VIEW_SET_SYNOP_TEXT_VIEW_PLACE_TO_DEFAULT, OnViewSetSynopTextViewPlaceToDefault)
	ON_COMMAND(ID_ACCELERATOR_TOGGLE_HELP_CURSOR_ON_MAP, OnAcceleratorToggleHelpCursorOnMap)
	ON_COMMAND(ID_OHJE_VIRHETESTI, OnOhjeVirhetesti)
	ON_COMMAND(ID_MENUITEM_MUOKKAA_KIELI, OnMenuitemMuokkaaKieli)
	ON_COMMAND(ID_BUTTON_TRAJECTORY, OnButtonTrajectory)
	ON_UPDATE_COMMAND_UI(ID_BUTTON_TRAJECTORY, OnUpdateButtonTrajectory)
	ON_COMMAND(ID_VIEW_SET_TRAJECTORY_VIEW_PLACE_TO_DEFAULT, OnViewSetTrajectoryViewPlaceToDefault)
	ON_COMMAND(ID_VIEW_EDITOR_LOG, &CSmartMetDoc::OnViewEditorLog)
	ON_COMMAND(ID_BUTTON_WARNING_CENTER_DLG, OnButtonWarningCenterDlg)
	ON_COMMAND(ID_BUTTON_SEA_ICING_WARNINGS_DLG, OnButtonSeaIcingWarningsDlg)
	ON_COMMAND(ID_ACCELERATOR_TOGGLE_OVERMAP_FORE_BACK_GROUND, OnToggleOverMapBackForeGround)
	ON_COMMAND(ID_ACCELERATOR_TOGGLE_TOOLTIP, OnAcceleratorToggleTooltip)
	ON_COMMAND(ID_ACCELERATOR_TOGGLE_KEEP_MAP_RATIO, OnAcceleratorToggleKeepMapRatio)
	ON_COMMAND(ID_MENUITEM_HELP_EDITOR_SETTINGS, OnMenuitemHelpEditorModeSettings)
	ON_COMMAND(ID_BUTTON_HELP_EDITOR_MODE, OnButtonHelpEditorMode)
	ON_UPDATE_COMMAND_UI(ID_BUTTON_HELP_EDITOR_MODE, OnUpdateButtonHelpEditorMode)
	ON_COMMAND(ID_VIEW_SET_WARNING_CENTER_DLG_PLACE_TO_DEFAULT, OnViewSetWarningCenterDlgPlaceToDefault)
	ON_COMMAND(ID_VIEW_SET_SEA_ICING_WARNINGS_DLG_PLACE_TO_DEFAULT, OnViewSetSeaIcingWarningsDlgPlaceToDefault)
	ON_COMMAND(ID_EXTRA_MAP_VIEW_1, OnExtraMapView1)
	ON_UPDATE_COMMAND_UI(ID_EXTRA_MAP_VIEW_1, OnUpdateOnExtraMapView1)
	ON_COMMAND(ID_EXTRA_MAP_VIEW_2, OnExtraMapView2)
	ON_UPDATE_COMMAND_UI(ID_EXTRA_MAP_VIEW_2, OnUpdateOnExtraMapView2)
	ON_COMMAND(ID_BUTTON_WIND_TABLE_DLG, &CSmartMetDoc::OnButtonWindTableDlg)
	ON_COMMAND(ID_VIEW_SET_WIND_TABLE_DLG_PLACE_TO_DEFAULT, &CSmartMetDoc::OnViewSetWindTableDlgPlaceToDefault)
	ON_COMMAND(ID_CHANGE_PREVIOUS_MAP_TYPE, &CSmartMetDoc::OnChangePreviousMapType)
	ON_COMMAND(ID_TOGGLE_SHOW_PREVIOUS_NAMES_ON_MAP, &CSmartMetDoc::OnToggleShowPreviousNamesOnMap)
	ON_COMMAND(ID_ACCELERATOR_LOCATION_FINDER_TOOL, &CSmartMetDoc::OnAcceleratorLocationFinderTool)
	ON_COMMAND(ID_BUTTON_DATA_QUALITY_CHECKER, &CSmartMetDoc::OnButtonDataQualityChecker)
	ON_COMMAND(ID_VIEW_SET_DATA_QUALITY_CHECKER_DLG_PLACE_TO_DEFAULT, &CSmartMetDoc::OnViewSetDataQualityCheckerDlgPlaceToDefault)
	ON_UPDATE_COMMAND_UI(ID_BUTTON_DATA_QUALITY_CHECKER, &CSmartMetDoc::OnUpdateButtonDataQualityChecker)
	ON_COMMAND(ID_ACCELERATOR_IGNORE_STATIONS_DLG, &CSmartMetDoc::OnAcceleratorIgnoreStationsDlg)
	ON_COMMAND(ID_ACCELERATOR_DO_MACRO_DIRECTORIES_SYNCRONIZATION, &CSmartMetDoc::OnAcceleratorDoMacroDirectoriesSyncronization)
	ON_COMMAND(ID_VIEW_CASE_STUDY_DIALOG, &CSmartMetDoc::OnViewCaseStudyDialog)
    ON_COMMAND(ID_ACCELERATOR_AREA_INFO_TO_CLIPBOARD, &CSmartMetDoc::OnAcceleratorAreaInfoToClipboard)
    ON_COMMAND(ID_VIEW_SET_SMARTTOOL_VIEW_PLACE_TO_DEFAULT, &CSmartMetDoc::OnViewSetSmarttoolViewPlaceToDefault)
    ON_COMMAND(ID_TOOLTIPSTATUS_CROSSSECTIONVIEW, &CSmartMetDoc::OnTooltipstatusCrosssectionview)
    ON_UPDATE_COMMAND_UI(ID_TOOLTIPSTATUS_CROSSSECTIONVIEW, &CSmartMetDoc::OnUpdateTooltipstatusCrosssectionview)
    ON_COMMAND(ID_TOOLTIPSTATUS_MAPVIEWS, &CSmartMetDoc::OnTooltipstatusMapviews)
    ON_UPDATE_COMMAND_UI(ID_TOOLTIPSTATUS_MAPVIEWS, &CSmartMetDoc::OnUpdateTooltipstatusMapviews)
    ON_COMMAND(ID_TOOLTIPSTATUS_SOUNDINGVIEW, &CSmartMetDoc::OnTooltipstatusSoundingview)
    ON_UPDATE_COMMAND_UI(ID_TOOLTIPSTATUS_SOUNDINGVIEW, &CSmartMetDoc::OnUpdateTooltipstatusSoundingview)
    ON_COMMAND(ID_TOOLTIPSTATUS_TIMESERIALVIEW, &CSmartMetDoc::OnTooltipstatusTimeserialview)
    ON_UPDATE_COMMAND_UI(ID_TOOLTIPSTATUS_TIMESERIALVIEW, &CSmartMetDoc::OnUpdateTooltipstatusTimeserialview)
    ON_COMMAND(ID_ACCELERATOR_APPLY_BACKUP_VIEW_MACRO, &CSmartMetDoc::OnAcceleratorApplyBackupViewMacro)
    ON_COMMAND(ID_ACCELERATOR_APPLY_CRASH_BACKUP_VIEW_MACRO, &CSmartMetDoc::OnAcceleratorApplyCrashBackupViewMacro)
    ON_COMMAND(ID_RELOADDATA_RELOADFAILEDSATELLITEIMAGES, &CSmartMetDoc::OnReloaddataReloadfailedsatelliteimages)
    ON_COMMAND(ID_ACCELERATOR_DEVELOPED_FUNCTION_TESTING, &CSmartMetDoc::OnAcceleratorDevelopedFunctionTesting)
    ON_COMMAND(ID_VIEW_BETAPRODUCTION, &CSmartMetDoc::OnViewBetaproduction)
    ON_COMMAND(ID_ACCELERATOR_RESET_TIME_FILTER_TIMES, &CSmartMetDoc::OnAcceleratorResetTimeFilterTimes)
    ON_COMMAND(ID_ACCELERATOR_LOG_VIEWER, &CSmartMetDoc::OnAcceleratorLogViewer)
    ON_COMMAND(ID_ACCELERATOR_CALCULATIONPOINT_INFO_TO_CLIPBOARD, &CSmartMetDoc::OnAcceleratorCalculationpointInfoToClipboard)
    ON_COMMAND(ID_ACCELERATOR_DATA_FILEPATHS_TO_CLIPBOARD, &CSmartMetDoc::OnAcceleratorDataFilepathsToClipboard)
    ON_COMMAND(ID_ACCELERATOR_MOVE_MANY_MAP_ROWS_UP, &CSmartMetDoc::OnAcceleratorMoveManyMapRowsUp)
    ON_COMMAND(ID_ACCELERATOR_MOVE_MANY_MAP_ROWS_DOWN, &CSmartMetDoc::OnAcceleratorMoveManyMapRowsDown)
    ON_COMMAND(ID_ACCELERATOR_APPLY_STARTUP_VIEW_MACRO, &CSmartMetDoc::OnAcceleratorApplyStartupViewMacro)
    ON_COMMAND(ID_ACCELERATOR_TOGGLE_WMS_MAP_MODE, &CSmartMetDoc::OnAcceleratorToggleWmsMapMode)
    ON_COMMAND(ID_VIEW_SET_PARAM_SELECTION_VIEW_PLACE_TO_DEFAULT, OnSetParamAddingDlgPlaceToDefault)
    END_MESSAGE_MAP()

BEGIN_DISPATCH_MAP(CSmartMetDoc, CDocument)
END_DISPATCH_MAP()

// Note: we add support for IID_ISmartMet to support typesafe binding
//  from VBA.  This IID must match the GUID that is attached to the
//  dispinterface in the .IDL file.

// {CDDC9574-B255-4A7C-A69C-FCD8AE5E8166}
static const IID IID_ISmartMet =
{ 0xCDDC9574, 0xB255, 0x4A7C, { 0xA6, 0x9C, 0xFC, 0xD8, 0xAE, 0x5E, 0x81, 0x66 } };

BEGIN_INTERFACE_MAP(CSmartMetDoc, CDocument)
	INTERFACE_PART(CSmartMetDoc, IID_ISmartMet, Dispatch)
END_INTERFACE_MAP()


// CSmartMetDoc construction/destruction

CSmartMetDoc::CSmartMetDoc()
:itsData(nullptr)
,itsTimeSerialDataEditorDlg(nullptr)
,itsLocationSelectionToolDlg(nullptr)
,itsZoomDlg(nullptr)
,itsFilterDlg(nullptr)
,itsBrushToolDlg(nullptr)
,itsLocationFinderDlg(nullptr)
,itsSmartToolDlg(nullptr)
,itsSmarttoolsTabControlDlg(nullptr)
,itsTempDialog(nullptr)
,itsShortCutsDialog(nullptr)
,itsViewMacroDlg(nullptr)
,itsCrossSectionDlg(nullptr)
,itsSynopPlotSettingsDlg(nullptr)
,itsSynopDataGridViewDlg(nullptr)
,itsTrajectoryDlg(nullptr)
,itsWarningCenterDlg(nullptr)
,itsSeaIcingWarningsDlg(nullptr)
,itsWindTableDlg(nullptr)
,itsDataQualityCheckerDialog(nullptr)
,itsMapViewDescTopIndex(0)
,itsExtraMapViewDlg1(nullptr)
,itsExtraMapViewDlg2(nullptr)
,itsIgnoreStationsDlg(nullptr)
,itsCaseStudyDlg(nullptr)
,itsBetaProductDialog(nullptr)
,itsLogViewer(nullptr)
,itsParamAddingDlg(nullptr)
{
	CSmartMetApp *app = (CSmartMetApp *)AfxGetApp();
	itsData = app->GeneralDocData();

    // Pitää luoda nämä ikkunant heti kun SmartMet aukeaa. Törmäsin ongelmiin, kun poistin luonnin tästä,
    // että satunnaisen oloisesti eri näyttömakrojen lataus saattoi sotkea eri näyttöjen ikkuna koot ja paikat,
    // niin että (satunnaiset?) näytöt yrittivät ottaa yhden ja saman CRect olion arvon itselleen.
    CreateTimeEditor(true);
    CreateLocationSelectionToolDialog(itsData);
    CreateFilterDlg(itsData);
    CreateTempDlg();
    CreateShortCutsDlg();
    CreateCrossSectionDlg();
    CreateSynopDataGridViewDlg(itsData);
    CreateTrajectoryDlg(itsData);
    CreateWarningCenterDlg(itsData);
    itsExtraMapViewDlg1 = CreateExtraMapViewDlg(itsData, 1);
    itsExtraMapViewDlg2 = CreateExtraMapViewDlg(itsData, 2);

	EnableAutomation();
	AfxOleLockApp();
}

static void DestroyModalessDialog(CDialog **theDlg) // annetaan pointterin pointteri sisään, että pointteri voidaan lopuksi nollata
{
	if((*theDlg) != 0)
	{
		// tällä DestroyWindow-jutulla ei tainnut olla vaikutusta, mutta vielä modaalittomat dialogit vuotavat jotain sanoo deleaker-ohjelma
//		(*theDlg)->DestroyWindow(); // tämä tuhoaa modaless dialogin sisukset
		delete (*theDlg); // tämä tuhoaa sen kuoret
		(*theDlg) = 0; // tässä vielä asetetaan kyseisen dialogin pointteri 0:ksi
	}
}

CSmartMetDoc::~CSmartMetDoc()
{
	AfxOleUnlockApp();

	::DestroyModalessDialog((CDialog **)(&itsZoomDlg));
	::DestroyModalessDialog((CDialog **)(&itsTimeSerialDataEditorDlg));
	::DestroyModalessDialog((CDialog **)(&itsLocationSelectionToolDlg));
	::DestroyModalessDialog((CDialog **)(&itsFilterDlg));
	::DestroyModalessDialog((CDialog **)(&itsBrushToolDlg));
	::DestroyModalessDialog((CDialog **)(&itsLocationFinderDlg));
	::DestroyModalessDialog((CDialog **)(&itsSmartToolDlg));
    ::DestroyModalessDialog((CDialog **)(&itsSmarttoolsTabControlDlg));
	::DestroyModalessDialog((CDialog **)(&itsTempDialog));
	::DestroyModalessDialog((CDialog **)(&itsShortCutsDialog));
	::DestroyModalessDialog((CDialog **)(&itsViewMacroDlg));
	::DestroyModalessDialog((CDialog **)(&itsCrossSectionDlg));
	::DestroyModalessDialog((CDialog **)(&itsSynopPlotSettingsDlg));
	::DestroyModalessDialog((CDialog **)(&itsSynopDataGridViewDlg));
	::DestroyModalessDialog((CDialog **)(&itsTrajectoryDlg));
	::DestroyModalessDialog((CDialog **)(&itsWarningCenterDlg));
	::DestroyModalessDialog((CDialog **)(&itsSeaIcingWarningsDlg));
	::DestroyModalessDialog((CDialog **)(&itsWindTableDlg));
	::DestroyModalessDialog((CDialog **)(&itsDataQualityCheckerDialog));
	::DestroyModalessDialog((CDialog **)(&itsExtraMapViewDlg1));
	::DestroyModalessDialog((CDialog **)(&itsExtraMapViewDlg2));
	::DestroyModalessDialog((CDialog **)(&itsIgnoreStationsDlg));
    ::DestroyModalessDialog((CDialog **)(&itsCaseStudyDlg));
    ::DestroyModalessDialog((CDialog **)(&itsBetaProductDialog));

    // Better clear callback function before logviewer is destroyed, just in case
    CatLog::setLogViewerUpdateCallback(std::function<void()>());
    ::DestroyModalessDialog((CDialog **)(&itsLogViewer));
    ::DestroyModalessDialog((CDialog **)(&itsParamAddingDlg));
}

BOOL CSmartMetDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: add reinitialization code here
	// (SDI documents will reuse this document)
/*
	// HUOM! OTA POIS TÄMÄ ALKU TESTIEN JÄLKEEN!!!!!!
	CFmiImageFromWeb imageFromWeb;
	imageFromWeb.GetImage();
	// HUOM! OTA POIS TÄMÄ ALKU TESTIEN JÄLKEEN!!!!!!
*/
	return TRUE;
}




// CSmartMetDoc serialization

void CSmartMetDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: add storing code here
	}
	else
	{
		// TODO: add loading code here
	}
}


// CSmartMetDoc diagnostics

#ifdef _DEBUG
void CSmartMetDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CSmartMetDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG


// CSmartMetDoc commands

NFmiEditMapGeneralDataDoc* CSmartMetDoc::GetData()
{
	return itsData;
}

void CSmartMetDoc::OnButtonZoomDialog()
{
	ActivateZoomDialog(itsMapViewDescTopIndex);
	GetData()->LogMessage("Opening Zoom dialog.", CatLog::Severity::Info, CatLog::Category::Operational);
}

void CSmartMetDoc::ActivateZoomDialog(int theWantedDescTopIndex)
{
	if(!itsZoomDlg)
		CreateZoomDlg(itsData);
	if(itsZoomDlg)
	{
		itsZoomDlg->ShowWindow(SW_RESTORE );	// Vaihdoin SW_RESTOREN, muistaa ikkunan muutetun koon.
		itsZoomDlg->ActivateMapViewDescTop(theWantedDescTopIndex);
	}
}

void CSmartMetDoc::OnButtonTimeEditValuesDlg()
{
	if(itsData)
	{
		if(!itsTimeSerialDataEditorDlg)
			CreateTimeEditor(true);

		if(itsData->TimeSerialDataViewOn())
		{ // laitetaan aikasarjaikkuna kiinni
			itsTimeSerialDataEditorDlg->ShowWindow(SW_HIDE);
			GetData()->LogMessage("Closing time serial view.", CatLog::Severity::Info, CatLog::Category::Operational);
		}
		else
		{ // laitetaan aikasarja ikkuna päälle
			itsTimeSerialDataEditorDlg->ShowWindow(SW_SHOW);	// Vaihdoin SW_RESTOREN, muistaa ikkunan muutetun koon.
			itsTimeSerialDataEditorDlg->SetActiveWindow();
			itsTimeSerialDataEditorDlg->Update();
			GetData()->LogMessage("Opening time serial view.", CatLog::Severity::Info, CatLog::Category::Operational);
		}
		itsData->TimeSerialDataViewOn(!itsData->TimeSerialDataViewOn());
	}
}

void CSmartMetDoc::OnUpdateButtonTimeEditValuesDlg(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(itsData->TimeSerialDataViewOn());
}

void CSmartMetDoc::ActivateParamAddingDlg()
{
    if(!itsParamAddingDlg)
        CreateParamAddingDlg(itsData);

    itsParamAddingDlg->ShowWindow(SW_SHOW);	// Vaihdoin SW_RESTOREN, muistaa ikkunan muutetun koon.
    itsParamAddingDlg->SetActiveWindow();

    GetData()->LogMessage("Param adding dialog on", CatLog::Severity::Info, CatLog::Category::Operational);
}


// Jos datan area ja kartta ovat "samanlaisia", laitetaan zoomiksi editoiavan datan alue
// muuten laitetaan kurrentti kartta kokonaisuudessaan näkyviin.
void CSmartMetDoc::OnButtonDataArea()
{
	GetData()->OnButtonDataArea(itsMapViewDescTopIndex);
}

void CSmartMetDoc::LoadDataOnStartUp(void)
{
    CtrlViewUtils::FmiSmartMetEditingMode oldMode = GetData()->SmartMetEditingMode();
    // asetetaan alkulatauksen ajaksi sen oma moodi päälle
	GetData()->SmartMetEditingMode(CtrlViewUtils::kFmiEditingModeStartUpLoading, false); // ei muutetä asetusta setting:eihin asti (false), koska tämä on vain väliaikainen muutos, lopussa palautetaan arvo takaisin
	try
	{
		OnButtonLoadData();
	}
	catch(std::exception &e)
	{
		std::string errStr("Error while doing CSmartMetDoc::LoadDataOnStartUp:\n");
		errStr += e.what();
		GetData()->LogAndWarnUser(errStr, "Error while loading data at startup", CatLog::Severity::Error, CatLog::Category::Data, false);
	}
	catch(...)
	{
		std::string errStr("Unknown error while doing CSmartMetDoc::LoadDataOnStartUp");
		GetData()->LogAndWarnUser(errStr, "Error while loading data at startup", CatLog::Severity::Error, CatLog::Category::Data, false);
	}
	GetData()->SmartMetEditingMode(oldMode, false);
}

void CSmartMetDoc::PostMessageToDialog(SmartMetViewId theDestDlg, UINT theMessage)
{
	switch(theDestDlg)
	{
    case SmartMetViewId::TimeSerialView:
		{
			if(itsTimeSerialDataEditorDlg)
				itsTimeSerialDataEditorDlg->PostMessage(theMessage);
			break;
		}
    case SmartMetViewId::SmartToolDlg:
		{
			if(itsSmartToolDlg)
				itsSmartToolDlg->PostMessage(theMessage);
			break;
		}
	default:
		{
			break;
		}
	}
}

void CSmartMetDoc::OnButtonLoadData()
{
	BOOL stat = PrepareForLoadingDataFromFile();

	CFmiLoadDataDialog dialog(SmartMetDocumentInterface::GetSmartMetDocumentInterfaceImplementation());
	bool fWasOnNormalMode = !GetData()->IsOperationalModeOn();
	if(fWasOnNormalMode)
	{ // Ikävää kikka vitoislua, tällä estetään kun ollaan normaali moodissa, ja vielä lataus dialogissa
		// ettei SmartMet mene laittamaan datan lataus threadeja päälle.
		GetData()->EditorModeDataWCTR()->InNormalModeStillInDataLoadDialog(true);
	}

	bool oldValue = GetData()->EditorModeDataWCTR()->UseNormalModeForAWhile();
	GetData()->EditorModeDataWCTR()->UseNormalModeForAWhile(false); // pitää laittaa varmuuden vuoksi pois päältä latauksen ajaksi
	bool isViewMode = (GetData()->SmartMetEditingMode() != CtrlViewUtils::kFmiEditingModeNormal);
	if(isViewMode || dialog.DoModal() == IDOK) // jos ollaan ns. view-moodissa, ei dialogia avata vaan mennaan oletus arvoilla eteenpain
	{
		GetData()->EditorModeDataWCTR()->InNormalModeStillInDataLoadDialog(false);
		CWaitCursor cursor;
		SetCursor(AfxGetApp()->LoadStandardCursor(IDC_WAIT)); // älä poista, muuten ei tule jostain syystä tiimalasia!?!?!?
		if(GetData())
			GetData()->LoadData(dialog.RemoveThundersOnLoad());
		SetPathName(_TEXT("_"));
		((CMainFrame*)AfxGetMainWnd())->StartDataLoadingWorkingThread(); // edellä load datassa ladattiin vain itse data, tässä käynnistetään apudatan lataus heti
		UpdateAllViewsAndDialogs(std::string(__FUNCTION__) + ": edited data loaded");
	}
	else
	{
		GetData()->EditorModeDataWCTR()->InNormalModeStillInDataLoadDialog(false);
		GetData()->EditorModeDataWCTR()->UseNormalModeForAWhile(oldValue); // jos cancelilla ulos, voidaan laittaa vanha moodi päälle takaisin

		if(fWasOnNormalMode)
		{
			// TÄMÄ pikaviritys palauttaa vain timebag/editormode sekoilut paikallee, jos on ollut
			// normaalisti dataa ladattuna ja painaa Lataa-nappulaa ja painaa Peruuta-nappia
			boost::shared_ptr<NFmiFastQueryInfo> smart = GetData()->EditedSmartInfo();
			if(smart)
			{
				string dataFileName(smart->DataFileName());
				string dataFilePattern(smart->DataFilePattern());
				GetData()->AddQueryData(smart->RefQueryData()->Clone(), dataFileName, dataFilePattern, NFmiInfoData::kEditable, "", dynamic_cast<NFmiSmartInfo*>(smart.get())->LoadedFromFile());
			}
			UpdateAllViewsAndDialogs(std::string(__FUNCTION__) + ": loading edited data canceled"); // näyttöä pitää päivittää, muuten ohjelma kaatuu kun drawparamit on tuhottu alta ja näyttöjä ei ole päivitetty oikein
		}
	}
}

BOOL CSmartMetDoc::PrepareForLoadingDataFromFile(void)
{
	BOOL status = CheckEditedDataAndStoreIfNeeded();
	return status;
}

BOOL CSmartMetDoc::CheckEditedDataAndStoreIfNeeded(void)
{
	boost::shared_ptr<NFmiFastQueryInfo> smart = GetData()->EditedSmartInfo();
	BOOL status = TRUE;
	if(smart != 0 && dynamic_cast<NFmiSmartInfo*>(smart.get())->IsDirty()) //jos on dataa jo ladattuna ja sitä on muutettu, talletetaan ensin
	{
		status = StoreEditedData();
	}
	return status;
}

BOOL CSmartMetDoc::StoreEditedData(void)
{
	boost::shared_ptr<NFmiFastQueryInfo> smart = GetData()->EditedSmartInfo();
	BOOL status = TRUE;
	if(smart)
	{
		if(dynamic_cast<NFmiSmartInfo*>(smart.get())->LoadedFromFile())
			status = StoreData();
		else
			status = StoreWorkingData(smart);
	}
	return status;
}

void CSmartMetDoc::OnButtonStoreData()
{
	GetData()->LogMessage("Saving edited data.", CatLog::Severity::Info, CatLog::Category::Editing);
	StoreEditedData();
}

BOOL CSmartMetDoc::StoreWorkingData(boost::shared_ptr<NFmiFastQueryInfo> &smart, bool askForSave)
{
	NFmiDataLoadingInfo& info = GetData()->GetUsedDataLoadingInfo();
	info.InitFileNameLists();
	NFmiString fileNameWithPath(info.CreateWorkingFileName(info.LatestWorkingVersion()+1));
	return StoreData(fileNameWithPath, smart, askForSave);
}

void CSmartMetDoc::DoAutoSave(void)
{
	GetData()->DoAutoSaveData();
}

BOOL CSmartMetDoc::StoreDataBaseDataMarko(boost::shared_ptr<NFmiFastQueryInfo> &smart)
{
	((CFrameWnd*)AfxGetMainWnd())->SetMessageText(_TEXT("Sending data to database."));

	CSaveToDataBaseDlg dlg(SmartMetDocumentInterface::GetSmartMetDocumentInterfaceImplementation());
	if(dlg.DoModal() == IDOK)
	{
		CWaitCursor waitCursor;
		NFmiEditMapGeneralDataDoc* doc = GetData();
		string forecasterId = CT2A(dlg.ForecasterIDStr());
		doc->DataToDBCheckMethod(dlg.DataCheckMethod());
		bool success = doc->StoreDataToDataBase(forecasterId);
		if(success && doc->IsOperationalModeOn())
		{
		}
		((CFrameWnd*)AfxGetMainWnd())->SetMessageText(AFX_IDS_IDLEMESSAGE); // LAURA!!!!!!!!
		return TRUE;
	}
	((CFrameWnd*)AfxGetMainWnd())->SetMessageText(AFX_IDS_IDLEMESSAGE); // LAURA!!!!!!!!
	return FALSE;
}

BOOL CSmartMetDoc::StoreData(bool newFile, bool askForSave)
{
	boost::shared_ptr<NFmiFastQueryInfo> info = GetData()->EditedSmartInfo();
	if(info)
	{
		if(newFile)
		{
			if(dynamic_cast<NFmiSmartInfo*>(info.get())->IsDirty())
				return StoreWorkingData(info, askForSave);
		}
		else
		{
			NFmiString fileName(info->DataFileName());
			BOOL status = StoreData(fileName, info, askForSave);
			return status;
		}
	}
	return TRUE;
}

// HUOM!!! laita tämä kutsumaan NFmiDoc:in metodia!!!!!!
BOOL CSmartMetDoc::StoreData(const NFmiString& theFileName, boost::shared_ptr<NFmiFastQueryInfo> &theSmartInfo, bool askForSave)
{
	CSaveDataDlg dlg;
	if(askForSave) // lisäsin askForSaven, että ei aina kysyttäisi talletetaanko
	{
		if(dlg.DoModal() != IDOK)
			return FALSE;
	}
	if(theSmartInfo && (theFileName != "")) // MITÄ JOS EI OLE NIMEÄ!!!!!!!
	{
		boost::shared_ptr<NFmiQueryData> data = theSmartInfo->DataReference();
		bool status = StoreData(theFileName, data) == TRUE;
		if(status)
			dynamic_cast<NFmiSmartInfo*>(theSmartInfo.get())->Dirty(false);
		return status;
	}
	return FALSE;
}

// HUOM!!! Tämä funktio pitää poistaa ja siirtyä käyttämään NFmiDoc:in metodia!!!!/Marko
BOOL CSmartMetDoc::StoreData(const NFmiString& theFileName, boost::shared_ptr<NFmiQueryData> &theData)
{
	if(theData)
	{
		std::ofstream out(theFileName, std::ios::binary);
		if(out)
		{
			CWaitCursor waitCursor;
			out << *theData;
			out.close();
			return TRUE;
		}
	}
	return FALSE;
}

void CSmartMetDoc::OnMenuitemOptiot()
{
	if(GetData())
	{
		COptionsDlg dlg;
		bool oldDoCacheSetting = GetData()->HelpDataInfoSystem()->UseQueryDataCache();
		dlg.SetData(SmartMetDocumentInterface::GetSmartMetDocumentInterfaceImplementation());
		if(dlg.DoModal() == IDOK)
		{
			GetData()->StoreOptionsData();
            CFmiQueryDataCacheLoaderThread::LoadDataAtStartUp(GetData()->ApplicationWinRegistry().ConfigurationRelatedWinRegistry().LoadDataAtStartUp());
            CFmiQueryDataCacheLoaderThread::AutoLoadNewCacheDataMode(GetData()->ApplicationWinRegistry().ConfigurationRelatedWinRegistry().AutoLoadNewCacheData());
			GetData()->MapDirty(itsMapViewDescTopIndex, true, true);
			bool cacheSettingChanged = oldDoCacheSetting != GetData()->HelpDataInfoSystem()->UseQueryDataCache();
			if(cacheSettingChanged)
			{
				CFmiCombineDataThread::InitCombineDataInfos(*(GetData()->HelpDataInfoSystem())); // kun cache asetukset muuttuvat, pitää yhdistelmä data asetuksia pävittää
				if(GetData()->HelpDataInfoSystem()->UseQueryDataCache())
					((CMainFrame*)AfxGetMainWnd())->StartQDataCacheThreads();
				else
					((CMainFrame*)AfxGetMainWnd())->StopQDataCacheThreads();
			}
			UpdateAllViewsAndDialogs(std::string(__FUNCTION__) + ": Settings dialog closed with Ok");
		}
	}
}

void CSmartMetDoc::OnSelectAll()
{
	if(GetData())
	{
		if(GetData()->SelectAllLocations(true))
		{
			GetData()->LogMessage("Select all grid points for editing.", CatLog::Severity::Info, CatLog::Category::Editing);
			UpdateAllViewsAndDialogs(std::string(__FUNCTION__) + ": selected all edited data grid points");
			GetData()->AreaViewDirty(itsMapViewDescTopIndex, false, false); // tämä pitäisi hoitaa järkevämmin, sillä tämä asetetaan true:ksi edellisessä funktiokutsussa
		}
	}
}

void CSmartMetDoc::OnDeselectAll()
{
	if(GetData())
	{
		if(GetData()->SelectAllLocations(false))
		{
			UpdateAllViewsAndDialogs(std::string(__FUNCTION__) + ": deselected all edited data grid points");
			GetData()->AreaViewDirty(itsMapViewDescTopIndex, false, false); // tämä pitäisi hoitaa järkevämmin, sillä tämä asetetaan true:ksi edellisessä funktiokutsussa
		}
	}
}

void CSmartMetDoc::OnEditUndo()
{
	if(GetData())
	{
		GetData()->OnButtonUndo();
	}
}

void CSmartMetDoc::OnEditRedo()
{
	if(GetData())
	{
		GetData()->OnButtonRedo();
	}
}

void CSmartMetDoc::OnButtonSelectionToolDlg()
{
	if(!itsLocationSelectionToolDlg)
		CreateLocationSelectionToolDialog(itsData);
	if(itsLocationSelectionToolDlg->ShowWindow(SW_RESTORE ))	// Vaihdoin SW_RESTOREN, muistaa ikkunan muutetun koon.
		itsLocationSelectionToolDlg->SetActiveWindow();
	GetData()->LogMessage("Opening editing grid point selector tool.", CatLog::Severity::Info, CatLog::Category::Operational);
}

void CSmartMetDoc::OnMenuitemSaveAs()
{
	boost::shared_ptr<NFmiFastQueryInfo> info = GetData()->EditedSmartInfo();
	if(info)
	{
		CFileDialog dlg(FALSE, _TEXT("sqd")); //muutin sqd:ksi
        dlg.m_ofn.lpstrInitialDir = CA2T(GetData()->FileDialogDirectoryMemory().c_str());
		if(dlg.DoModal() == IDOK)
		{
			NFmiString fileName = CT2A(dlg.GetPathName());
			BOOL status = StoreData(fileName,info);
			GetData()->LogMessage("Save as edited data...", CatLog::Severity::Info, CatLog::Category::Data);
		}
	}
}

template<typename DialogType>
void UpdateModalessDialog(DialogType *dialog)
{
    if(dialog)
        dialog->Update();
}

template<typename DialogType>
void CreateModalessDialog(DialogType **dialog, UINT resourceId, SmartMetDocumentInterface *smartMetDocumentInterface, bool callUpdate = true)
{
    if(!(*dialog))
    {
        *dialog = new DialogType(smartMetDocumentInterface, nullptr);
        BOOL status = (*dialog)->Create(resourceId, CWnd::FromHandle(GetDesktopWindow()));
    }
    if(callUpdate)
        UpdateModalessDialog(*dialog);
}

void CSmartMetDoc::CreateTimeEditor(bool callUpdate)
{
    CreateModalessDialog(&itsTimeSerialDataEditorDlg, IDD_DIALOG_TIME_EDIT_VALUES, SmartMetDocumentInterface::GetSmartMetDocumentInterfaceImplementation(), callUpdate);
}

void CSmartMetDoc::CreateParamAddingDlg(NFmiEditMapGeneralDataDoc *theDoc)
{
    CreateModalessDialog(&itsParamAddingDlg, IDD_DIALOG_PARAM_ADDING, SmartMetDocumentInterface::GetSmartMetDocumentInterfaceImplementation());
}

void CSmartMetDoc::CreateSynopDataGridViewDlg(NFmiEditMapGeneralDataDoc *theDoc)
{
    CreateModalessDialog(&itsSynopDataGridViewDlg, IDD_DIALOG_SYNOP_DATA_GRID_VIEW, SmartMetDocumentInterface::GetSmartMetDocumentInterfaceImplementation(), false);
}

void CSmartMetDoc::CreateShortCutsDlg(void)
{
    if(!itsShortCutsDialog)
    {
        itsShortCutsDialog = new CFmiShortCutsDlg2();
        BOOL status = itsShortCutsDialog->Create(IDD_DIALOG_SHORT_CUTS, CWnd::FromHandle(GetDesktopWindow()));
    }
}

void CSmartMetDoc::CreateViewMacroDlg(NFmiEditMapGeneralDataDoc * theDoc)
{
    CreateModalessDialog(&itsViewMacroDlg, IDD_DIALOG_VIEW_MACRO_DLG, SmartMetDocumentInterface::GetSmartMetDocumentInterfaceImplementation(), false);
}

void CSmartMetDoc::CreateCrossSectionDlg(bool callUpdate)
{
    CreateModalessDialog(&itsCrossSectionDlg, IDD_DIALOG_CROSS_SECTION, SmartMetDocumentInterface::GetSmartMetDocumentInterfaceImplementation(), callUpdate);
}

void CSmartMetDoc::CreateTrajectoryDlg(NFmiEditMapGeneralDataDoc * theDoc)
{
    CreateModalessDialog(&itsTrajectoryDlg, IDD_DIALOG_TRAJECTORY, SmartMetDocumentInterface::GetSmartMetDocumentInterfaceImplementation(), false);
}

void CSmartMetDoc::CreateWarningCenterDlg(NFmiEditMapGeneralDataDoc *theDoc)
{
    CreateModalessDialog(&itsWarningCenterDlg, IDD_DIALOG_WARNING_CENTER_DLG, SmartMetDocumentInterface::GetSmartMetDocumentInterfaceImplementation(), false);
}

void CSmartMetDoc::CreateSeaIcingWarningsDlg(NFmiEditMapGeneralDataDoc *theDoc)
{
    CreateModalessDialog(&itsSeaIcingWarningsDlg, IDD_DIALOG_WARNING_CENTER_DLG, SmartMetDocumentInterface::GetSmartMetDocumentInterfaceImplementation(), false);
}

void CSmartMetDoc::CreateWindTableDlg(NFmiEditMapGeneralDataDoc *theDoc)
{
    CreateModalessDialog(&itsWindTableDlg, IDD_DIALOG_WIND_TABLE, SmartMetDocumentInterface::GetSmartMetDocumentInterfaceImplementation(), false);
}

void CSmartMetDoc::CreateDataQualityCheckerDialog(NFmiEditMapGeneralDataDoc *theDoc)
{
    CreateModalessDialog(&itsDataQualityCheckerDialog, IDD_DIALOG_DATA_QUALITY_CHECKER, SmartMetDocumentInterface::GetSmartMetDocumentInterfaceImplementation(), false);
}

void CSmartMetDoc::CreateIgnoreStationsDlg(NFmiEditMapGeneralDataDoc *theDoc)
{
    CreateModalessDialog(&itsIgnoreStationsDlg, IDD_DIALOG_IGNORE_STATIONS, SmartMetDocumentInterface::GetSmartMetDocumentInterfaceImplementation(), false);
}

void CSmartMetDoc::CreateCaseStudyDlg(NFmiEditMapGeneralDataDoc *theDoc)
{
	if(itsCaseStudyDlg)
		delete itsCaseStudyDlg;

	POSITION pos = GetFirstViewPosition();	// annetaan aikasarjaikkunalle cview parentti-ikkunaksi näytön päivityksiä varten
	CView* pView = GetNextView(pos);		// ei toimi jos tulevaisuudessa vaihdetaan näyttöä kesken kaiken (esim. multi-doc-view)

	std::string titleStr = ::GetDictionaryString("Case study dialog");
    itsCaseStudyDlg = new CFmiCaseStudyDlg(&theDoc->GetSmartMetDocumentInterface(), titleStr, pView);
	BOOL status = itsCaseStudyDlg->Create(pView->GetDesktopWindow());
}

void CSmartMetDoc::CreateBetaProductDialog(SmartMetDocumentInterface *smartMetDocumentInterface)
{
    if(itsBetaProductDialog)
        return ; // Ei luoda dialogia uudestaan, tätä kutsutaan myös GeneralDataDocista, joka ei tieä onko dialogi jo olemassa

    POSITION pos = GetFirstViewPosition();	// annetaan aikasarjaikkunalle cview parentti-ikkunaksi näytön päivityksiä varten
    CView* pView = GetNextView(pos);		// ei toimi jos tulevaisuudessa vaihdetaan näyttöä kesken kaiken (esim. multi-doc-view)

    itsBetaProductDialog = new CFmiBetaProductTabControlDialog(smartMetDocumentInterface, pView);
    BOOL status = itsBetaProductDialog->Create(pView->GetDesktopWindow());
}

void CSmartMetDoc::CreateLogViewer(NFmiEditMapGeneralDataDoc *theDoc)
{
    if(itsLogViewer)
        return; // Ei luoda dialogia uudestaan, tätä kutsutaan myös GeneralDataDocista, joka ei tieä onko dialogi jo olemassa

    CSmartMetView *smartmetView = ApplicationInterface::GetSmartMetView();
    itsLogViewer = new CFmiLogViewer(theDoc->ApplicationWinRegistry(), smartmetView);
    BOOL status = itsLogViewer->Create(IDD_DIALOG_LOG_VIEWER, smartmetView->GetDesktopWindow());
    if(status)
    {
        auto refreshCallback = [this]() { this->itsLogViewer->StartRefreshTimer(); };
        CatLog::setLogViewerUpdateCallback(refreshCallback);
    }
}

void CSmartMetDoc::OnAcceleratorIgnoreStationsDlg()
{
	if(itsData)
	{
		if(!itsIgnoreStationsDlg)
			CreateIgnoreStationsDlg(itsData);

		if(itsData->IgnoreStationsData().IgnoreStationsDialogOn())
		{ // laitetaan aikasarjaikkuna kiinni
			itsIgnoreStationsDlg->ShowWindow(SW_HIDE);
            itsData->IgnoreStationsData().IgnoreStationsDialogOn(false);
            GetData()->LogMessage("Closes Ignore stations dialog.", CatLog::Severity::Info, CatLog::Category::Operational);
		}
		else
		{ // laitetaan aikasarja ikkuna päälle
			itsIgnoreStationsDlg->ShowWindow(SW_SHOW);	// Vaihdoin SW_RESTOREN, muistaa ikkunan muutetun koon.
			itsIgnoreStationsDlg->SetActiveWindow();
			itsData->IgnoreStationsData().IgnoreStationsDialogOn(true);
//			itsIgnoreStationsDlg->Update();
			GetData()->LogMessage("Open Ignore station dialog.", CatLog::Severity::Info, CatLog::Category::Operational);
		}
		itsData->TimeSerialDataViewOn(!itsData->TimeSerialDataViewOn());
	}
}

// tämä create dialogi todella luo uuden dialogin ja palauttaa
// sen toisin kuin muut vastaavat dialogit, jotka luovat haluttuun member-data-pointteriin dialogin
CFmiExtraMapViewDlg* CSmartMetDoc::CreateExtraMapViewDlg(NFmiEditMapGeneralDataDoc *theDoc, unsigned int theMapViewDescTopIndex)
{
	POSITION pos = GetFirstViewPosition();	// annetaan aikasarjaikkunalle cview parentti-ikkunaksi näytön päivityksiä varten
	CView* pView = GetNextView(pos);		// ei toimi jos tulevaisuudessa vaihdetaan näyttöä kesken kaiken (esim. multi-doc-view)

	CFmiExtraMapViewDlg *extraMapViewDlg = new CFmiExtraMapViewDlg(SmartMetDocumentInterface::GetSmartMetDocumentInterfaceImplementation(), theMapViewDescTopIndex, pView);
	BOOL status = extraMapViewDlg->Create(IDD_DIALOG_MAP_VIEW_EXTRA, pView->GetDesktopWindow());
	if(status)
	{
		// asetetaan vielä karttanäytön otsikko
		string magickWord("ID_EXTRA_MAP_VIEW_");
		magickWord += NFmiStringTools::Convert<unsigned int>(theMapViewDescTopIndex);
		magickWord += "_TOOLTIP"; // otetaan extrakarttanäytön otsikko extrakarttanäytön toolbar-buttonin tooltip arvosta
        extraMapViewDlg->SetWindowText(CA2T(::GetDictionaryString(magickWord.c_str()).c_str()));
		return extraMapViewDlg;
	}
	else
	{
		delete extraMapViewDlg;
		return 0;
	}
}

void CSmartMetDoc::CreateSynopPlotSettingsDlg(NFmiEditMapGeneralDataDoc * theDoc)
{
	if(itsSynopPlotSettingsDlg)
		delete itsSynopPlotSettingsDlg; // jos joskus pitää välillä tuhota aikasarjaeditori, tämä ei riitä, koska tämän jälkeen luotu dialogi tulee olemaan aina pääohjelman päällä?!?

	POSITION pos = GetFirstViewPosition();	// annetaan aikasarjaikkunalle cview parentti-ikkunaksi näytön päivityksiä varten
	CView* pView = GetNextView(pos);		// ei toimi jos tulevaisuudessa vaihdetaan näyttöä kesken kaiken (esim. multi-doc-view)

	itsSynopPlotSettingsDlg = new CFmiSynopPlotSettingsDlg(SmartMetDocumentInterface::GetSmartMetDocumentInterfaceImplementation(), pView);
	BOOL status = itsSynopPlotSettingsDlg->Create(IDD_DIALOG_SYNOP_PLOT_SETTINGS, pView->GetDesktopWindow());
}

void CSmartMetDoc::CreateLocationSelectionToolDialog(NFmiEditMapGeneralDataDoc * theDoc)
{
	if(itsLocationSelectionToolDlg)
		delete itsLocationSelectionToolDlg; // jos joskus pitää välillä tuhota aikasarjaeditori, tämä ei riitä, koska tämän jälkeen luotu dialogi tulee olemaan aina pääohjelman päällä?!?

	POSITION pos = GetFirstViewPosition();	// annetaan aikasarjaikkunalle cview parentti-ikkunaksi näytön päivityksiä varten
	CView* pView = GetNextView(pos);		// ei toimi jos tulevaisuudessa vaihdetaan näyttöä kesken kaiken (esim. multi-doc-view)

	itsLocationSelectionToolDlg = new CFmiLocationSelectionToolDlg2(SmartMetDocumentInterface::GetSmartMetDocumentInterfaceImplementation());
	BOOL status = itsLocationSelectionToolDlg->Create(IDD_DIALOG_LOCATION_SELECTION_TOOL2, pView->GetDesktopWindow());
}

void CSmartMetDoc::CreateZoomDlg(NFmiEditMapGeneralDataDoc * theDoc)
{
	if(itsZoomDlg)
		delete itsZoomDlg; // jos joskus pitää välillä tuhota zoomi-dialogi, tämä ei riitä, koska tämän jälkeen luotu dialogi tulee olemaan aina pääohjelman päällä?!?

	POSITION pos = GetFirstViewPosition();	// annetaan aikasarjaikkunalle cview parentti-ikkunaksi näytön päivityksiä varten
	CView* pView = GetNextView(pos);		// ei toimi jos tulevaisuudessa vaihdetaan näyttöä kesken kaiken (esim. multi-doc-view)

	itsZoomDlg = new CZoomDlg(SmartMetDocumentInterface::GetSmartMetDocumentInterfaceImplementation(), pView);
	BOOL status = itsZoomDlg->Create(IDD_DIALOG_ZOOM_MAP, pView->GetDesktopWindow());
}

void CSmartMetDoc::OnButtonSelectEuropeMap()
{
	if(GetData())
	{
		GetData()->LogMessage("Map 3 selected.", CatLog::Severity::Info, CatLog::Category::Operational);
		GetData()->SetSelectedMapHandler(itsMapViewDescTopIndex, 2);
		UpdateAllViewsAndDialogs("Map area 3 selected in main map view");
	}
}

void CSmartMetDoc::OnButtonSelectFinlandMap()
{
	if(GetData())
	{
		GetData()->LogMessage("Map 1 selected.", CatLog::Severity::Info, CatLog::Category::Operational);
		GetData()->SetSelectedMapHandler(itsMapViewDescTopIndex, 0);
		UpdateAllViewsAndDialogs("Map area 1 selected in main map view");
	}
}

void CSmartMetDoc::OnButtonSelectScandinaviaMap()
{
	if(GetData())
	{
		GetData()->LogMessage("Map 2 selected.", CatLog::Severity::Info, CatLog::Category::Operational);
		GetData()->SetSelectedMapHandler(itsMapViewDescTopIndex, 1);
		UpdateAllViewsAndDialogs("Map area 2 selected in main map view");
	}
}

bool CSmartMetDoc::IsMapSelected(int theMapIndex)
{
    return GetData()->ApplicationWinRegistry().ConfigurationRelatedWinRegistry().MapView(itsMapViewDescTopIndex)->SelectedMapIndex() == theMapIndex;
}

void CSmartMetDoc::OnUpdateButtonSelectScandinaviaMap(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(IsMapSelected(1));
}

void CSmartMetDoc::OnButtonGlobe()
{
	if(GetData())
	{
		GetData()->LogMessage("Map 4 selected.", CatLog::Severity::Info, CatLog::Category::Operational);
		GetData()->SetSelectedMapHandler(itsMapViewDescTopIndex, 3);
		UpdateAllViewsAndDialogs("Map area 4 selected in main map view");
	}
}

void CSmartMetDoc::OnButtonDataToDatabase()
{
	boost::shared_ptr<NFmiFastQueryInfo> editedInfo = GetData()->EditedSmartInfo();
	NFmiSmartInfo *smart = dynamic_cast<NFmiSmartInfo*>(editedInfo.get());
	if(smart == 0)
	{
		CNoSaveInfoDlg dlg;
		if(dlg.DoModal() == IDOK)
			return;
	}
	else
	{
		NFmiMetTime origTime(1); // lähetettäessä data tietokantaan, tehdään datalle minuutin tarkuudella origin-aika
		smart->OriginTime(origTime);
		smart->RefQueryData()->Info()->OriginTime(origTime);

		BOOL status = TRUE;
		if(smart->IsDirty())
		{
			if(smart->LoadedFromFile())
				StoreData();
			else
				status = StoreWorkingData(editedInfo);
		}
		if(!smart->LoadedFromFile()) //vain dataloadinginfon mukaiset smartit tiedostoon
		{
			if(GetData()->CheckEditedDataForStartUpLoadErrors(MB_OKCANCEL | MB_ICONERROR))
			{
				status = StoreDataBaseDataMarko(editedInfo);
			}
		}
		else
		{
			std::string msgStr;
			msgStr += ::GetDictionaryString("MessageBoxNotSupposeToSendThisDataToDBRow1");
			msgStr += "\n";
			msgStr += ::GetDictionaryString("MessageBoxNotSupposeToSendThisDataToDBRow2");
			std::string titleStr(::GetDictionaryString("MessageBoxNotSupposeToSendThisDataToDBTitle"));
            if(::MessageBox(AfxGetMainWnd()->GetSafeHwnd(), CA2T(msgStr.c_str()), CA2T(titleStr.c_str()), MB_YESNO | MB_ICONWARNING) == IDYES)
				status = StoreDataBaseDataMarko(editedInfo);
		}
	}
}

void CSmartMetDoc::OnDataLoadFromFile()
{
	BOOL stat = PrepareForLoadingDataFromFile();

	static TCHAR BASED_CODE szFilter[] = _TEXT("QD-files (*.sqd;*.fqd)|*.sqd; *.fqd|all files (*.*)|*.*||");
	CFileDialog dlg(TRUE, NULL, NULL, OFN_ALLOWMULTISELECT|OFN_EXPLORER, szFilter);
    dlg.m_ofn.lpstrInitialDir = CA2T(GetData()->FileDialogDirectoryMemory().c_str());

	// Allocate storage for szSelections
	const int kBufferSize = 20000;
	TCHAR *szSelections = new TCHAR[kBufferSize];
	if(szSelections == NULL) // can't use assert in release builds
	{
		::MessageBox(ApplicationInterface::GetSmartMetView()->GetSafeHwnd(), _TEXT("Could not allocate 20K block."),
			_TEXT("Memory Allocation Error"), MB_OK|MB_ICONEXCLAMATION);
		return ;
	}
	memset(szSelections, NULL, kBufferSize);

	dlg.m_ofn.lpstrFile = szSelections;
	dlg.m_ofn.nMaxFile = kBufferSize;

	int status = static_cast<int>(dlg.DoModal());

	if(status == IDOK) // || status == IDCANCEL)
	{
		vector<string> files;

		POSITION pos = dlg.GetStartPosition();
		for(int i=0; pos != 0; i++)
		{
			string fileName = CT2A(dlg.GetNextPathName(pos));
			files.push_back(fileName);
			if(i==0)
				GetData()->MakeAndStoreFileDialogDirectoryMemory(fileName);
		}
		LoadDataFromFilesAndAdd(files);
		delete [] szSelections;
	}
}

BOOL CSmartMetDoc::LoadDataFromFile(const char* theFileName)
{
	CWaitCursor cursor;
    CString errorStringU_;
	try
	{
        std::unique_ptr<NFmiQueryData> data = QueryDataReading::ReadDataFromFile(theFileName, GetData()->ApplicationWinRegistry().ConfigurationRelatedWinRegistry().DroppedDataEditable() == false);
        if(data)
        {
            GetData()->AddQueryData(data.release(), theFileName, "", NFmiInfoData::kEditable, "", true);
            SetPathName(CA2T(theFileName)); // pitää laittaa AddData:n jälkeen, muuten tiedoston nimi ei tule oikein ohjelman otsikkoon.
            UpdateAllViewsAndDialogs("Loaded new edited data");
            return TRUE;
        }
    }
	catch(char *msg)
	{
        errorStringU_ = CA2T(msg);
        CDataLoadingProblemsDlg dlg(errorStringU_);
		if(dlg.DoModal() == IDOK)
			return FALSE;
	}
	catch(exception &e)
	{
        errorStringU_ = CA2T(e.what());
        CDataLoadingProblemsDlg dlg(errorStringU_);
		if(dlg.DoModal() == IDOK)
			return FALSE;
	}
	catch(...)
	{
        errorStringU_ = _TEXT("Unknown error while readin data file: ");
        errorStringU_ += CA2T(theFileName);
        CDataLoadingProblemsDlg dlg(errorStringU_);
		if(dlg.DoModal() == IDOK)
			return FALSE;
	}

	return FALSE;
}

void CSmartMetDoc::OnDataStoreToFile()
{
	StoreData();
}


// lähinnä undo- ja redo-nappuloiden päivitys (aikasarjadialogissa ja filtteröintidialogissa)
void CSmartMetDoc::UpdateAllDialogsButtons(void)
{
	if(itsFilterDlg)
		itsFilterDlg->EnableButtons();
	if(itsBrushToolDlg)
		itsBrushToolDlg->EnableButtons();
}

template<typename View>
static void UpdateMapView(NFmiEditMapGeneralDataDoc *doc, View *extraMapView, unsigned int mapViewIndex)
{
    // karttaa ei päivitetä jos ollaan klikattu hiirellä karttaa ja valittu asema
    // tällöin ruutu piirretään vain vastaväreillä (voi muuttua tulevaisuudessa!!!)

    if(extraMapView && doc->MapViewDescTop(mapViewIndex)->MapViewUpdated())
    {
        doc->AreaViewDirty(mapViewIndex, true, false);
        extraMapView->Update();
    }
    else if(extraMapView)
        doc->MapViewDescTop(mapViewIndex)->MapViewUpdated(true); // tämä tarkoittaa, että seuraavalla kerralla voidaan päivittää karttaa
}

bool CSmartMetDoc::UpdateAllViewsAndDialogsIsAllowed()
{
    // printatessa ruutujen päivitys voi aiheuttaa ongelmia
    if(GetData()->Printing())
        return false;
    // Uusien datan muokkaus rutiinien kanssa ruutujen päivitys voi aiheuttaa ongelmia 
    // Multi-thread ja/tai progress/cancel (-> erillinen muokkaus threadi) systeemit menevät jotenkin sekaisin
    if(GetData()->DataModificationInProgress())
        return false;

    return true;
}

static void MakeGeneralUpdateTraceLogging(const std::string &totalLogMessage, size_t &updateCounter)
{
    // OBS! Don't do the CatLog::doTraceLevelLogging check here, because we want that updateCounter keeps incremented through out all application life time.

    CtrlViewUtils::CtrlViewTimeConsumptionReporter::setCurrentUpdateId(updateCounter++);
    // HUOM! ei kannata mitata koko UpdateAllViewsAndDialogs metodin käyttämää aikaa, koska se vain tekee päivitys pyyntöjä eri ikkunoille,
    // joille win32 systeemi jakaa käskyjä miten haluaa. Eli kokonais SmartMet ikkunoiden päivitys aikaa on mahdoton koostaa suoraan.
    CtrlViewUtils::CtrlViewTimeConsumptionReporter::makeSeparateTraceLogging(totalLogMessage, nullptr);
}

// Lasketaan update counter indeksiä, jonka avulla voidaan lokiviesteistä hakea tietyn update kierroksen lokituksia.
// Ei aloiteta lukua 1:stä, koska jos etsii vain lukua 1 tai 2-9, tulee liian paljon osumia vääristä lokiviesteistä
static size_t g_UpdateAllViewsAndDialogsCounter = 1001;

void CSmartMetDoc::UpdateAllViewsAndDialogs(const std::string &reasonForUpdate, bool fUpdateOnlyMapViews)
{
    if(UpdateAllViewsAndDialogsIsAllowed())
    {
        ::MakeGeneralUpdateTraceLogging(std::string("***** ") + __FUNCTION__ + " ***** {Reason: " + reasonForUpdate + "}", g_UpdateAllViewsAndDialogsCounter);
        if(fUpdateOnlyMapViews)
            CtrlViewUtils::CtrlViewTimeConsumptionReporter::makeSeparateTraceLogging(std::string(__FUNCTION__) + ": only map-views are updated", nullptr);

        if(fUpdateOnlyMapViews == false)
            ::UpdateModalessDialog(itsZoomDlg);

        ::UpdateMapView(GetData(), ApplicationInterface::GetSmartMetView(), 0);
        ::UpdateMapView(GetData(), itsExtraMapViewDlg1, 1);
        ::UpdateMapView(GetData(), itsExtraMapViewDlg2, 2);

        if(fUpdateOnlyMapViews == false)
        {
            ::UpdateModalessDialog(itsTimeSerialDataEditorDlg);
            ::UpdateModalessDialog(itsFilterDlg);
            ::UpdateModalessDialog(itsBrushToolDlg);
            UpdateAllDialogsButtons();
            ::UpdateModalessDialog(itsTempDialog);
            ::UpdateModalessDialog(itsCrossSectionDlg);
            ::UpdateModalessDialog(itsSynopPlotSettingsDlg);
            ::UpdateModalessDialog(itsSmartToolDlg);
            ::UpdateModalessDialog(itsSynopDataGridViewDlg);
            ::UpdateModalessDialog(itsTrajectoryDlg);
            ::UpdateModalessDialog(itsWarningCenterDlg);
            ::UpdateModalessDialog(itsSeaIcingWarningsDlg);
            ::UpdateModalessDialog(itsWindTableDlg);
//            ::UpdateModalessDialog(itsDataQualityCheckerDialog);
            ::UpdateModalessDialog(itsBetaProductDialog);
            ::UpdateModalessDialog(itsViewMacroDlg);
            ::UpdateModalessDialog(itsParamAddingDlg);
        }
    }
}

static void MakeUpdatedViewsTraceLogging(const std::string &functionName, SmartMetViewId updatedViewsFlag)
{
    if(CatLog::doTraceLevelLogging())
    {
        std::string totalLogMessage = functionName;
        totalLogMessage += ": Updating view(s): ";
        totalLogMessage += SmartMetViewIdGetNameList(updatedViewsFlag);

        CtrlViewUtils::CtrlViewTimeConsumptionReporter::makeSeparateTraceLogging(totalLogMessage, nullptr);
    }
}

// If updatedViewsFlag = SmartMetViewId::NoViews, it means that all the views are updated, and it's handled by other UpdateAllViewsAndDialogs method
void CSmartMetDoc::UpdateAllViewsAndDialogs(const std::string &reasonForUpdate, SmartMetViewId updatedViewsFlag)
{
    if(updatedViewsFlag == SmartMetViewId::NoViews)
        UpdateAllViewsAndDialogs(reasonForUpdate);
    else if(UpdateAllViewsAndDialogsIsAllowed())
    {
        ::MakeGeneralUpdateTraceLogging(std::string("***** ") + __FUNCTION__ + "-v2 ***** {Reason: " + reasonForUpdate + "}", g_UpdateAllViewsAndDialogsCounter);
        ::MakeUpdatedViewsTraceLogging(__FUNCTION__, updatedViewsFlag);

        if(SmartMetViewIdFlagCheck(updatedViewsFlag, SmartMetViewId::ZoomDlg))
            ::UpdateModalessDialog(itsZoomDlg);
        if(SmartMetViewIdFlagCheck(updatedViewsFlag, SmartMetViewId::MainMapView))
            ::UpdateMapView(GetData(), ApplicationInterface::GetSmartMetView(), 0);
        if(SmartMetViewIdFlagCheck(updatedViewsFlag, SmartMetViewId::MapView2))
            ::UpdateMapView(GetData(), itsExtraMapViewDlg1, 1);
        if(SmartMetViewIdFlagCheck(updatedViewsFlag, SmartMetViewId::MapView3))
            ::UpdateMapView(GetData(), itsExtraMapViewDlg2, 2);
        if(SmartMetViewIdFlagCheck(updatedViewsFlag, SmartMetViewId::TimeSerialView))
            ::UpdateModalessDialog(itsTimeSerialDataEditorDlg);
        if(SmartMetViewIdFlagCheck(updatedViewsFlag, SmartMetViewId::DataFilterToolDlg))
            ::UpdateModalessDialog(itsFilterDlg);
        if(SmartMetViewIdFlagCheck(updatedViewsFlag, SmartMetViewId::BrushToolDlg))
            ::UpdateModalessDialog(itsBrushToolDlg);
        UpdateAllDialogsButtons();
        if(SmartMetViewIdFlagCheck(updatedViewsFlag, SmartMetViewId::SoundingView))
            ::UpdateModalessDialog(itsTempDialog);
        if(SmartMetViewIdFlagCheck(updatedViewsFlag, SmartMetViewId::CrossSectionView))
            ::UpdateModalessDialog(itsCrossSectionDlg);
        if(SmartMetViewIdFlagCheck(updatedViewsFlag, SmartMetViewId::SynopPlotSettingsDlg))
            ::UpdateModalessDialog(itsSynopPlotSettingsDlg);
        if(SmartMetViewIdFlagCheck(updatedViewsFlag, SmartMetViewId::SmartToolDlg))
            ::UpdateModalessDialog(itsSmartToolDlg);
        if(SmartMetViewIdFlagCheck(updatedViewsFlag, SmartMetViewId::StationDataTableView))
            ::UpdateModalessDialog(itsSynopDataGridViewDlg);
        if(SmartMetViewIdFlagCheck(updatedViewsFlag, SmartMetViewId::TrajectoryView))
            ::UpdateModalessDialog(itsTrajectoryDlg);
        if(SmartMetViewIdFlagCheck(updatedViewsFlag, SmartMetViewId::WarningCenterDlg))
            ::UpdateModalessDialog(itsWarningCenterDlg);
        if(SmartMetViewIdFlagCheck(updatedViewsFlag, SmartMetViewId::SeaIcingDlg))
            ::UpdateModalessDialog(itsSeaIcingWarningsDlg);
        if(SmartMetViewIdFlagCheck(updatedViewsFlag, SmartMetViewId::WindTableDlg))
            ::UpdateModalessDialog(itsWindTableDlg);
        //            ::UpdateModalessDialog(itsDataQualityCheckerDialog);
        if(SmartMetViewIdFlagCheck(updatedViewsFlag, SmartMetViewId::BetaProductionDlg))
            ::UpdateModalessDialog(itsBetaProductDialog);
        if(SmartMetViewIdFlagCheck(updatedViewsFlag, SmartMetViewId::ViewMacroDlg))
            ::UpdateModalessDialog(itsViewMacroDlg);
        if(SmartMetViewIdFlagCheck(updatedViewsFlag, SmartMetViewId::ParamAddingDlg))
            ::UpdateModalessDialog(itsParamAddingDlg);
    }
}

void CSmartMetDoc::UpdateOnlyExtraMapViews(bool updateMap1, bool updateMap2)
{
	if(updateMap1)
        ::UpdateMapView(GetData(), itsExtraMapViewDlg1, 1);

	if(updateMap2)
        ::UpdateMapView(GetData(), itsExtraMapViewDlg2, 2);
}

void CSmartMetDoc::UpdateCrossSectionView(void)
{
    ::UpdateModalessDialog(itsCrossSectionDlg);
}

void CSmartMetDoc::UpdateTempView(void)
{
    ::UpdateModalessDialog(itsTempDialog);
}

void CSmartMetDoc::UpdateTimeSerialView(void)
{
    ::UpdateModalessDialog(itsTimeSerialDataEditorDlg);
}

void CSmartMetDoc::UpdateTrajectorySystem(void)
{
    ::UpdateModalessDialog(itsTrajectoryDlg);
}

void CSmartMetDoc::OnUpdateEditUndo(CCmdUI* pCmdUI)
{
	if(GetData()->SmartMetEditingMode() == CtrlViewUtils::kFmiEditingModeNormal) // eli jos ollaan edit-moodissa, asetetaan nappulan tilaa
	{
		boost::shared_ptr<NFmiFastQueryInfo> editedInfo = GetData()->EditedSmartInfo();
		if(editedInfo)
		{
			pCmdUI->Enable(dynamic_cast<NFmiSmartInfo*>(editedInfo.get())->Undo());
		}
		else
			pCmdUI->Enable(FALSE);
	}
	else	// jos ollaan ns. view-moodissa, disabloidaan koko nappula
		pCmdUI->Enable(FALSE);
}

void CSmartMetDoc::OnUpdateEditRedo(CCmdUI* pCmdUI)
{
	if(GetData()->SmartMetEditingMode() == CtrlViewUtils::kFmiEditingModeNormal) // eli jos ollaan edit-moodissa, asetetaan nappulan tilaa
	{
		boost::shared_ptr<NFmiFastQueryInfo> editedInfo = GetData()->EditedSmartInfo();
		if(editedInfo)
		{
			pCmdUI->Enable(dynamic_cast<NFmiSmartInfo*>(editedInfo.get())->Redo());
		}
		else
			pCmdUI->Enable(FALSE);
	}
	else	// jos ollaan ns. view-moodissa, disabloidaan koko nappula
		pCmdUI->Enable(FALSE);
}

void CSmartMetDoc::OnHideParamWindow()
{
	GetData()->OnHideParamWindow(itsMapViewDescTopIndex);
}

void CSmartMetDoc::OnMakeGridFile()
{
	if(GetData())
	{
		CWaitCursor cursor;
		GetData()->MakeGridFile("grid.txt");
		GetData()->LogMessage("Storing active grid data to file.", CatLog::Severity::Debug, CatLog::Category::Data);
	}
}

void CSmartMetDoc::OnButtonMakeEditedDataCopy()
{
	if(GetData())
	{
		GetData()->UpdateEditedDataCopy();
		GetData()->LogMessage("Copy active grid data.", CatLog::Severity::Debug, CatLog::Category::Editing);
		UpdateAllViewsAndDialogs("Updated edited data copy");
	}
}

void CSmartMetDoc::OnButtonFilterDialog()
{
	if(!itsFilterDlg)
		CreateFilterDlg(itsData);
	if(itsFilterDlg->ShowWindow(SW_RESTORE ))	// Vaihdoin SW_RESTOREN, muistaa ikkunan muutetun koon.
		itsFilterDlg->SetActiveWindow();
	GetData()->FilterDialogUpdateStatus(2); // 2 = päivitä koko ruutu
	itsFilterDlg->Update();
	GetData()->LogMessage("Opening Data Filter tool.", CatLog::Severity::Info, CatLog::Category::Editing);
}

void CSmartMetDoc::CreateFilterDlg(NFmiEditMapGeneralDataDoc * theDoc)
{
	if(itsFilterDlg)
		delete itsFilterDlg;

	POSITION pos = GetFirstViewPosition();	// annetaan aikasarjaikkunalle cview parentti-ikkunaksi näytön päivityksiä varten
	CView* pView = GetNextView(pos);		// ei toimi jos tulevaisuudessa vaihdetaan näyttöä kesken kaiken (esim. multi-doc-view)

	itsFilterDlg = new CFmiFilterDataDlg(SmartMetDocumentInterface::GetSmartMetDocumentInterfaceImplementation(), pView);
	BOOL status = itsFilterDlg->Create(pView->GetDesktopWindow());
}

void CSmartMetDoc::ActivateFilterDlg(void)
{
	if(itsFilterDlg)
	{
		itsFilterDlg->InitDialogFromDoc();
		OnButtonFilterDialog();
	}
}

static bool IsHybridData(NFmiQueryData *theData)
{
	if(theData && theData->Info() && theData->Info()->SizeLevels() > 1)
	{
		theData->Info()->FirstLevel();
		if(theData->Info()->Level()->LevelType() == kFmiHybridLevel)
			return true;
	}
	return false;
}

// tämä hoitaa sekä multi-file drag'n dropin ja file-open dialog jutun
void CSmartMetDoc::LoadDataFromFilesAndAdd(std::vector<std::string> &theFileList)
{
	NFmiInfoData::Type datatype = NFmiInfoData::kEditable; // 1. data on aina editable
    if(theFileList.size())
    {
        for(size_t i = 0; i < theFileList.size(); i++)
        {
            LoadDataFromFileAndAdd(theFileList[i], datatype, GetData()->ApplicationWinRegistry().ConfigurationRelatedWinRegistry().DroppedDataEditable() == false);
            datatype = NFmiInfoData::kViewable; // loput ovat ns. apudatoja eli kViewable:ja (LoadDataFromFileAndAdd metodissa vielä lisää tarkasteluja)

            // Jos 1. data, joka laitetaan  editoitavaksi dataksi ja muitakin datoja on tiputettu,
            // laitetaan 1. data myös viewable-dataksi. Töllöin jos tiputat esim. Hirlamin pinta+painepinta ja mallipinta
            // datat editoriin, päätyvät ne kaikki myös hirlam-valikoihin, eikä 1. jää vain editoitavaksi dataksi.
            if(i == 0)
            {
                SetPathName(CA2T(theFileList[i].c_str())); // pitää laittaa AddData:n jälkeen, muuten tiedoston nimi ei tule oikein ohjelman otsikkoon.

                if(theFileList.size() > 1)
                    LoadDataFromFileAndAdd(theFileList[i], datatype, true);
            }
        }
        UpdateAllViewsAndDialogs("Data files where drag'n dropped on SmartMet");
    }
}

// Tämä on tehty ainakin file drop datan yhteydessä käytettäväksi.
// Oletus, tätä ei kutsuta mistään muualta kuin LoadDataFromFilesAndAdd metodista ja siellä tehdään yhteinen UpdateAllViewsAndDialogs kutsu.
void CSmartMetDoc::LoadDataFromFileAndAdd(const std::string &theFileName, NFmiInfoData::Type theDatatype, bool useMemoryMapping)
{
    CString errorStringU_;
    try
	{
        std::unique_ptr<NFmiQueryData> data = QueryDataReading::ReadDataFromFile(theFileName, useMemoryMapping);
        if(data)
        {
            if(theDatatype == NFmiInfoData::kViewable)
            { // tarkistetaan että onko data hybridi dataa, jos on, laitetaan data tyypiksi hybridi
                if(::IsHybridData(data.get()))
                    theDatatype = NFmiInfoData::kHybridData;
                // sitten tarkistetaan onko se mahdollisesti havainto dataa
                if(data->Info() && data->Info()->IsGrid() == false)
                    theDatatype = NFmiInfoData::kObservations;
            }

            std::string filePattern = theFileName; // annetaan filepatterniksi tiputettujen datojen yhteydessä tiedoston nimi, muuten homma ei toimi kunnolla infoorganizerissa
            GetData()->AddQueryData(data.release(), theFileName, filePattern, theDatatype, "", true);

            if(theDatatype == NFmiInfoData::kEditable)
            {
                GetData()->EditedDataNotInPreferredState(false); // tämä pitää asettaa varmuuden vuoksi falseksi, oletetaan että kaikki tiputetut tai erikseen ladattavat editoitavat datat ovat 'halutussa' tilassa eli eivät ole liian vanhoja.
                SetPathName(CA2T(theFileName.c_str())); // pitää laittaa AddData:n jälkeen, muuten tiedoston nimi ei tule oikein ohjelman otsikkoon.
            }
        }
	}
	catch(char *msg)
	{
        GetData()->LogMessage(msg, CatLog::Severity::Error, CatLog::Category::Data);
        errorStringU_ = CA2T(msg);
        CDataLoadingProblemsDlg dlg(errorStringU_);
		if(dlg.DoModal() == IDOK)
			return;
	}
	catch(exception &e)
	{
		GetData()->LogMessage(e.what() , CatLog::Severity::Error, CatLog::Category::Data);
        errorStringU_ = CA2T(e.what());
        CDataLoadingProblemsDlg dlg(errorStringU_);
		if(dlg.DoModal() == IDOK)
			return;
	}
	catch(...)
	{
        errorStringU_ = _TEXT("Unknown error while readin data file: ");
        GetData()->LogMessage(std::string(CT2A(errorStringU_)), CatLog::Severity::Error, CatLog::Category::Data);
        errorStringU_ += CA2T(theFileName.c_str());
        CDataLoadingProblemsDlg dlg(errorStringU_);
		if(dlg.DoModal() == IDOK)
			return ;
	}

}

// laittaa sivellintyökalun päälle/pois
void CSmartMetDoc::OnButtonBrushToolDlg()
{
	NFmiEditMapGeneralDataDoc * doc = GetData();
	if(doc)
	{
		if(doc->ModifyToolMode() != CtrlViewUtils::kFmiEditorModifyToolModeBrush)
		{
			doc->LogMessage("Opening brush tool.", CatLog::Severity::Info, CatLog::Category::Editing);

			doc->ModifyToolMode(CtrlViewUtils::kFmiEditorModifyToolModeBrush);
			if(!itsBrushToolDlg)
				CreateBrushToolDlg(itsData);
			itsBrushToolDlg->ShowWindow(SW_RESTORE);
		}
		else // Editor mode back to normal and hide dialog. This is not done when brush tool is closed from it's own window's close button!
		{
			doc->LogMessage("Closing brush tool.", CatLog::Severity::Info, CatLog::Category::Editing);
			doc->ModifyToolMode(CtrlViewUtils::kFmiEditorModifyToolModeNormal);
			if(itsBrushToolDlg)
				itsBrushToolDlg->ShowWindow(SW_HIDE);
		}
	}
}

void CSmartMetDoc::CreateBrushToolDlg(NFmiEditMapGeneralDataDoc* theDoc)
{
	if(itsBrushToolDlg)
		delete itsBrushToolDlg;

	POSITION pos = GetFirstViewPosition();	// annetaan aikasarjaikkunalle cview parentti-ikkunaksi näytön päivityksiä varten
	CView* pView = GetNextView(pos);		// ei toimi jos tulevaisuudessa vaihdetaan näyttöä kesken kaiken (esim. multi-doc-view)

	itsBrushToolDlg = new CFmiBrushToolDlg(&theDoc->GetSmartMetDocumentInterface(), pView);
	BOOL status = itsBrushToolDlg->Create();
}

void CSmartMetDoc::CreateLocationFinderDlg(NFmiEditMapGeneralDataDoc* theDoc)
{
	if(itsLocationFinderDlg)
		delete itsLocationFinderDlg;

	POSITION pos = GetFirstViewPosition();	// annetaan aikasarjaikkunalle cview parentti-ikkunaksi näytön päivityksiä varten
	CView* pView = GetNextView(pos);		// ei toimi jos tulevaisuudessa vaihdetaan näyttöä kesken kaiken (esim. multi-doc-view)

	itsLocationFinderDlg = new CFmiLocationFinderDlg(SmartMetDocumentInterface::GetSmartMetDocumentInterfaceImplementation(), pView);
	BOOL status = itsLocationFinderDlg->Create();
}

void CSmartMetDoc::OnUpdateButtonBrushToolDlg(CCmdUI* pCmdUI)
{
	if(GetData()->SmartMetEditingMode() == CtrlViewUtils::kFmiEditingModeNormal) // eli jos ollaan edit-moodissa, asetetaan nappulan tilaa
		pCmdUI->SetCheck(GetData()->ModifyToolMode() == CtrlViewUtils::kFmiEditorModifyToolModeBrush);
	else	// jos ollaan ns. view-moodissa, disabloidaan koko nappula
		pCmdUI->Enable(FALSE);
}

void CSmartMetDoc::OnButtonOpenSmartToolDlg()
{
    NFmiEditMapGeneralDataDoc * doc = GetData();
    if(doc)
    {
        doc->LogMessage("Opening smarttool dialog.", CatLog::Severity::Info, CatLog::Category::Editing);

        bool openNewSmarttoolDialog = false;

        if(openNewSmarttoolDialog)
        {
            if(!itsSmarttoolsTabControlDlg)
                CreateSmartToolsTabControlDlg(itsData);
            itsSmarttoolsTabControlDlg->ShowWindow(SW_RESTORE);
            itsSmarttoolsTabControlDlg->SetActiveWindow();
        }
        else
        {
            if(!itsSmartToolDlg)
                CreateSmartToolDlg(itsData);
            itsSmartToolDlg->ShowWindow(SW_RESTORE);
            itsSmartToolDlg->SetActiveWindow();
        }
    }
}

void CSmartMetDoc::CreateTempDlg()
{
    if(!itsTempDialog)
    {
        POSITION pos = GetFirstViewPosition();	// annetaan aikasarjaikkunalle cview parentti-ikkunaksi näytön päivityksiä varten
        CView* pView = GetNextView(pos);		// ei toimi jos tulevaisuudessa vaihdetaan näyttöä kesken kaiken (esim. multi-doc-view)

        itsTempDialog = new CFmiTempDlg(SmartMetDocumentInterface::GetSmartMetDocumentInterfaceImplementation(), pView);
        BOOL status = itsTempDialog->Create(IDD_TEMP_DIALOG, pView->GetDesktopWindow());
    }
}

void CSmartMetDoc::CreateSmartToolDlg(NFmiEditMapGeneralDataDoc* theDoc)
{
	if(itsSmartToolDlg)
		delete itsSmartToolDlg;

	POSITION pos = GetFirstViewPosition();	// annetaan aikasarjaikkunalle cview parentti-ikkunaksi näytön päivityksiä varten
	CView* pView = GetNextView(pos);		// ei toimi jos tulevaisuudessa vaihdetaan näyttöä kesken kaiken (esim. multi-doc-view)

	itsSmartToolDlg = new CFmiSmartToolDlg(SmartMetDocumentInterface::GetSmartMetDocumentInterfaceImplementation(), pView);
	BOOL status = itsSmartToolDlg->Create(IDD_DIALOG_SMART_TOOL, pView->GetDesktopWindow());
}

void CSmartMetDoc::CreateSmartToolsTabControlDlg(NFmiEditMapGeneralDataDoc* theDoc)
{
    if(itsSmarttoolsTabControlDlg)
        delete itsSmarttoolsTabControlDlg;

    POSITION pos = GetFirstViewPosition();	// annetaan aikasarjaikkunalle cview parentti-ikkunaksi näytön päivityksiä varten
    CView* pView = GetNextView(pos);		// ei toimi jos tulevaisuudessa vaihdetaan näyttöä kesken kaiken (esim. multi-doc-view)

    itsSmarttoolsTabControlDlg = new CFmiSmarttoolsTabControlDlg(SmartMetDocumentInterface::GetSmartMetDocumentInterfaceImplementation(), pView);
    BOOL status = itsSmarttoolsTabControlDlg->Create(pView->GetDesktopWindow());
}

void CSmartMetDoc::OnButtonRefresh()
{
	GetData()->OnButtonRefresh();
}

void CSmartMetDoc::OnButtonReloadAllDynamicHelpData()
{
	GetData()->ReloadAllDynamicHelpData();

	GetData()->LogMessage("Reloading all the dynamic data.", CatLog::Severity::Info, CatLog::Category::Data);
	GetData()->MapDirty(CtrlViewUtils::kDoAllMapViewDescTopIndex, true, true); // laitetaan kaikki kartta näytöt likaiseksi
	CFmiDataLoadingThread2::ResetTimeStamps();
	CFmiDataLoadingThread2::LoadDataNow();
	UpdateAllViewsAndDialogs("Reloading all the dynamic data");
}

void CSmartMetDoc::CaseStudyLoadingActions(const NFmiMetTime &theUsedTime, const std::string &updateReasonText)
{
	itsSynopDataGridViewDlg->MinMaxRangeStartTime(theUsedTime);
	CFmiDataLoadingThread2::SettingsChanged(*itsData->HelpDataInfoSystem(), true); // Datan lataus threadille laitetaan tässä uusi CaseStudy-helpDataInfoSetting-olio käyttöön
	CFmiDataLoadingThread2::ResetTimeStamps();
	CFmiDataLoadingThread2::LoadDataNow();
	GetData()->MapDirty(CtrlViewUtils::kDoAllMapViewDescTopIndex, true, true); // laitetaan kaikki kartta näytöt likaiseksi
	UpdateAllViewsAndDialogs(updateReasonText);
}

void CSmartMetDoc::CaseStudyToNormalModeActions(void)
{
	NFmiMetTime currentTime;
    CaseStudyLoadingActions(currentTime, "Coming back from case study mode to normal mode");
}

void CSmartMetDoc::SetAllViewIconsDynamically(void)
{
	HICON usedSmallIcon = 0;
	HICON usedBigIcon = 0;
    if(itsData->BetaProductionSystem().AutomationModeOn())
    {
        usedSmallIcon = CCloneBitmap::BitmapToIcon(IDB_BITMAP_FMI_LOGO_BETA_PRODUCT_MODE, ColorPOD(160, 160, 164));
        usedBigIcon = CCloneBitmap::BitmapToIcon(IDB_BITMAP_FMI_LOGO_BETA_PRODUCT_MODE, ColorPOD(160, 160, 164));
    }
    else if(itsData->CaseStudyModeOn())
	{
		usedSmallIcon = CCloneBitmap::BitmapToIcon(IDB_BITMAP_FMI_LOGO_CASE_STUDY, ColorPOD(160, 160, 164));
		usedBigIcon = CCloneBitmap::BitmapToIcon(IDB_BITMAP_FMI_LOGO_CASE_STUDY, ColorPOD(160, 160, 164));
	}
	else
	{
		usedSmallIcon = (HICON) ::LoadImage(::AfxGetResourceHandle(), MAKEINTRESOURCE(IDR_MAINFRAME), IMAGE_ICON, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), 0);
		usedBigIcon = (HICON) ::LoadImage(::AfxGetResourceHandle(), MAKEINTRESOURCE(IDR_MAINFRAME), IMAGE_ICON, ::GetSystemMetrics(SM_CXICON), ::GetSystemMetrics(SM_CYICON), 0);
	}

	CFmiWin32Helpers::SetWindowIconDynamically(AfxGetMainWnd(), usedSmallIcon, usedBigIcon);

	CFmiWin32Helpers::SetWindowIconDynamically(itsTimeSerialDataEditorDlg, usedSmallIcon, usedBigIcon);
	CFmiWin32Helpers::SetWindowIconDynamically(itsLocationSelectionToolDlg, usedSmallIcon, usedBigIcon);
	CFmiWin32Helpers::SetWindowIconDynamically(itsZoomDlg, usedSmallIcon, usedBigIcon);
	CFmiWin32Helpers::SetWindowIconDynamically(itsFilterDlg, usedSmallIcon, usedBigIcon);
	CFmiWin32Helpers::SetWindowIconDynamically(itsBrushToolDlg, usedSmallIcon, usedBigIcon);
	CFmiWin32Helpers::SetWindowIconDynamically(itsLocationFinderDlg, usedSmallIcon, usedBigIcon);
	CFmiWin32Helpers::SetWindowIconDynamically(itsSmartToolDlg, usedSmallIcon, usedBigIcon);
	CFmiWin32Helpers::SetWindowIconDynamically(itsTempDialog, usedSmallIcon, usedBigIcon);
	CFmiWin32Helpers::SetWindowIconDynamically(itsShortCutsDialog, usedSmallIcon, usedBigIcon);
	CFmiWin32Helpers::SetWindowIconDynamically(itsViewMacroDlg, usedSmallIcon, usedBigIcon);
	CFmiWin32Helpers::SetWindowIconDynamically(itsCrossSectionDlg, usedSmallIcon, usedBigIcon);
	CFmiWin32Helpers::SetWindowIconDynamically(itsSynopPlotSettingsDlg, usedSmallIcon, usedBigIcon);
	CFmiWin32Helpers::SetWindowIconDynamically(itsTrajectoryDlg, usedSmallIcon, usedBigIcon);
	CFmiWin32Helpers::SetWindowIconDynamically(itsWarningCenterDlg, usedSmallIcon, usedBigIcon);
	CFmiWin32Helpers::SetWindowIconDynamically(itsExtraMapViewDlg1, usedSmallIcon, usedBigIcon);
	CFmiWin32Helpers::SetWindowIconDynamically(itsExtraMapViewDlg2, usedSmallIcon, usedBigIcon);
	CFmiWin32Helpers::SetWindowIconDynamically(itsSeaIcingWarningsDlg, usedSmallIcon, usedBigIcon);
	CFmiWin32Helpers::SetWindowIconDynamically(itsWindTableDlg, usedSmallIcon, usedBigIcon);
	CFmiWin32Helpers::SetWindowIconDynamically(itsDataQualityCheckerDialog, usedSmallIcon, usedBigIcon);
	CFmiWin32Helpers::SetWindowIconDynamically(itsIgnoreStationsDlg, usedSmallIcon, usedBigIcon);
    CFmiWin32Helpers::SetWindowIconDynamically(itsCaseStudyDlg, usedSmallIcon, usedBigIcon);
    CFmiWin32Helpers::SetWindowIconDynamically(itsBetaProductDialog, usedSmallIcon, usedBigIcon);
    CFmiWin32Helpers::SetWindowIconDynamically(itsLogViewer, usedSmallIcon, usedBigIcon);
    CFmiWin32Helpers::SetWindowIconDynamically(itsParamAddingDlg, usedSmallIcon, usedBigIcon);
}

// piti tehdä uuden karttaruudukon valinnan lisäksi paikka mistä
// aukeaa vanha dialogi, koska citrix-linux yhdistelmällä uusi ruudukon valinta systeemi ei toimi
void CSmartMetDoc::OnMenuitemViewGridSelectionDlg2()
{
	NFmiEditMapGeneralDataDoc *doc = GetData();
	if(doc)
	{
		NFmiPoint oldSize(doc->MapViewDescTop(itsMapViewDescTopIndex)->ViewGridSize());
		CFmiViewGridSelectorDlg dlg(itsMapViewDescTopIndex, SmartMetDocumentInterface::GetSmartMetDocumentInterfaceImplementation());
		if(dlg.DoModal() == IDOK)
		{
			if(doc->SetMapViewGrid(itsMapViewDescTopIndex, dlg.SelectedGridSize()))
				UpdateAllViewsAndDialogs("Changing main map view grid");
		}
	}
}

void CSmartMetDoc::OnEditSpaceOut()
{
	GetData()->OnEditSpaceOut(itsMapViewDescTopIndex);
}

void CSmartMetDoc::OnButtonValidateData()
{
	if(GetData())
	{
		CWaitCursor cursor;
		GetData()->LogMessage("Validation checking to edited data.", CatLog::Severity::Info, CatLog::Category::Editing);
		GetData()->MakeDataValiditation();
		UpdateAllViewsAndDialogs("Edited data validitation modifications");
	}
}

void CSmartMetDoc::OnUpdateButtonValidateData(CCmdUI* pCmdUI)
{
	if(GetData()->SmartMetEditingMode() != CtrlViewUtils::kFmiEditingModeNormal) // jos ollaan ns. view-moodissa, disabloidaan koko nappula
		pCmdUI->Enable(FALSE);
}

void CSmartMetDoc::OnChangeMapType()
{
	GetData()->OnChangeMapType(itsMapViewDescTopIndex, true);
}
void CSmartMetDoc::OnUpdateButtonSelectEuropeMap(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(IsMapSelected(2));
}

void CSmartMetDoc::OnUpdateButtonSelectFinlandMap(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(IsMapSelected(0));
}

void CSmartMetDoc::OnUpdateButtonGlobe(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(IsMapSelected(3));
}

void CSmartMetDoc::OnShowTimeString()
{
	GetData()->OnShowTimeString(itsMapViewDescTopIndex);
}

void CSmartMetDoc::OnShowGridPoints()
{
	GetData()->OnShowGridPoints(itsMapViewDescTopIndex);
}

void CSmartMetDoc::OnToggleGridPointColor()
{
	GetData()->OnToggleGridPointColor(itsMapViewDescTopIndex);
}

void CSmartMetDoc::OnToggleGridPointSize()
{
	GetData()->OnToggleGridPointSize(itsMapViewDescTopIndex);
}

void CSmartMetDoc::OnToggleShowNamesOnMap()
{
	GetData()->OnToggleShowNamesOnMap(itsMapViewDescTopIndex, true);
}

// vaihtaaa "näytä maskit kartalla" -tilaa ja päivittää ruudut
void CSmartMetDoc::OnShowMasksOnMap()
{
	GetData()->OnShowMasksOnMap(itsMapViewDescTopIndex);
}

template<typename T>
static void SetViewPlaceToDefault(CSmartMetDoc *theDoc, T *theView, const std::string &updateMessage)
{
	if(theView)
	{
		theView->SetDefaultValues();
		theView->Update();
		theView->SetActiveWindow();
		theDoc->UpdateAllViewsAndDialogs(updateMessage);
	}
}

void CSmartMetDoc::OnViewSetTimeEditorPlaceToDefault()
{
    ::SetViewPlaceToDefault(this, itsTimeSerialDataEditorDlg, "Time serial view set to default size and position");
}

void CSmartMetDoc::OnViewSetMapView2PlaceToDefault(CFmiExtraMapViewDlg *theExtraMapViewDlg, const std::string &updateMessage)
{
    ::SetViewPlaceToDefault(this, theExtraMapViewDlg, updateMessage);
}

void CSmartMetDoc::OnViewSetMapView2PlaceToDefault()
{
	OnViewSetMapView2PlaceToDefault(itsExtraMapViewDlg1, "Map view 2 set to default size and position");
}

void CSmartMetDoc::OnViewSetMapView3PlaceToDefault()
{
	OnViewSetMapView2PlaceToDefault(itsExtraMapViewDlg2, "Map view 3 set to default size and position");
}

void CSmartMetDoc::OnViewSetTempViewPlaceToDefault()
{
    ::SetViewPlaceToDefault(this, itsTempDialog, "Sounding view set to default size and position");
}

void CSmartMetDoc::OnViewSetCrosssectionViewPlaceToDefault()
{
	if(itsCrossSectionDlg)
	{
		itsCrossSectionDlg->SetDefaultValues();
		itsCrossSectionDlg->Update();
		itsCrossSectionDlg->SetActiveWindow();
		UpdateAllViewsAndDialogs("Cross section view set to default size and position");
	}
}

void CSmartMetDoc::OnSetParamAddingDlgPlaceToDefault()
{
    ::SetViewPlaceToDefault(this, itsParamAddingDlg, "Param adding dialog set to default size and position");
}

void CSmartMetDoc::OnViewSetZoomViewPlaceToDefault()
{
    ::SetViewPlaceToDefault(this, itsZoomDlg, "Zooming dialog set to default size and position");
}

void CSmartMetDoc::OnViewSetSmarttoolViewPlaceToDefault()
{
    ::SetViewPlaceToDefault(this, itsSmartToolDlg, "SmartTool dialog set to default size and position");
}

void CSmartMetDoc::OnViewSetViewMacroDlgPlaceToDefault()
{
    ::SetViewPlaceToDefault(this, itsViewMacroDlg, "View macro dialog set to default size and position");
}

void CSmartMetDoc::OnViewSetSynopTextViewPlaceToDefault()
{
    ::SetViewPlaceToDefault(this, itsSynopDataGridViewDlg, "Station data grid view set to default size and position");
}

void CSmartMetDoc::OnViewSetTrajectoryViewPlaceToDefault()
{
	if(itsTrajectoryDlg)
	{
		itsTrajectoryDlg->SetDefaultValues();
		itsTrajectoryDlg->Update();
		itsTrajectoryDlg->SetActiveWindow();
		UpdateAllViewsAndDialogs("trajectory view set to default size and position");
	}
}

void CSmartMetDoc::OnViewSetDataQualityCheckerDlgPlaceToDefault()
{
	if(itsDataQualityCheckerDialog)
	{
		itsDataQualityCheckerDialog->SetDefaultValues();
		itsDataQualityCheckerDialog->SetActiveWindow();
	}
}

void CSmartMetDoc::OnFileOpen()
{
	OnDataLoadFromFile();
}

void CSmartMetDoc::OnFileNew()
{
// ei tehdä mitään/Marko
	int x = 0;
}

void CSmartMetDoc::OnButtonAnimation()
{
	GetData()->ToggleTimeControlAnimationView(itsMapViewDescTopIndex);
}

void CSmartMetDoc::OnUpdateButtonAnimation(CCmdUI* pCmdUI)
{
	NFmiEditMapGeneralDataDoc *doc = GetData();
	if(doc)
	{
//		pCmdUI->SetCheck(doc->DoAnimation());
	}
}

void CSmartMetDoc::OnButtonEditorControlPointMode()
{
	NFmiEditMapGeneralDataDoc *doc = GetData();
	if(doc)
	{
		doc->MetEditorOptionsData().ControlPointMode(!doc->MetEditorOptionsData().ControlPointMode());
		if(doc->MetEditorOptionsData().ControlPointMode())
			doc->LogMessage("Opening Control point tool.", CatLog::Severity::Info, CatLog::Category::Editing);
		else
			doc->LogMessage("Closing Control point tool.", CatLog::Severity::Info, CatLog::Category::Editing);

		doc->MapDirty(itsMapViewDescTopIndex, true, true);
		UpdateAllViewsAndDialogs("Control point mode changed");
	}
}

void CSmartMetDoc::OnUpdateButtonEditorControlPointMode(CCmdUI* pCmdUI)
{
	if(GetData()->SmartMetEditingMode() == CtrlViewUtils::kFmiEditingModeNormal) // eli jos ollaan edit-moodissa, asetetaan nappulan tilaa
	{
		NFmiEditMapGeneralDataDoc *doc = GetData();
		if(doc)
		{
			pCmdUI->SetCheck(doc->MetEditorOptionsData().ControlPointMode());
		}
	}
	else	// jos ollaan ns. view-moodissa, disabloidaan koko nappula
		pCmdUI->Enable(FALSE);

}

void CSmartMetDoc::OnButtonDelete()
{
	NFmiEditMapGeneralDataDoc *doc = GetData();
	if(doc && doc->MetEditorOptionsData().ControlPointMode())
	{
		doc->CPManager()->RemoveCP();
		doc->MapDirty(itsMapViewDescTopIndex, true, true);
		GetData()->LogMessage("Deleting control point.", CatLog::Severity::Debug, CatLog::Category::Editing);
		UpdateAllViewsAndDialogs("Deleting control point");
	}
}

void CSmartMetDoc::MakeViewsDirty(void)
{
	GetData()->MapDirty(itsMapViewDescTopIndex, true, true);
}

void CSmartMetDoc::OnDataStoreViewToPictureFile()
{
	CBitmap* bm = ApplicationInterface::GetSmartMetView()->MemoryBitmap();
	BOOL status = SaveViewToFilePicture("CSmartMetDoc::StoreMainMapViewImage", bm, &(GetData()->MapViewDescTop(itsMapViewDescTopIndex)->RelativeMapRect()));
	if(status)
		GetData()->LogMessage("Stored map view to image file.", CatLog::Severity::Info, CatLog::Category::Operational);
	else
		GetData()->LogMessage("Storing map view to image file failed.", CatLog::Severity::Error, CatLog::Category::Operational);
}

void CSmartMetDoc::OnUpdateDataStoreViewToPictureFile(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(TRUE);
}

void CSmartMetDoc::OnDataStoreExtraMapViewToPictureFile(CFmiExtraMapViewDlg *theExtraMapViewDlg)
{
	if(theExtraMapViewDlg)
	{
		CBitmap* bm = theExtraMapViewDlg->MemoryBitmap();
		BOOL status = SaveViewToFilePicture(__FUNCTION__, bm, &(GetData()->MapViewDescTop(theExtraMapViewDlg->MapViewDescTopIndex())->RelativeMapRect()));
		if(status)
			GetData()->LogMessage("Stored map view to image file.", CatLog::Severity::Info, CatLog::Category::Operational);
		else
			GetData()->LogMessage("Storing map view to image file failed.", CatLog::Severity::Error, CatLog::Category::Operational);
	}
}

void CSmartMetDoc::OnDataStoreMapView2ToPictureFile()
{
	OnDataStoreExtraMapViewToPictureFile(itsExtraMapViewDlg1);
}

void CSmartMetDoc::OnDataStoreMapView3ToPictureFile()
{
	OnDataStoreExtraMapViewToPictureFile(itsExtraMapViewDlg2);
}

BOOL CSmartMetDoc::SaveViewToFilePicture(const std::string &theCallingFunctionName, CBitmap* bm, const NFmiRect *theRelativeOutputArea)
{
	return CFmiGdiPlusHelpers::SaveMfcBitmapToFile(theCallingFunctionName, bm, SmartMetDocumentInterface::GetSmartMetDocumentInterfaceImplementation(), theRelativeOutputArea);
}

void CSmartMetDoc::OnHelpShortCuts()
{
	static bool firstTime = true;
	if(!itsShortCutsDialog)
		CreateShortCutsDlg();
	itsShortCutsDialog->ShowWindow(SW_SHOW);	// Vaihdoin SW_RESTOREN, muistaa ikkunan muutetun koon.
	itsShortCutsDialog->SetActiveWindow();
	if(firstTime)
	{
		// Kun 1. kerran avataan shortcuts dialogi, pitää siinä oleva teksti
		// deselectoida, koska se on jostain mfc-jutun takia selectoitu.
		firstTime = false;
		itsShortCutsDialog->DeselectTexts();
	}
	GetData()->LogMessage("Opening shortcuts dialog.", CatLog::Severity::Info, CatLog::Category::Operational);
}

void CSmartMetDoc::OnShowProjectionLines()
{
	GetData()->OnShowProjectionLines();
}

void CSmartMetDoc::OnMenuitemProjectionLineSetup()
{
	NFmiEditMapGeneralDataDoc * doc = GetData();
	if(doc)
	{
		NFmiProjectionCurvatureInfo* projInfo = doc->ProjectionCurvatureInfo();
		if(projInfo)
		{
			CFmiProjectionLineSetupDlg dlg(projInfo);
			if(dlg.DoModal() == IDOK)
			{
				projInfo->StoreToSettings();
				doc->MapDirty(itsMapViewDescTopIndex, true, true);
				UpdateAllViewsAndDialogs("Projection line drawing setup changed");
			}
		}
	}
}

void CSmartMetDoc::OnButtonViewSelectParamDialog()
{
    ActivateParamAddingDlg();
}

void CSmartMetDoc::OnDataStoreTimeserialviewToPictureFile()
{
	if(itsTimeSerialDataEditorDlg)
	{
		CBitmap* bm = itsTimeSerialDataEditorDlg->ViewMemoryBitmap();
		BOOL status = SaveViewToFilePicture(__FUNCTION__, bm, 0);
		if(status)
			GetData()->LogMessage("Storing image from time serial view to file.", CatLog::Severity::Info, CatLog::Category::Operational);
		else
			GetData()->LogMessage("Storing image from time serial view to file failed.", CatLog::Severity::Error, CatLog::Category::Operational);
	}
}

void CSmartMetDoc::OnDataStoreTrajectoryViewToPictureFile()
{
	if(itsTrajectoryDlg)
	{
		CBitmap* bm = itsTrajectoryDlg->MemoryBitmap();
		BOOL status = SaveViewToFilePicture(__FUNCTION__, bm, 0);
		if(status)
			GetData()->LogMessage("Storing trajectory-view image to file.", CatLog::Severity::Info, CatLog::Category::Operational);
		else
			GetData()->LogMessage("Storing trajectory-view image to file failed.", CatLog::Severity::Error, CatLog::Category::Operational);
	}
}

void CSmartMetDoc::OnMenuitemGriddingOptions()
{
	NFmiEditMapGeneralDataDoc* doc = GetData();
	if(doc)
	{
		NFmiCPGriddingProperties options(doc->CPGriddingProperties());
		CFmiGriddingOptionsDlg dlg(&options);
		if(dlg.DoModal() == IDOK)
		{
			doc->CPGriddingProperties(options);
			doc->StoreCPGriddingProperties();
			doc->MapDirty(itsMapViewDescTopIndex, true, true);
			UpdateAllViewsAndDialogs("Used ToolMaster gridding function changed");
		}
	}

}

void CSmartMetDoc::OnUpdateButtonFilterDialog(CCmdUI* pCmdUI)
{
	if(GetData()->SmartMetEditingMode() != CtrlViewUtils::kFmiEditingModeNormal) // jos ollaan ns. view-moodissa, disabloidaan koko nappula
		pCmdUI->Enable(FALSE);
}

void CSmartMetDoc::OnButtonTempDlg()
{
	if(itsData)
	{
		if(!itsTempDialog)
			CreateTempDlg();
		if(itsData->GetMTATempSystem().TempViewOn())
		{ // jos luotaus dialogi oli päällä, laitetaan se pois päältä
			itsTempDialog->ShowWindow(SW_HIDE);
			itsData->LogMessage("Opening sounding view.", CatLog::Severity::Info, CatLog::Category::Operational);
		}
		else
		{ // muuten laitetaan se päälle
			itsTempDialog->ShowWindow(SW_SHOW);	// Vaihdoin SW_RESTOREN, muistaa ikkunan muutetun koon.
			itsTempDialog->SetActiveWindow();
			itsData->LogMessage("Closing sounding view.", CatLog::Severity::Info, CatLog::Category::Operational);
		}
		itsData->GetMTATempSystem().TempViewOn(!itsData->GetMTATempSystem().TempViewOn());
		itsData->GetMTATempSystem().ShowMapMarkers(itsData->GetMTATempSystem().TempViewOn());
		// päivitetään kartta ja muutkin näytöt, koska luotaus asemien kolmioiden kartta piirto riippuu tästä
		itsData->AreaViewDirty(itsMapViewDescTopIndex, false, true); // laitetaan viela kaikki ajat likaisiksi cachesta
        ApplicationInterface::GetApplicationInterfaceImplementation()->RefreshApplicationViewsAndDialogs("CSmartMetDoc: Opening sounding view", TRUE);
	}
}

void CSmartMetDoc::OnUpdateButtonTempDlg(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(itsData->GetMTATempSystem().TempViewOn());
}

void CSmartMetDoc::OnButtonShowCrossSection()
{
	if(itsData)
	{
		if(!itsCrossSectionDlg)
			CreateCrossSectionDlg();

		if(itsData->CrossSectionSystem()->CrossSectionViewOn())
		{ // poikkileikkaus näyttö oli päällä, laitetaan se pois päältä
			itsCrossSectionDlg->ShowWindow(SW_HIDE);
			itsData->CrossSectionSystem()->CrossSectionSystemActive(false);
			itsData->LogMessage("Opening cross section view.", CatLog::Severity::Info, CatLog::Category::Operational);
		}
		else
		{ // laitetaan poikkileikkuas ikkuna päälle
			itsCrossSectionDlg->ShowWindow(SW_SHOW);	// Vaihdoin SW_RESTOREN, muistaa ikkunan muutetun koon.
			itsData->CrossSectionSystem()->CrossSectionSystemActive(true);
			itsCrossSectionDlg->SetActiveWindow();
			itsData->LogMessage("Closing cross section view.", CatLog::Severity::Info, CatLog::Category::Operational);
		}
		itsData->CrossSectionSystem()->CrossSectionViewOn(!itsData->CrossSectionSystem()->CrossSectionViewOn());
		itsData->MapDirty(itsMapViewDescTopIndex, true, true);
        ApplicationInterface::GetApplicationInterfaceImplementation()->RefreshApplicationViewsAndDialogs("CSmartMetDoc: Opening cross section view", TRUE);
	}
}

void CSmartMetDoc::OnUpdateButtonShowCrossSection(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(itsData->CrossSectionSystem()->CrossSectionViewOn());
}

void CSmartMetDoc::OnButtonObservationComparisonMode()
{
	GetData()->ObsComparisonInfo().NextComparisonMode();
	GetData()->MapDirty(itsMapViewDescTopIndex, true, true);
	GetData()->LogMessage("Toggling observation comparison mode.", CatLog::Severity::Info, CatLog::Category::Operational);
	UpdateAllViewsAndDialogs("Changed Obs comparison mode");
}

void CSmartMetDoc::OnUpdateButtonObservationComparisonMode(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(GetData()->ObsComparisonInfo().ComparisonMode() > 0);
}

void CSmartMetDoc::OnButtonSynopDataGridView()
{
	if(itsData)
	{
		if(itsData->SynopDataGridViewOn() == false)
		{
			if(!itsSynopDataGridViewDlg)
				CreateSynopDataGridViewDlg(itsData);
			itsSynopDataGridViewDlg->ShowWindow(SW_SHOW);	// Vaihdoin SW_RESTOREN, muistaa ikkunan muutetun koon.
			itsSynopDataGridViewDlg->SetActiveWindow();
			GetData()->LogMessage("Opening synop data table view.", CatLog::Severity::Info, CatLog::Category::Operational);
		}
		else
		{ // jos dialogi oli päällä, laitetaan se pois
			itsSynopDataGridViewDlg->ShowWindow(SW_HIDE);	// Vaihdoin SW_RESTOREN, muistaa ikkunan muutetun koon.
			GetData()->LogMessage("Closing synop data table view.", CatLog::Severity::Info, CatLog::Category::Operational);
		}
		itsData->SynopDataGridViewOn(!itsData->SynopDataGridViewOn());
		if(itsData->SynopDataGridViewOn())
			itsSynopDataGridViewDlg->Update();

	}
}

void CSmartMetDoc::OnUpdateSynopDataGridView(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(GetData()->SynopDataGridViewOn());
}

void CSmartMetDoc::OnAcceleratorObsComparisonChangeSymbol()
{
	GetData()->ObsComparisonInfo().NextSymbolType();
	GetData()->MapDirty(itsMapViewDescTopIndex, true, true);
	UpdateAllViewsAndDialogs("Changed Obs comparison mode symbol");
}

void CSmartMetDoc::OnAcceleratorObsComparisonChangeSymbolSize()
{
	GetData()->ObsComparisonInfo().NextSymbolSize();
	GetData()->MapDirty(itsMapViewDescTopIndex, true, true);
	UpdateAllViewsAndDialogs("Changed Obs comparison symbol size");
}

void CSmartMetDoc::OnAcceleratorObsComparisonToggleBorderDraw()
{
	GetData()->ObsComparisonInfo().DrawBorders(!GetData()->ObsComparisonInfo().DrawBorders());
	GetData()->MapDirty(itsMapViewDescTopIndex, true, true);
	UpdateAllViewsAndDialogs("Changed Obs comparison symbol border draw mode");
}

void CSmartMetDoc::OnButtonShowSynopPlotSettings()
{
	if(!itsSynopPlotSettingsDlg)
		CreateSynopPlotSettingsDlg(itsData);
	itsSynopPlotSettingsDlg->Update();
	itsSynopPlotSettingsDlg->ShowWindow(SW_SHOW);	// Vaihdoin SW_RESTOREN, muistaa ikkunan muutetun koon.
	itsSynopPlotSettingsDlg->SetActiveWindow();

	GetData()->LogMessage("Opening synop plot settings dialog.", CatLog::Severity::Info, CatLog::Category::Operational);
}

void CSmartMetDoc::OnDataStoreTempviewToPictureFile()
{
	if(itsTempDialog)
	{
		CBitmap* bm = itsTempDialog->ViewMemoryBitmap();
		BOOL status = SaveViewToFilePicture(__FUNCTION__, bm, 0);
		if(status)
			GetData()->LogMessage("Store image from sounding view to file.", CatLog::Severity::Info, CatLog::Category::Operational);
		else
			GetData()->LogMessage("Storing image from sounding view to file failed.", CatLog::Severity::Error, CatLog::Category::Operational);
	}
}

void CSmartMetDoc::OnDataStoreCrosssectionviewToPictureFile()
{
	if(itsCrossSectionDlg)
	{
		CBitmap* bm = itsCrossSectionDlg->MemoryBitmap();
		BOOL status = SaveViewToFilePicture(__FUNCTION__, bm, 0);
		if(status)
			GetData()->LogMessage("Store image from cross section view to file.", CatLog::Severity::Info, CatLog::Category::Operational);
		else
			GetData()->LogMessage("Storing image from cross section view to file failed.", CatLog::Severity::Error, CatLog::Category::Operational);
	}
}

void CSmartMetDoc::OnToggleLandBorderDrawColor()
{
	GetData()->OnToggleLandBorderDrawColor(itsMapViewDescTopIndex);
}

void CSmartMetDoc::OnToggleLandBorderPenSize()
{
	GetData()->OnToggleLandBorderPenSize(itsMapViewDescTopIndex);
}

void CSmartMetDoc::OnAcceleratorBorrowParams1()
{
	GetData()->OnAcceleratorBorrowParams(itsMapViewDescTopIndex, 1);
}
void CSmartMetDoc::OnAcceleratorBorrowParams2()
{
	GetData()->OnAcceleratorBorrowParams(itsMapViewDescTopIndex, 2);
}
void CSmartMetDoc::OnAcceleratorBorrowParams3()
{
	GetData()->OnAcceleratorBorrowParams(itsMapViewDescTopIndex, 3);
}
void CSmartMetDoc::OnAcceleratorBorrowParams4()
{
	GetData()->OnAcceleratorBorrowParams(itsMapViewDescTopIndex, 4);
}
void CSmartMetDoc::OnAcceleratorBorrowParams5()
{
	GetData()->OnAcceleratorBorrowParams(itsMapViewDescTopIndex, 5);
}

static CRect NFmiRect2CRect(const NFmiRect &theRect)
{
	return CRect(static_cast<int>(theRect.Left()), static_cast<int>(theRect.Top()), static_cast<int>(theRect.Right()), static_cast<int>(theRect.Bottom()));
}

// Function checks if loaded viewMacro actually uses this view. And if view is nullptr, this creates it with given creation function.
// Return true if view existed or it was successfully created for showing.
template<typename MacroViewInfo, typename ViewType, typename ViewCreationFunction>
static bool GenerateViewIfNeededAndContinueToInitializations(const MacroViewInfo &macroViewInfo, ViewType **view, ViewCreationFunction &viewCreationFunction)
{
    if(*view)
        return true;
    else
    {
        if(macroViewInfo.ViewStatus().ShowWindow())
        {
            viewCreationFunction();
            return (*view != nullptr);
        }
        else
            return false;
    }
}

void CSmartMetDoc::LoadViewMacroSettingsForWarningCenterView(NFmiViewSettingMacro &theViewMacro, int &theStartCornerCounter)
{
    // ** hälytyskeskusnäytön sijainti **
    auto createViewFunction = [this]() {this->CreateWarningCenterDlg(this->itsData); };
    if(::GenerateViewIfNeededAndContinueToInitializations(theViewMacro.GetWarningCenterView(), &itsWarningCenterDlg, createViewFunction))
    {
        CFmiWin32Helpers::SetWindowSettings(itsWarningCenterDlg, ::NFmiRect2CRect(theViewMacro.GetWarningCenterView().AbsolutRect()), theViewMacro.GetWarningCenterView().ViewStatus(), theStartCornerCounter);
        itsWarningCenterDlg->LoadViewMacroSettingsFromDocument();
        itsWarningCenterDlg->Update();
    }
}

void CSmartMetDoc::LoadViewMacroSettingsForTrajectoryDlg(NFmiViewSettingMacro &theViewMacro, int &theStartCornerCounter)
{
    // ** trajektorinäytön sijainti **
    auto createViewFunction = [this]() {this->CreateTrajectoryDlg(this->itsData); };
    if(::GenerateViewIfNeededAndContinueToInitializations(theViewMacro.GetTrajectoryView(), &itsTrajectoryDlg, createViewFunction))
    {
        CFmiWin32Helpers::SetWindowSettings(itsTrajectoryDlg, ::NFmiRect2CRect(theViewMacro.GetTrajectoryView().AbsolutRect()), theViewMacro.GetTrajectoryView().ViewStatus(), theStartCornerCounter);
        itsTrajectoryDlg->InitFromTrajectorySystem();
    }
}

void CSmartMetDoc::LoadViewMacroSettingsForSynopDataGridView(NFmiViewSettingMacro &theViewMacro, int &theStartCornerCounter)
{
    // ** SynopDataGridView sijainti ja asetukset **
    auto createViewFunction = [this]() {this->CreateSynopDataGridViewDlg(this->itsData); };
    if(::GenerateViewIfNeededAndContinueToInitializations(theViewMacro.GetSynopDataGridView(), &itsSynopDataGridViewDlg, createViewFunction))
    {
        CFmiWin32Helpers::SetWindowSettings(itsSynopDataGridViewDlg, ::NFmiRect2CRect(theViewMacro.GetSynopDataGridView().AbsolutRect()), theViewMacro.GetSynopDataGridView().ViewStatus(), theStartCornerCounter);
        itsSynopDataGridViewDlg->ApplySynopDataGridViewMacro(theViewMacro.GetSynopDataGridView());
        itsSynopDataGridViewDlg->Update();
    }
}

void CSmartMetDoc::LoadViewMacroSettingsForTempDialog(NFmiViewSettingMacro &theViewMacro, int &theStartCornerCounter)
{
    // ** luotausnäytön sijainti **
    auto createViewFunction = [this]() {this->CreateTempDlg(); };
    if(::GenerateViewIfNeededAndContinueToInitializations(theViewMacro.GetTempView(), &itsTempDialog, createViewFunction))
    {
        CFmiWin32Helpers::SetWindowSettings(itsTempDialog, ::NFmiRect2CRect(theViewMacro.GetTempView().AbsolutRect()), theViewMacro.GetTempView().ViewStatus(), theStartCornerCounter);
        itsTempDialog->Update();
    }
}

void CSmartMetDoc::LoadViewMacroSettingsForCrossSectionDlg(NFmiViewSettingMacro &theViewMacro, int &theStartCornerCounter)
{
    // ** poikkileikkausnäytön sijainti **
    auto createViewFunction = [this]() {this->CreateCrossSectionDlg(); };
    if(::GenerateViewIfNeededAndContinueToInitializations(theViewMacro.GetCrossSectionView(), &itsCrossSectionDlg, createViewFunction))
    {
        CFmiWin32Helpers::SetWindowSettings(itsCrossSectionDlg, ::NFmiRect2CRect(theViewMacro.GetCrossSectionView().AbsolutRect()), theViewMacro.GetCrossSectionView().ViewStatus(), theStartCornerCounter);
        itsCrossSectionDlg->InitFromCrossSectionSystem();
    }
}

void CSmartMetDoc::LoadViewMacroSettingsForExtraMapViewDlg(CFmiExtraMapViewDlg **extraMapViewDlg, unsigned int theMapViewDescTopIndex, NFmiViewSettingMacro &theViewMacro, int &theStartCornerCounter)
{
    // ** Extra MapViewdescTop nr. 1 ja 2 sijainti ja asetukset **
    NFmiViewSettingMacro::MapViewDescTop &mapViewDescTop = theViewMacro.ExtraMapViewDescTops()[theMapViewDescTopIndex];
    auto createViewFunction = [this, theMapViewDescTopIndex, extraMapViewDlg]() {*extraMapViewDlg = this->CreateExtraMapViewDlg(this->itsData, theMapViewDescTopIndex); };
    if(::GenerateViewIfNeededAndContinueToInitializations(mapViewDescTop, extraMapViewDlg, createViewFunction))
    {
        CFmiWin32Helpers::SetWindowSettings(*extraMapViewDlg, ::NFmiRect2CRect(mapViewDescTop.AbsolutRect()), mapViewDescTop.ViewStatus(), theStartCornerCounter);
        (*extraMapViewDlg)->Update();
    }
}

void CSmartMetDoc::LoadViewMacroSettingsForTimeSerialDataView(NFmiViewSettingMacro &theViewMacro, int &theStartCornerCounter)
{
    // ** aikasarjanäytön sijainti **
    auto createViewFunction = [this]() {this->CreateTimeEditor(true); };
    if(::GenerateViewIfNeededAndContinueToInitializations(theViewMacro.GetTimeView(), &itsTimeSerialDataEditorDlg, createViewFunction))
    {
        CFmiWin32Helpers::SetWindowSettings(itsTimeSerialDataEditorDlg, ::NFmiRect2CRect(theViewMacro.GetTimeView().AbsolutRect()), theViewMacro.GetTimeView().ViewStatus(), theStartCornerCounter);
        GetData()->TimeSerialDataViewOn(theViewMacro.GetTimeView().ViewStatus().ShowWindow());
        itsTimeSerialDataEditorDlg->fUseMaskInTimeSerialViews = theViewMacro.GetMaskSettings().UseMasksInTimeSerialViews();
        itsTimeSerialDataEditorDlg->UpdateData(FALSE);
    }
}

void CSmartMetDoc::LoadViewMacroWindowsSettings(NFmiViewSettingMacro &theViewMacro)
{
	NFmiEditMapGeneralDataDoc* doc = GetData();
	if(doc)
	{
		if(doc->MetEditorOptionsData().DisableWindowManipulations())
			return ;
		try
		{
			if(theViewMacro.ViewMacroWasCorrupted())
				return ; // ei tehdä mitään, jos korruptoitunut viewMacro
			int startCornerCounter = 0;
			// ** kartta eli mainframen sijainti **
			NFmiViewSettingMacro::MapViewDescTop &mapViewDescTop0 = theViewMacro.ExtraMapViewDescTops()[0];
			CFmiWin32Helpers::SetWindowSettings(AfxGetMainWnd(), ::NFmiRect2CRect(mapViewDescTop0.AbsolutRect()), mapViewDescTop0.ViewStatus(), startCornerCounter);

            LoadViewMacroSettingsForTimeSerialDataView(theViewMacro, startCornerCounter);
            LoadViewMacroSettingsForTempDialog(theViewMacro, startCornerCounter);

			if(theViewMacro.OriginalLoadVersionNumber() > 1.)
			{ // tietyt asetukset tehdään vain jos originaali macro-versio on tarpeeksi suuri
                LoadViewMacroSettingsForTrajectoryDlg(theViewMacro, startCornerCounter);
                LoadViewMacroSettingsForCrossSectionDlg(theViewMacro, startCornerCounter);

				// ** synop-plot asetukset **
                if(!itsSynopPlotSettingsDlg)
                    CreateSynopPlotSettingsDlg(doc);
                itsSynopPlotSettingsDlg->InitFromDoc();

				// ** obs-comparison asetukset **
				// ei obscomparison näyttö asetuksia

                LoadViewMacroSettingsForWarningCenterView(theViewMacro, startCornerCounter);
                LoadViewMacroSettingsForSynopDataGridView(theViewMacro, startCornerCounter);
                LoadViewMacroSettingsForExtraMapViewDlg(&itsExtraMapViewDlg1, 1, theViewMacro, startCornerCounter);
                LoadViewMacroSettingsForExtraMapViewDlg(&itsExtraMapViewDlg2, 2, theViewMacro, startCornerCounter);
			}
            MakeViewActivationAfterLoadingViewMacro();
        }
		catch(exception &e)
		{
			std::string errStr("Unable to load current view-macro, reason: \n");
			errStr += e.what();
			// pitäisikö raportoida message boxin avulla?
			doc->LogAndWarnUser(errStr, "ViewMacro loading problem", CatLog::Severity::Error, CatLog::Category::Macro, false);
		}
	}
}

void CSmartMetDoc::MakeViewActivationAfterLoadingViewMacro()
{
    // Lopuksi aktivoidaan viewMacro-dialogi, koska se on jäänyt luultavasti kaikkien ikkunoiden alle,
    // tai jos sitä ei ole edes luotu vielä (kun ladataan esim. backup-macro viime ajosta) niin pääikkuna (kuten ennen tehtiin)
    if(itsViewMacroDlg)
        itsViewMacroDlg->SetActiveWindow();
    else
        AfxGetMainWnd()->SetActiveWindow();
}

// CTRL+I avaa nykyään vain näyttömakro dialogin, ei muuta, tässä vanha harhaan johtava nimi
void CSmartMetDoc::OnAcceleratorStoreViewMacro()
{
	if(!itsViewMacroDlg)
		CreateViewMacroDlg(GetData()); // pakko luoda tässä, koska muuten dialogin hndl jää nollaksi, nyt sitten jää leijumaan päänäytön päälle
	itsViewMacroDlg->ShowWindow(SW_SHOW);	// Vaihdoin SW_RESTOREN, muistaa ikkunan muutetun koon.
	itsViewMacroDlg->SetActiveWindow();
	GetData()->LogMessage("Opening view macro dialog.", CatLog::Severity::Info, CatLog::Category::Operational);
}

template<class Tview>
static void GetWindowSettings(NFmiApplicationWinRegistry &theApplicationWinRegistry, Tview *window, NFmiRect &theRect, bool &fShowWindow, unsigned int theMapViewDescTopIndex, UINT &showCommand)
{
    if(window)
    {
        WINDOWPLACEMENT wndpl;
        wndpl.length = sizeof(WINDOWPLACEMENT);
        BOOL bRet = window->GetWindowPlacement(&wndpl); // gets current window position and iconized/maximized status
        NFmiRect rect(wndpl.rcNormalPosition.left,
            wndpl.rcNormalPosition.top,
            wndpl.rcNormalPosition.right,
            wndpl.rcNormalPosition.bottom);
        theRect = rect;
        fShowWindow = window->IsWindowVisible() != 0;
        showCommand = wndpl.showCmd;
    }
    else
    {
        // Given window is not created yet, get window rect from registry
        CRect windowRect = Persist2::GetWindowRectFromWinRegistry(theApplicationWinRegistry, Tview::MakeUsedWinRegistryKeyStr(theMapViewDescTopIndex));
        fShowWindow = FALSE;
    }
}

// Haluamme tallettaa ikkunasta minimized/maximized tilan, mutta emme muita tiloja.
// Aiemmin emme välittäneet min/max tiloista, mutta nyt niin halutaan. En halua kuitenkaan käyttää muita tiloja,
// koska en tiedä sivuvaikutuksia (erilaisia tiloja on n. 10 kpl). Tämä mahdollistaa sen, että jos showCommand on 0, 
// toimitaan kuten ennenkin (se arvo on nykyään jokaisessa näyttömakrossa), muuten palautetaan showCommand arvo.
static bool IsViewShowCommandStoredInViewMacro(UINT showCommand)
{
    if(CFmiWin32Helpers::IsShowCommandMinimized(showCommand) || CFmiWin32Helpers::IsShowCommandMaximized(showCommand))
        return true;
    else
        return false;
}

template<typename  View, typename MacroViewSettings>
void StoreViewRectToMacro(View *view, MacroViewSettings &macroSettings, NFmiApplicationWinRegistry &applicationWinRegistry, unsigned int theMapViewDescTopIndex)
{
    NFmiRect windowRect;
    bool showWindow = false;
    UINT showCommand = 0;
    ::GetWindowSettings(applicationWinRegistry, view, windowRect, showWindow, theMapViewDescTopIndex, showCommand);
    macroSettings.AbsolutRect(windowRect);
    macroSettings.ViewStatus().ShowWindow(showWindow);
    if(::IsViewShowCommandStoredInViewMacro(showCommand))
        macroSettings.ViewStatus().ShowCommand(showCommand);
    else
        macroSettings.ViewStatus().ShowCommand(0);
}

// tämä on hankala, tätä kutsutaan viewwmakro-dialogista zeditmapview-otuksen kautta
void CSmartMetDoc::StoreViewMacroWindowsSettings(NFmiViewSettingMacro &theViewMacro)
{
	NFmiRect windowRect;
	bool showWindow = false;

	std::vector<NFmiViewSettingMacro::MapViewDescTop>& extraMapViewDescTops = theViewMacro.ExtraMapViewDescTops();
	extraMapViewDescTops.resize(GetData()->MapViewDescTopList().size());
    auto &applicationWinRegistry = GetData()->ApplicationWinRegistry();

	// ** pääkartta eli mainframen sijainti **
    StoreViewRectToMacro(dynamic_cast<CMainFrame*>(AfxGetMainWnd()), extraMapViewDescTops[0], applicationWinRegistry, 0);

	// ** aikasarjanäytön sijainti **
    StoreViewRectToMacro(itsTimeSerialDataEditorDlg, theViewMacro.GetTimeView(), applicationWinRegistry, 0);
	// ** luotausnäytön sijainti **
    StoreViewRectToMacro(itsTempDialog, theViewMacro.GetTempView(), applicationWinRegistry, 0);
	// ** trajektorinäytön sijainti **
    if(itsTrajectoryDlg)
    	itsTrajectoryDlg->UpdateTrajectorySystem(); // näin otetaan trajektori-dialogissa tehdyt muutokset mukaan
    StoreViewRectToMacro(itsTrajectoryDlg, theViewMacro.GetTrajectoryView(), applicationWinRegistry, 0);
	// ** poikkileikkausnäytön sijainti **
    StoreViewRectToMacro(itsCrossSectionDlg, theViewMacro.GetCrossSectionView(), applicationWinRegistry, 0);

	// ** synop-plot asetukset **
	// ei ole mitään näyttö asetuksia

	// ** obs-comparison asetukset **
	// ei ole mitään näyttö asetuksia

	// ** hälytyskeskusnäytön sijainti **
    StoreViewRectToMacro(itsWarningCenterDlg, theViewMacro.GetWarningCenterView(), applicationWinRegistry, 0);

	// ** SynopDataGridView sijainti ja asetukset **
    StoreViewRectToMacro(itsSynopDataGridViewDlg, theViewMacro.GetSynopDataGridView(), applicationWinRegistry, 0);
    if(itsSynopDataGridViewDlg)
    	itsSynopDataGridViewDlg->FillSynopDataGridViewMacro(theViewMacro.GetSynopDataGridView());

	// ** Extra MapViewdescTop nr. 1 sijainti ja asetukset **
    NFmiViewSettingMacro::MapViewDescTop &mapViewDescTop1 = extraMapViewDescTops[1];
    StoreViewRectToMacro(itsExtraMapViewDlg1, extraMapViewDescTops[1], applicationWinRegistry, 2);
	// ** Extra MapViewdescTop nr. 2 sijainti ja asetukset **
    StoreViewRectToMacro(itsExtraMapViewDlg2, extraMapViewDescTops[2], applicationWinRegistry, 3);
}

void CSmartMetDoc::OnUpdateButtonDataToDatabase(CCmdUI *pCmdUI)
{
	if(GetData()->SmartMetEditingMode() != CtrlViewUtils::kFmiEditingModeNormal) // jos ollaan ns. view-moodissa, disabloidaan koko nappula
		pCmdUI->Enable(FALSE);
}

void CSmartMetDoc::OnUpdateButtonStoreData(CCmdUI *pCmdUI)
{
	if(GetData()->SmartMetEditingMode() != CtrlViewUtils::kFmiEditingModeNormal) // jos ollaan ns. view-moodissa, disabloidaan koko nappula
		pCmdUI->Enable(FALSE);
}

void CSmartMetDoc::OnUpdateButtonSelectionToolDlg(CCmdUI *pCmdUI)
{
	if(GetData()->SmartMetEditingMode() != CtrlViewUtils::kFmiEditingModeNormal) // jos ollaan ns. view-moodissa, disabloidaan koko nappula
		pCmdUI->Enable(FALSE);
}

void CSmartMetDoc::OnUpdateSelectAll(CCmdUI *pCmdUI)
{
	if(GetData()->SmartMetEditingMode() != CtrlViewUtils::kFmiEditingModeNormal) // jos ollaan ns. view-moodissa, disabloidaan koko nappula
		pCmdUI->Enable(FALSE);
}

void CSmartMetDoc::OnUpdateMenuitemSaveAs(CCmdUI *pCmdUI)
{
	if(GetData()->SmartMetEditingMode() != CtrlViewUtils::kFmiEditingModeNormal) // jos ollaan ns. view-moodissa, disabloidaan koko nappula
		pCmdUI->Enable(FALSE);
}

void CSmartMetDoc::OnUpdateMenuitemGriddingOptions(CCmdUI *pCmdUI)
{
	if(GetData()->SmartMetEditingMode() != CtrlViewUtils::kFmiEditingModeNormal) // jos ollaan ns. view-moodissa, disabloidaan koko nappula
		pCmdUI->Enable(FALSE);
}

void CSmartMetDoc::OnUpdateDeselectAll(CCmdUI *pCmdUI)
{
	if(GetData()->SmartMetEditingMode() != CtrlViewUtils::kFmiEditingModeNormal) // jos ollaan ns. view-moodissa, disabloidaan koko nappula
		pCmdUI->Enable(FALSE);
}

void CSmartMetDoc::OnUpdateDataStoreToFile(CCmdUI *pCmdUI)
{
	if(GetData()->SmartMetEditingMode() != CtrlViewUtils::kFmiEditingModeNormal) // jos ollaan ns. view-moodissa, disabloidaan koko nappula
		pCmdUI->Enable(FALSE);
}

void CSmartMetDoc::OnUpdateButtonMakeEditedDataCopy(CCmdUI *pCmdUI)
{
	if(GetData()->SmartMetEditingMode() != CtrlViewUtils::kFmiEditingModeNormal) // jos ollaan ns. view-moodissa, disabloidaan koko nappula
		pCmdUI->Enable(FALSE);
}

void CSmartMetDoc::OnUpdateMakeGridFile(CCmdUI *pCmdUI)
{
	if(GetData()->SmartMetEditingMode() != CtrlViewUtils::kFmiEditingModeNormal) // jos ollaan ns. view-moodissa, disabloidaan koko nappula
		pCmdUI->Enable(FALSE);
}

void CSmartMetDoc::OnUpdateEditPaste(CCmdUI *pCmdUI)
{
	if(GetData()->SmartMetEditingMode() != CtrlViewUtils::kFmiEditingModeNormal) // jos ollaan ns. view-moodissa, disabloidaan koko nappula
		pCmdUI->Enable(FALSE);
}

void CSmartMetDoc::OnUpdateEditCopy(CCmdUI *pCmdUI)
{
	if(GetData()->SmartMetEditingMode() != CtrlViewUtils::kFmiEditingModeNormal) // jos ollaan ns. view-moodissa, disabloidaan koko nappula
		pCmdUI->Enable(FALSE);
}

void CSmartMetDoc::OnAcceleratorCrossSectionMode()
{
	GetData()->LogMessage("Set cross section mode on map view.", CatLog::Severity::Info, CatLog::Category::Operational);
	GetData()->CrossSectionSystem()->CrossSectionSystemActive(!GetData()->CrossSectionSystem()->CrossSectionSystemActive());
	GetData()->MapDirty(itsMapViewDescTopIndex, true, true);
	UpdateAllViewsAndDialogs("Set cross section mode on map view (F4)");
}

void CSmartMetDoc::OnAcceleratorMapRow1()
{
	GetData()->OnAcceleratorMapRow(itsMapViewDescTopIndex, 1);
}

void CSmartMetDoc::OnAcceleratorMapRow2()
{
	GetData()->OnAcceleratorMapRow(itsMapViewDescTopIndex, 2);
}

void CSmartMetDoc::OnAcceleratorMapRow3()
{
	GetData()->OnAcceleratorMapRow(itsMapViewDescTopIndex, 3);
}

void CSmartMetDoc::OnAcceleratorMapRow4()
{
	GetData()->OnAcceleratorMapRow(itsMapViewDescTopIndex, 4);
}

void CSmartMetDoc::OnAcceleratorMapRow5()
{
	GetData()->OnAcceleratorMapRow(itsMapViewDescTopIndex, 5);
}

void CSmartMetDoc::OnAcceleratorMapRow6()
{
    GetData()->OnAcceleratorMapRow(itsMapViewDescTopIndex, 6);
}

void CSmartMetDoc::OnAcceleratorMapRow7()
{
    GetData()->OnAcceleratorMapRow(itsMapViewDescTopIndex, 7);
}

void CSmartMetDoc::OnAcceleratorMapRow8()
{
    GetData()->OnAcceleratorMapRow(itsMapViewDescTopIndex, 8);
}

void CSmartMetDoc::OnAcceleratorMapRow9()
{
    GetData()->OnAcceleratorMapRow(itsMapViewDescTopIndex, 9);
}

void CSmartMetDoc::OnAcceleratorMapRow10()
{
    GetData()->OnAcceleratorMapRow(itsMapViewDescTopIndex, 10);
}

void CSmartMetDoc::OnMuokkaaMakropolkuasetukset()
{
	if(GetData()->MakeMacroPathConfigurations())
	{
		// en laita näitä muutamia dialogi päivityksiä UpdateAllViewsAndDialogs-metodiin, koska ne ovat muuten turhia kun tendään näytön päivityksiä
		if(itsViewMacroDlg)
			itsViewMacroDlg->InitMacroListFromDoc();
		if(itsSmartToolDlg)
			itsSmartToolDlg->UpdateMacroParamDisplayList(true);

		UpdateAllViewsAndDialogs("Updated macro path settings");
	}
}

template<class TView>
static void SaveViewPositionToRegistry(TView *theView, NFmiApplicationWinRegistry &theApplicationWinRegistry, unsigned int theMapViewDescTopIndex)
{
    if(theView)
        Persist2::WriteWindowRectToWinRegistry(theApplicationWinRegistry, theView->MakeUsedWinRegistryKeyStr(theMapViewDescTopIndex), theView);
}

void CSmartMetDoc::SaveViewPositionsToRegistry(void)
{
	// tässä pitää tallettaa 'permanent' dialogien sijainnit rekisteriin.
	// piti laittaa tänne, koska aikasarja-ikkuna ja jotkin dialogit toimivat
	// myös CDOCin destruktorista, mutta jotkin dialogit eivät, joten laitan
	// talletukset tähän metodiin.
    auto &applicationWinRegistry = itsData->ApplicationWinRegistry();
    // Only mapView 2 and 3 need really given index to work, all othe vies may use this dummy index
    unsigned long dummyMapDescTopIndex = 0;
    ::SaveViewPositionToRegistry((CMainFrame*)AfxGetMainWnd(), applicationWinRegistry, dummyMapDescTopIndex);
    ::SaveViewPositionToRegistry(itsZoomDlg, applicationWinRegistry, dummyMapDescTopIndex);
    ::SaveViewPositionToRegistry(itsTimeSerialDataEditorDlg, applicationWinRegistry, dummyMapDescTopIndex);
    ::SaveViewPositionToRegistry(itsTempDialog, applicationWinRegistry, dummyMapDescTopIndex);
    ::SaveViewPositionToRegistry(itsCrossSectionDlg, applicationWinRegistry, dummyMapDescTopIndex);
    ::SaveViewPositionToRegistry(itsSynopDataGridViewDlg, applicationWinRegistry, dummyMapDescTopIndex);
    ::SaveViewPositionToRegistry(itsTrajectoryDlg, applicationWinRegistry, dummyMapDescTopIndex);
    ::SaveViewPositionToRegistry(itsViewMacroDlg, applicationWinRegistry, dummyMapDescTopIndex);
    ::SaveViewPositionToRegistry(itsWarningCenterDlg, applicationWinRegistry, dummyMapDescTopIndex);
    ::SaveViewPositionToRegistry(itsSeaIcingWarningsDlg, applicationWinRegistry, dummyMapDescTopIndex);
    ::SaveViewPositionToRegistry(itsWindTableDlg, applicationWinRegistry, dummyMapDescTopIndex);
    ::SaveViewPositionToRegistry(itsExtraMapViewDlg1, applicationWinRegistry, 2);
    ::SaveViewPositionToRegistry(itsExtraMapViewDlg2, applicationWinRegistry, 3);
    ::SaveViewPositionToRegistry(itsDataQualityCheckerDialog, applicationWinRegistry, dummyMapDescTopIndex);
    ::SaveViewPositionToRegistry(itsCaseStudyDlg, applicationWinRegistry, dummyMapDescTopIndex);
    ::SaveViewPositionToRegistry(itsSmartToolDlg, applicationWinRegistry, dummyMapDescTopIndex);
    ::SaveViewPositionToRegistry(itsBetaProductDialog, applicationWinRegistry, dummyMapDescTopIndex);
    ::SaveViewPositionToRegistry(itsLogViewer, applicationWinRegistry, dummyMapDescTopIndex);
    ::SaveViewPositionToRegistry(itsParamAddingDlg, applicationWinRegistry, dummyMapDescTopIndex);

    // Talletetaan myös tiettyjä GeneralDocissa olevia juttuja aika-ajoin WinRekisteriin
    itsData->StoreSettingsToWinRegistry();
}

void CSmartMetDoc::OnCloseDocument()
{
	try
	{
        SaveViewPositionsToRegistry();
	}
	catch(std::exception &e)
	{
		std::string problemStr("Problems when trying to save the settings to the settings files:\n");
		problemStr += e.what();
		GetData()->LogAndWarnUser(problemStr, "Problems when saving settings.", CatLog::Severity::Error, CatLog::Category::Configuration, false);
	}
	catch(...)
	{
		std::string problemStr("Problems when trying to save the settings to the settings files:\n");
		GetData()->LogAndWarnUser(problemStr, "Problems when saving settings.", CatLog::Severity::Error, CatLog::Category::Configuration, false);
	}

	CDocument::OnCloseDocument();
}

// Tämä on pikkuinen pikaviritys.
// Tätä kutsutaan CMainFrame::OnClose -metodissa ennen kuin kutsutaan 
// NFmiEditMapGeneralDataDoc::StoreSupplementaryData -metodia.
// Ideaali paikka muuten tehdä tietyt sulkemis jutut olisi OnCloseDocument-metodissa, mutta
// se kutsutaan vasta NFmiEditMapGeneralDataDoc::StoreSupplementaryData -metodin jälkeen. 
// Enkä halua muuttaa enää ohjelman rakenteita kovinkaan radikaalisti.
void CSmartMetDoc::DoOnClose(void)
{
	if(itsCaseStudyDlg)
		itsCaseStudyDlg->GetBasicInfoFromDialog(); // jos caseStudy-dialogissa on tehty perus infoihin muutoksia, eikä dialogia ole suljettu, pitää arvot ottaa erikseen talteen dokumentin olioon ennen arvojen talletuksia tiedostoon.
}

static const int g_missingViewIndex = -9999;

template<typename View>
void MakeMakeWindowPosMapInsert(std::map<std::string, std::string> &windowPosMap, int possibleViewIndex = g_missingViewIndex)
{
    std::string winDefaultRectStr = Persist2::MakeWindowRectString(View::ViewPosRegistryInfo().DefaultWindowRect());
    std::string winRegistryKeyStr = View::ViewPosRegistryInfo().WinRegistryKeyStr();
    if(possibleViewIndex != g_missingViewIndex)
        winRegistryKeyStr += NFmiStringTools::Convert(possibleViewIndex);
    windowPosMap.insert(std::make_pair(winRegistryKeyStr, winDefaultRectStr));
}

// Tehdään map, jossa on kaikkien eri karttanäyttöjen Win-rekisteri avaimet ja default-ikkuna rect stringinä
std::map<std::string, std::string> CSmartMetDoc::MakeMapWindowPosMap(void)
{
    std::map<std::string, std::string> windowPosMap;

    // Pääkarttanäyttö
    MakeMakeWindowPosMapInsert<CMainFrame>(windowPosMap);

    // apukarttanäytöille pitää antaa avaimeen tuleva indeksi, koska sitä ei voi kysyä suoraan luokan staattiselta funktiolta
    MakeMakeWindowPosMapInsert<CFmiExtraMapViewDlg>(windowPosMap, 2);
    MakeMakeWindowPosMapInsert<CFmiExtraMapViewDlg>(windowPosMap, 3);

    return windowPosMap;
}

// Tehdään map, jossa on kaikkien eri ei-karttanäyttöjen Win-rekisteri avaimet ja default-ikkuna rect stringinä
std::map<std::string, std::string> CSmartMetDoc::MakeOtherWindowPosMap(void)
{
    std::map<std::string, std::string> windowPosMap;

    MakeMakeWindowPosMapInsert<CFmiCrossSectionDlg>(windowPosMap);
    MakeMakeWindowPosMapInsert<CTimeEditValuesDlg>(windowPosMap);
    MakeMakeWindowPosMapInsert<CFmiTrajectoryDlg>(windowPosMap);
    MakeMakeWindowPosMapInsert<CZoomDlg>(windowPosMap);
    MakeMakeWindowPosMapInsert<CFmiTempDlg>(windowPosMap);
    MakeMakeWindowPosMapInsert<CFmiSynopDataGridViewDlg>(windowPosMap);
    MakeMakeWindowPosMapInsert<CFmiViewMacroDlg>(windowPosMap);
    MakeMakeWindowPosMapInsert<CFmiWarningCenterDlg>(windowPosMap);
    MakeMakeWindowPosMapInsert<CFmiSeaIcingWarningsDlg>(windowPosMap);
    MakeMakeWindowPosMapInsert<CFmiWindTableDlg>(windowPosMap);
    MakeMakeWindowPosMapInsert<CFmiDataQualityCheckerDialog>(windowPosMap);
    MakeMakeWindowPosMapInsert<CFmiCaseStudyDlg>(windowPosMap);
    MakeMakeWindowPosMapInsert<CFmiSmartToolDlg>(windowPosMap);
    MakeMakeWindowPosMapInsert<CFmiBetaProductTabControlDialog>(windowPosMap);
    MakeMakeWindowPosMapInsert<CFmiLogViewer>(windowPosMap);
    MakeMakeWindowPosMapInsert<CFmiParamAddingDlg>(windowPosMap);
    MakeMakeWindowPosMapInsert<CFmiWarningMessageOptionsDlg>(windowPosMap);

    return windowPosMap;
}

void CSmartMetDoc::OnOhjeVirhetesti()
{
	DoCrashTest();
}

void CSmartMetDoc::OnMenuitemMuokkaaKieli()
{
	GetData()->SelectLanguage();
}

void CSmartMetDoc::OnButtonTrajectory()
{
	if(GetData())
	{
		if(!itsTrajectoryDlg)
			CreateTrajectoryDlg(GetData());
		if(GetData()->TrajectorySystem()->TrajectoryViewOn())
		{ // jos trajektori dialogi oli päällä, laitetaan se pois päältä
			itsTrajectoryDlg->ShowWindow(SW_HIDE);
			GetData()->LogMessage("Closing trajectory dialog.", CatLog::Severity::Info, CatLog::Category::Operational);
		}
		else
		{ // muuten laitetaan se päälle
			itsTrajectoryDlg->ShowWindow(SW_SHOW);	// Vaihdoin SW_RESTOREN, muistaa ikkunan muutetun koon.
			itsTrajectoryDlg->SetActiveWindow();
			GetData()->LogMessage("Opening trajectory dialog.", CatLog::Severity::Info, CatLog::Category::Operational);
		}
		GetData()->TrajectorySystem()->TrajectoryViewOn(!GetData()->TrajectorySystem()->TrajectoryViewOn());
		// päivitetään kartta ja muutkin näytöt, koska luotaus asemien kolmioiden kartta piirto riippuu tästä
		GetData()->AreaViewDirty(itsMapViewDescTopIndex, false, true); // laitetaan viela kaikki ajat likaisiksi cachesta
		UpdateAllViewsAndDialogs("Opening/closing trajectory view");
	}
}

void CSmartMetDoc::OnButtonDataQualityChecker()
{
	if(GetData())
	{
		if(!itsDataQualityCheckerDialog)
			CreateDataQualityCheckerDialog(GetData());
		if(GetData()->DataQualityChecker().ViewOn())
		{ // jos dialogi oli päällä, laitetaan se pois päältä
			itsDataQualityCheckerDialog->ShowWindow(SW_HIDE);
			GetData()->LogMessage("DataQualityChecker Dialog off.", CatLog::Severity::Info, CatLog::Category::Operational);
		}
		else
		{ // muuten laitetaan se päälle
			itsDataQualityCheckerDialog->ShowWindow(SW_SHOW);	// Vaihdoin SW_RESTOREN, muistaa ikkunan muutetun koon.
			itsDataQualityCheckerDialog->SetActiveWindow();
			GetData()->LogMessage("DataQualityChecker Dialog on.", CatLog::Severity::Info, CatLog::Category::Operational);
		}
		GetData()->DataQualityChecker().ViewOn(!GetData()->DataQualityChecker().ViewOn());
	}
}


void CSmartMetDoc::OnButtonSeaIcingWarningsDlg()
{
	if(GetData())
	{
		if(!itsSeaIcingWarningsDlg)
			CreateSeaIcingWarningsDlg(GetData());
		if(GetData()->SeaIcingWarningSystem().ViewVisible())
		{ // jos dialogi oli päällä, laitetaan se pois päältä
			itsSeaIcingWarningsDlg->ShowWindow(SW_HIDE);
			GetData()->LogMessage("Closing Sea Icing dialog.", CatLog::Severity::Info, CatLog::Category::Operational);
		}
		else
		{ // muuten laitetaan se päälle
			itsSeaIcingWarningsDlg->ShowWindow(SW_SHOW);	// Vaihdoin SW_RESTOREN, muistaa ikkunan muutetun koon.
			itsSeaIcingWarningsDlg->SetActiveWindow();
			GetData()->LogMessage("Opening Sea Icing dialog.", CatLog::Severity::Info, CatLog::Category::Operational);
		}
		GetData()->SeaIcingWarningSystem().ViewVisible(!GetData()->SeaIcingWarningSystem().ViewVisible());
		// päivitetään kartta ja muutkin näytöt, koska luotaus asemien kolmioiden kartta piirto riippuu tästä
		GetData()->AreaViewDirty(itsMapViewDescTopIndex, false, true); // laitetaan viela kaikki ajat likaisiksi cachesta
		UpdateAllViewsAndDialogs("Opening/closing sea-icing warnings dialog");
	}
}

void CSmartMetDoc::OnExtraMapView(unsigned int theMapViewDescTopIndex, CFmiExtraMapViewDlg **theExtraMapViewDlg)
{
	if(GetData())
	{
		if(*theExtraMapViewDlg == 0)
			*theExtraMapViewDlg = CreateExtraMapViewDlg(GetData(), theMapViewDescTopIndex);
		if(GetData()->MapViewDescTop(theMapViewDescTopIndex)->DescTopOn())
		{ // jos dialogi oli päällä, laitetaan se pois päältä
			(*theExtraMapViewDlg)->ShowWindow(SW_HIDE);
			GetData()->LogMessage(string("Put ExtraMapViewDlg-") + NFmiStringTools::Convert<unsigned int>(theMapViewDescTopIndex) + " off.", CatLog::Severity::Info, CatLog::Category::Operational);
		}
		else
		{ // muuten laitetaan se päälle
			(*theExtraMapViewDlg)->ShowWindow(SW_SHOW);	// Vaihdoin SW_RESTOREN, muistaa ikkunan muutetun koon.
			(*theExtraMapViewDlg)->SetActiveWindow();
			GetData()->LogMessage(string("Put ExtraMapViewDlg-") + NFmiStringTools::Convert<unsigned int>(theMapViewDescTopIndex) + " on.", CatLog::Severity::Info, CatLog::Category::Operational);
		}
		GetData()->MapViewDescTop(theMapViewDescTopIndex)->DescTopOn(!GetData()->MapViewDescTop(theMapViewDescTopIndex)->DescTopOn());
		// päivitetään kartta ja muutkin näytöt, koska luotaus asemien kolmioiden kartta piirto riippuu tästä
		GetData()->AreaViewDirty(theMapViewDescTopIndex, true, true); // laitetaan viela kaikki ajat likaisiksi cachesta
		UpdateAllViewsAndDialogs("Opening/closing map view " + std::to_string(theMapViewDescTopIndex + 1));
	}
}

void CSmartMetDoc::OnExtraMapView1(void)
{
	OnExtraMapView(1, &itsExtraMapViewDlg1);
}

void CSmartMetDoc::OnUpdateOnExtraMapView1(CCmdUI *pCmdUI)
{
	try
	{
	pCmdUI->SetCheck(GetData()->MapViewDescTop(1)->DescTopOn());
	}
	catch(...)
	{
	}
}

void CSmartMetDoc::OnExtraMapView2(void)
{
	OnExtraMapView(2, &itsExtraMapViewDlg2);
}

void CSmartMetDoc::OnUpdateOnExtraMapView2(CCmdUI *pCmdUI)
{
	try
	{
	pCmdUI->SetCheck(GetData()->MapViewDescTop(2)->DescTopOn());
	}
	catch(...)
	{
	}
}

void CSmartMetDoc::OnUpdateButtonTrajectory(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(GetData()->TrajectorySystem()->TrajectoryViewOn());
}

void CSmartMetDoc::OnViewEditorLog()
{
	std::string commandStr("notepad ");
	commandStr += CatLog::currentLogFilePath();
	CFmiProcessHelpers::ExecuteCommandInSeparateProcess(commandStr);
}

void CSmartMetDoc::OnToggleOverMapBackForeGround()
{
	GetData()->OnToggleOverMapBackForeGround(itsMapViewDescTopIndex);
}

void CSmartMetDoc::OnAcceleratorToggleTooltip()
{
	GetData()->MetEditorOptionsData().ShowToolTipsOnMapView(!GetData()->MetEditorOptionsData().ShowToolTipsOnMapView());
}

void CSmartMetDoc::OnAcceleratorToggleKeepMapRatio()
{
	GetData()->OnAcceleratorToggleKeepMapRatio();
}

void CSmartMetDoc::DoCrashTest(void)
{
    CatLog::logMessage(std::string(__FUNCTION__) + ": user starts crash test that will crash SmartMet", CatLog::Severity::Warning, CatLog::Category::Operational, true);
	// tämän on vain tarkoitus testata editorin virhe raportointia
//	throw std::runtime_error("Can you intersept this!!!"); // poikkeuksien käsittely näemmä tuottaa ongelmia crashrpt:lle
	NFmiFastQueryInfo *dummy = 0;
	dummy->Area();
	int x = 5;
	int y = 0;
	int voivoi = x%y;

	NFmiDataMatrix<float> bs;
	bs[10][20] = 4.6f;
	bs.At(10, 20) = 4.6f;
}

void CSmartMetDoc::OnMenuitemHelpEditorModeSettings()
{
	CFmiHelpEditorSettingsDlg dlg(GetData()->HelpEditorSystem());
	if(dlg.DoModal() == IDOK)
	{
		GetData()->HelpEditorSystem().StoreSettings(false);
		GetData()->MapDirty(itsMapViewDescTopIndex, true, true);// tämä 'aiheuttaa' datan harvennuksen
		UpdateAllViewsAndDialogs("Help edit mode settings changed");
	}
}

void CSmartMetDoc::OnButtonHelpEditorMode()
{
	GetData()->HelpEditorSystem().HelpEditor(!GetData()->HelpEditorSystem().HelpEditor());
	GetData()->MapDirty(itsMapViewDescTopIndex, true, true);// tämä 'aiheuttaa' datan harvennuksen
	UpdateAllViewsAndDialogs("Help edit mode changed");
}

void CSmartMetDoc::OnUpdateButtonHelpEditorMode(CCmdUI *pCmdUI)
{
	if(GetData()->HelpEditorSystem().Use())
	{
		pCmdUI->Enable(TRUE);
		pCmdUI->SetCheck(GetData()->HelpEditorSystem().HelpEditor());
	}
	else
	{
		pCmdUI->Enable(FALSE);
		pCmdUI->SetCheck(FALSE);
	}
}

void CSmartMetDoc::OnViewSetWarningCenterDlgPlaceToDefault()
{
	if(itsWarningCenterDlg)
	{
		itsWarningCenterDlg->SetDefaultValues();
		itsWarningCenterDlg->Update();
		itsWarningCenterDlg->SetActiveWindow();
	}
}

void CSmartMetDoc::OnViewSetSeaIcingWarningsDlgPlaceToDefault()
{
	if(itsSeaIcingWarningsDlg)
	{
		itsSeaIcingWarningsDlg->SetDefaultValues();
		itsSeaIcingWarningsDlg->Update();
		itsSeaIcingWarningsDlg->SetActiveWindow();
	}
}


void CSmartMetDoc::OnButtonWindTableDlg()
{
	if(GetData())
	{
		if(!itsWindTableDlg)
			CreateWindTableDlg(GetData());
		if(GetData()->WindTableSystem().ViewVisible())
		{ // jos dialogi oli päällä, laitetaan se pois päältä
			itsWindTableDlg->ShowWindow(SW_HIDE);
			GetData()->LogMessage("Hide WindTable view.", CatLog::Severity::Info, CatLog::Category::Operational);
		}
		else
		{ // muuten laitetaan se päälle
			itsWindTableDlg->ShowWindow(SW_SHOW);	// Vaihdoin SW_RESTOREN, muistaa ikkunan muutetun koon.
			itsWindTableDlg->SetActiveWindow();
			GetData()->LogMessage("Show Wind table view.", CatLog::Severity::Info, CatLog::Category::Operational);
		}
		GetData()->WindTableSystem().ViewVisible(!GetData()->WindTableSystem().ViewVisible());
		// päivitetään kartta ja muutkin näytöt, koska luotaus asemien kolmioiden kartta piirto riippuu tästä
		GetData()->AreaViewDirty(itsMapViewDescTopIndex, false, true); // laitetaan viela kaikki ajat likaisiksi cachesta
		UpdateAllViewsAndDialogs("Opening/closing Wind table view");
	}
}

void CSmartMetDoc::OnViewSetWindTableDlgPlaceToDefault()
{
	if(itsWindTableDlg)
	{
		itsWindTableDlg->SetDefaultValues();
		itsWindTableDlg->Update();
		itsWindTableDlg->SetActiveWindow();
	}
}

void CSmartMetDoc::OnChangePreviousMapType()
{
	GetData()->OnChangeMapType(itsMapViewDescTopIndex, false);
}

void CSmartMetDoc::OnToggleShowPreviousNamesOnMap()
{
	GetData()->OnToggleShowNamesOnMap(itsMapViewDescTopIndex, false);
}

void CSmartMetDoc::OnAcceleratorLocationFinderTool()
{
	NFmiEditMapGeneralDataDoc * doc = GetData();
	if(doc && doc->AutoComplete().Use())
	{
		if(itsLocationFinderDlg == 0)
			CreateLocationFinderDlg(itsData);
		doc->AutoComplete().AutoCompleteDialogOn(true);
		itsLocationFinderDlg->ShowWindow(SW_RESTORE);
		itsLocationFinderDlg->SetActiveWindow();
		itsLocationFinderDlg->SetFocus();
	}
}

void CSmartMetDoc::SetMacroErrorText(const std::string &theErrorStr)
{
	if(itsSmartToolDlg)
		itsSmartToolDlg->SetMacroErrorText(theErrorStr);
}


void CSmartMetDoc::OnUpdateButtonDataQualityChecker(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(GetData()->DataQualityChecker().ViewOn());
}

// F7 -pressed
void CSmartMetDoc::OnAcceleratorToggleHelpCursorOnMap()
{
	GetData()->ShowMouseHelpCursorsOnMap(!GetData()->ShowMouseHelpCursorsOnMap());
    ApplicationInterface::GetApplicationInterfaceImplementation()->ForceOtherMapViewsDrawOverBitmapThings(999);
}

// F9 -pressed
void CSmartMetDoc::OnButtonWarningCenterDlg()
{
#ifndef DISABLE_CPPRESTSDK
    if(GetData())
	{
		if(!itsWarningCenterDlg)
			CreateWarningCenterDlg(GetData());
		if(GetData()->WarningCenterSystem().getLegacyData().WarningCenterViewOn())
		{ // jos dialogi oli päällä, laitetaan se pois päältä
			itsWarningCenterDlg->ShowWindow(SW_HIDE);
			GetData()->LogMessage("Closing Warning Center dialog.", CatLog::Severity::Info, CatLog::Category::Operational);
		}
		else
		{ // muuten laitetaan se päälle
			itsWarningCenterDlg->ShowWindow(SW_SHOW);	// Vaihdoin SW_RESTOREN, muistaa ikkunan muutetun koon.
			itsWarningCenterDlg->SetActiveWindow();
			GetData()->LogMessage("Opening Warning Center dialog.", CatLog::Severity::Info, CatLog::Category::Operational);
		}
		GetData()->WarningCenterSystem().getLegacyData().WarningCenterViewOn(!GetData()->WarningCenterSystem().getLegacyData().WarningCenterViewOn());
		// päivitetään kartta ja muutkin näytöt, koska luotaus asemien kolmioiden kartta piirto riippuu tästä
		GetData()->AreaViewDirty(itsMapViewDescTopIndex, false, true); // laitetaan viela kaikki ajat likaisiksi cachesta
		UpdateAllViewsAndDialogs("Opening/closing Warning center dialog");
	}
#endif // DISABLE_CPPRESTSDK
}

void CSmartMetDoc::OnAcceleratorDoMacroDirectoriesSyncronization()
{
	CWaitCursor cursor;
	GetData()->DoMacroDirectoriesSyncronization();
}

void CSmartMetDoc::OnViewCaseStudyDialog()
{
    if(GetData())
    {
        if(itsCaseStudyDlg == 0)
        {
            CreateCaseStudyDlg(GetData());
        }
        itsCaseStudyDlg->ShowWindow(SW_SHOW);	// Vaihdoin SW_RESTOREN, muistaa ikkunan muutetun koon.
        itsCaseStudyDlg->SetActiveWindow();
        itsCaseStudyDlg->AdjustDialogControls();

        GetData()->LogMessage("Case Study Dialog on.", CatLog::Severity::Info, CatLog::Category::Operational);
    }
}

static void PutTextToClipboard(const std::string &text)
{
    CString csClipDataU_ = CA2T(text.c_str());
    if ( !OpenClipboard(NULL) )
        return;
    // Remove the current Clipboard contents
    if( !EmptyClipboard() )
        return;
    size_t size = sizeof(TCHAR)*(1 + csClipDataU_.GetLength());
    HGLOBAL hResult = GlobalAlloc(GMEM_MOVEABLE, size); 
    LPTSTR lptstrCopy = (LPTSTR)GlobalLock(hResult); 
    memcpy(lptstrCopy, csClipDataU_.GetBuffer(), size);
    GlobalUnlock(hResult); 
#ifndef _UNICODE
    if ( ::SetClipboardData( CF_TEXT, hResult ) == NULL )
#else
    if ( ::SetClipboardData( CF_UNICODETEXT, hResult ) == NULL )
#endif
    {
        GlobalFree(hResult); 
        CloseClipboard();
        return;
    }
    CloseClipboard();
}

std::string MakeSizeString(double x, double y)
{
    std::string str;
    str += NFmiStringTools::Convert(FmiRound(x));
    str += "x";
    str += NFmiStringTools::Convert(FmiRound(y));
    return str;
}

double MetersToDegrees(double latitude, double meters)
{
    return meters / (111.32 * 1000 * cos(latitude * (3.14159 / 180)));
}

// Tekee mapserver url:in halutulla resoluutiolla, esim:
// 60px/deg (2 km) resolution:   	&SRS=EPSG:4326&BBOX=-77.9276,12.7648,-58.0438,21.538&WIDTH=1193&HEIGHT=526
static std::string MakeUrlStr(boost::shared_ptr<NFmiArea> &area, int pixelPerDegree, double pixelResolutionInKMAbout, unsigned long areaId)
{
    const int desiredBaseStringLength = 30;
    std::string baseUrlStr;
    std::stringstream ss;
    ss << pixelPerDegree << "px/deg (" << pixelResolutionInKMAbout << " km) resolution:";
    baseUrlStr += ss.str();
    int fillSize = desiredBaseStringLength - static_cast<int>(baseUrlStr.size());
    for(int i = 0; i < fillSize; i++)
        baseUrlStr += ' ';
    baseUrlStr += '\t';

    if(areaId == kNFmiLatLonArea)
    {
        baseUrlStr += "&SRS=EPSG:4326&BBOX=";
	    baseUrlStr += NFmiStringTools::Convert(area->BottomLeftLatLon().X()) + ",";
	    baseUrlStr += NFmiStringTools::Convert(area->BottomLeftLatLon().Y()) + ",";
	    baseUrlStr += NFmiStringTools::Convert(area->TopRightLatLon().X()) + ",";
	    baseUrlStr += NFmiStringTools::Convert(area->TopRightLatLon().Y());

        baseUrlStr += "&WIDTH=";
        long width = FmiRound((area->TopRightLatLon().X() - area->BottomLeftLatLon().X()) * pixelPerDegree);
        baseUrlStr += NFmiStringTools::Convert(width);
        baseUrlStr += "&HEIGHT=";
        long height = FmiRound((area->TopRightLatLon().Y() - area->BottomLeftLatLon().Y()) * pixelPerDegree);
        baseUrlStr += NFmiStringTools::Convert(height);

    }
    else if (areaId == kNFmiStereographicArea)
    {
        auto bottomLeft = area->WorldRect().TopLeft();
        auto topRight = area->WorldRect().BottomRight();

        baseUrlStr += "&SRS=EPSG:999920&BBOX=";
        baseUrlStr += to_string(bottomLeft.X()) + ",";
        baseUrlStr += to_string(bottomLeft.Y()) + ",";
        baseUrlStr += to_string(topRight.X()) + ",";
        baseUrlStr += to_string(topRight.Y());

        baseUrlStr += "&WIDTH=";
        long width = FmiRound(MetersToDegrees(60,topRight.X() - bottomLeft.X()) * pixelPerDegree);
        baseUrlStr += NFmiStringTools::Convert(width);
        baseUrlStr += "&HEIGHT=";
        long height = FmiRound(MetersToDegrees(60, topRight.Y() - bottomLeft.Y()) * pixelPerDegree);
        baseUrlStr += NFmiStringTools::Convert(height);
    }

    return baseUrlStr;
}

// Creates map string for configurations and some example mapserver urls.
// latlon:-77.9276,12.7648,-58.0438,21.538
// Original map's total area size in pixels:3000x1425
// Original map's visible area size in pixels:1491x658
// Map's size in pixels on screen:1893x833
// Mapserver urls:
// 60px/deg (2 km) resolution:   	&SRS=EPSG:4326&BBOX=-77.9276,12.7648,-58.0438,21.538&WIDTH=1193&HEIGHT=526
// 120px/deg (1 km) resolution:  	&SRS=EPSG:4326&BBOX=-77.9276,12.7648,-58.0438,21.538&WIDTH=2386&HEIGHT=1053
// 240px/deg (0.5 km) resolution:	&SRS=EPSG:4326&BBOX=-77.9276,12.7648,-58.0438,21.538&WIDTH=4772&HEIGHT=2106
std::string MapAreaInfo(NFmiMapViewDescTop *mapViewDecsTop)
{
    if(mapViewDecsTop)
    {
        boost::shared_ptr<NFmiArea> zoomedArea = mapViewDecsTop->MapHandler()->Area();
        if(zoomedArea)
        {
            const std::string usedEndLineStr = "\r\n";
            std::string areaInfoStr = zoomedArea->AreaStr();

            areaInfoStr += usedEndLineStr + "Original map's total area size in pixels:";
            NFmiRect totalAreaInPixels = mapViewDecsTop->MapHandler()->TotalAbsolutRect();
            areaInfoStr += ::MakeSizeString(totalAreaInPixels.Width(), totalAreaInPixels.Height());

            areaInfoStr += usedEndLineStr + "Original map's visible area size in pixels:";
            NFmiRect zoomedAreaInPixels = mapViewDecsTop->MapHandler()->ZoomedAbsolutRect();
            areaInfoStr += ::MakeSizeString(zoomedAreaInPixels.Width(), zoomedAreaInPixels.Height());

            areaInfoStr += usedEndLineStr + "Map's size in pixels on screen:";
            NFmiPoint viewSizeInPixels = mapViewDecsTop->ActualMapBitmapSizeInPixels();
            areaInfoStr += ::MakeSizeString(viewSizeInPixels.X(), viewSizeInPixels.Y());

            areaInfoStr += usedEndLineStr + "Mapserver urls:";
            areaInfoStr += usedEndLineStr;
            areaInfoStr += MakeUrlStr(zoomedArea, 60, 2, zoomedArea->ClassId());
            areaInfoStr += usedEndLineStr;
            areaInfoStr += MakeUrlStr(zoomedArea, 120, 1, zoomedArea->ClassId());
            areaInfoStr += usedEndLineStr;
            areaInfoStr += MakeUrlStr(zoomedArea, 240, 0.5, zoomedArea->ClassId());
            areaInfoStr += usedEndLineStr + "Change EPSG if necesssary!";
            areaInfoStr += usedEndLineStr;
            return areaInfoStr;
        }
    }

    return "Unable to retrieve map area info for main map view.";
}

void CSmartMetDoc::OnAcceleratorAreaInfoToClipboard()
{
    ::PutTextToClipboard(::MapAreaInfo(itsData->MapViewDescTop(itsMapViewDescTopIndex)));
}


void CSmartMetDoc::OnTooltipstatusCrosssectionview()
{
    GetData()->CrossSectionSystem()->ShowTooltipOnCrossSectionView(!GetData()->CrossSectionSystem()->ShowTooltipOnCrossSectionView());
}


void CSmartMetDoc::OnUpdateTooltipstatusCrosssectionview(CCmdUI *pCmdUI)
{
    pCmdUI->SetCheck(GetData()->CrossSectionSystem()->ShowTooltipOnCrossSectionView());
}


void CSmartMetDoc::OnTooltipstatusMapviews()
{
	GetData()->MetEditorOptionsData().ShowToolTipsOnMapView(!GetData()->MetEditorOptionsData().ShowToolTipsOnMapView());
}


void CSmartMetDoc::OnUpdateTooltipstatusMapviews(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(GetData()->MetEditorOptionsData().ShowToolTipsOnMapView());
}


void CSmartMetDoc::OnTooltipstatusSoundingview()
{
    GetData()->ShowToolTipTempView(!GetData()->ShowToolTipTempView());
}


void CSmartMetDoc::OnUpdateTooltipstatusSoundingview(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(GetData()->ShowToolTipTempView());
}


void CSmartMetDoc::OnTooltipstatusTimeserialview()
{
    GetData()->ShowToolTipTimeView(!GetData()->ShowToolTipTimeView());
}


void CSmartMetDoc::OnUpdateTooltipstatusTimeserialview(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(GetData()->ShowToolTipTimeView());
}

// HUOM! Tässä käytetään F12 pikanäppäintä laukaisemaan tämä toiminto.
// Kun Visual C++:ssa ajaa debug versiota ohjelmasta ja painaa F12:sta, tällöin
// ohjelman suoritus keskeytetään ja tulee näkyviin dialogi, joka ilmoittaa että on tehty 
// pakotettu breakpoint. Jos tällöin painaa continue -nappulaa, ohjelma jatkaa normaalia
// toimintaansa. Release versio ei ole moksiskaan kun F12 painaa.
// Tätä breakpoint toimintoa ei voi mitenkään estää, se kuuluu VC++:n toimintaan.
void CSmartMetDoc::OnAcceleratorApplyBackupViewMacro()
{
    GetData()->ApplyBackupViewMacro(true);
}


void CSmartMetDoc::OnAcceleratorApplyCrashBackupViewMacro()
{
    GetData()->ApplyBackupViewMacro(false);
}

void CSmartMetDoc::OnReloaddataReloadfailedsatelliteimages()
{
    GetData()->ReloadFailedSatelliteImages();
}


void CSmartMetDoc::OnAcceleratorDevelopedFunctionTesting()
{
    // Täällä voi testata mitä tahansa kehitettävää funktionaalisuutta, mikä laukaistaan
    // testimielessä painamalla näppäimistön Insert -nappulaa.

    MakeVisualizationImages();
}

void CSmartMetDoc::MakeVisualizationImages()
{
/*
    NFmiMetTime startingTime(2015, 8, 24, 12, 0);
    long timeLenghtInMinutes = 5 * 60;
    long timeStepInMinutes = 60;
    std::string imageFileNameBase = "D:\\data\\images\\map_image_";
    std::string imageFileExtension = ".png";
    NFmiMetTime currentTime(startingTime);
    CBitmap mapScreenBitmap;
    for(long advancedMinutes = 0; advancedMinutes <= timeLenghtInMinutes; advancedMinutes += timeStepInMinutes)
    {
        std::string imageFileName = imageFileNameBase;
        imageFileName += currentTime.ToStr(kYYYYMMDDHHMM);
        imageFileName += imageFileExtension;
        GetData()->CurrentTime(itsMapViewDescTopIndex, currentTime);
        GetData()->MapView()->DoOffScreenDraw(mapScreenBitmap);
        
        CFmiGdiPlusHelpers::SaveMfcBitmapToFile(&mapScreenBitmap, imageFileName, &(GetData()->MapViewDescTop(itsMapViewDescTopIndex)->RelativeMapRect()));
        currentTime.ChangeByMinutes(timeStepInMinutes);
    }

    mapScreenBitmap.DeleteObject();
    GetData()->ChangeTime(0, kBackward, 1, itsMapViewDescTopIndex, timeLenghtInMinutes); // asetus takaisin alkuaikaan
    */
}


void CSmartMetDoc::OnViewBetaproduction()
{
    if(GetData())
    {
        if(itsBetaProductDialog == 0)
            CreateBetaProductDialog(SmartMetDocumentInterface::GetSmartMetDocumentInterfaceImplementation());
        itsBetaProductDialog->ShowWindow(SW_SHOW);	// Vaihdoin SW_RESTOREN, muistaa ikkunan muutetun koon.
        itsBetaProductDialog->SetActiveWindow();
        itsBetaProductDialog->Update();

        GetData()->LogMessage("Beta Production Dialog on.", CatLog::Severity::Info, CatLog::Category::Operational);
    }
}

void CSmartMetDoc::UpdateBetaProductDialog()
{
    if(itsBetaProductDialog == 0)
        CreateBetaProductDialog(SmartMetDocumentInterface::GetSmartMetDocumentInterfaceImplementation());
    itsBetaProductDialog->Update();
}

CWnd* CSmartMetDoc::GetView(int theWantedDescTopIndex)
{
    switch(theWantedDescTopIndex)
    {
    case 0:
        return ApplicationInterface::GetSmartMetView();
    case 1:
        return itsExtraMapViewDlg1;
    case 2:
        return itsExtraMapViewDlg2;
    case CtrlViewUtils::kFmiCrossSectionView:
        return itsCrossSectionDlg;
    case CtrlViewUtils::kFmiTimeSerialView:
        return itsTimeSerialDataEditorDlg;
    default:
        return nullptr;
    }
}

void CSmartMetDoc::DoOffScreenDraw(unsigned int theMapViewDescTopIndex, CBitmap &theDrawedScreenBitmapOut)
{
    if(theMapViewDescTopIndex == 0)
        CFmiWin32TemplateHelpers::DoOffScreenDraw(ApplicationInterface::GetSmartMetView(), SmartMetDocumentInterface::GetSmartMetDocumentInterfaceImplementation(), theDrawedScreenBitmapOut);
    else if(theMapViewDescTopIndex == 1)
        CFmiWin32TemplateHelpers::DoOffScreenDraw(itsExtraMapViewDlg1->MapView(), SmartMetDocumentInterface::GetSmartMetDocumentInterfaceImplementation(), theDrawedScreenBitmapOut);
    else if(theMapViewDescTopIndex == 2)
        CFmiWin32TemplateHelpers::DoOffScreenDraw(itsExtraMapViewDlg2->MapView(), SmartMetDocumentInterface::GetSmartMetDocumentInterfaceImplementation(), theDrawedScreenBitmapOut);
    else if(theMapViewDescTopIndex == 3)
        CFmiWin32TemplateHelpers::DoOffScreenDrawForNonMapView(itsTimeSerialDataEditorDlg->GetTimeEditValuesView(), SmartMetDocumentInterface::GetSmartMetDocumentInterfaceImplementation(), theDrawedScreenBitmapOut);
    else if(theMapViewDescTopIndex == 4)
    {
        // Lets set all possible temp info times to the main map's time
        SmartMetDocumentInterface::GetSmartMetDocumentInterfaceImplementation()->GetMTATempSystem().SetAllTempTimes(SmartMetDocumentInterface::GetSmartMetDocumentInterfaceImplementation()->CurrentTime(0));
        CFmiWin32TemplateHelpers::DoOffScreenDrawForNonMapView(itsTempDialog->GetTempView(), SmartMetDocumentInterface::GetSmartMetDocumentInterfaceImplementation(), theDrawedScreenBitmapOut);
    }
    else if(theMapViewDescTopIndex == 5)
        CFmiWin32TemplateHelpers::DoOffScreenDrawForNonMapView(itsCrossSectionDlg->GetCrossSectionView(), SmartMetDocumentInterface::GetSmartMetDocumentInterfaceImplementation(), theDrawedScreenBitmapOut);
}

void CSmartMetDoc::UpdateViewForOffScreenDraw(unsigned int theMapViewDescTopIndex)
{
    if(theMapViewDescTopIndex == 0)
        ApplicationInterface::GetSmartMetView()->Update();
    else if(theMapViewDescTopIndex == 1)
        itsExtraMapViewDlg1->Update(true);
    else if(theMapViewDescTopIndex == 2)
        itsExtraMapViewDlg2->Update(true);
    else if(theMapViewDescTopIndex == 3)
        CreateTimeEditor(true); // This creates dialog, if not created, and then updates it. 
    else if(theMapViewDescTopIndex == 4)
    {
        CreateTempDlg(); // This creates dialog, if not created
        itsTempDialog->Update();
    }
    else if(theMapViewDescTopIndex == 5)
        CreateCrossSectionDlg(true); // This creates dialog, if not created, and then updates it. 
}

void CSmartMetDoc::OnAcceleratorResetTimeFilterTimes()
{
    GetData()->ResetTimeFilterTimes();
    UpdateAllViewsAndDialogs("Reset time filter times (data editing related)", true);
}


void CSmartMetDoc::OnAcceleratorLogViewer()
{
    if(!itsLogViewer)
        CreateLogViewer(GetData());

    itsLogViewer->ShowWindow(SW_SHOW);
    itsLogViewer->SetActiveWindow();
    itsLogViewer->Update();

    GetData()->LogMessage("Log Viewer on.", CatLog::Severity::Info, CatLog::Category::Operational);
}

static const std::string gCalculationPointStartStr = "calculationpoint = ";
static const std::string gMaxLatlonLengthStr = "-123.123,-12.123";

static std::string MakeLatlonStr(const NFmiLocation &theLocation)
{
    std::string str = NFmiValueString::GetStringWithMaxDecimalsSmartWay(theLocation.GetLatitude(), 3);
    str += ",";
    str += NFmiValueString::GetStringWithMaxDecimalsSmartWay(theLocation.GetLongitude(), 3);
    // Tehdään kaikista stringeistä samanpituisia, ennen kuin lisätään aseman nimi lainausmerkeissä
    // perään, näin listasta tulee siistimmän näköistä katsella.
    if(str.size() < gMaxLatlonLengthStr.size())
        str += std::string(gMaxLatlonLengthStr.size() - str.size(), ' ');
    str += "\t// ";
    str += theLocation.GetName().CharPtr();
    return str;
}

static std::string CalculationPointInfo(NFmiMapViewDescTop *mapViewDecsTop, NFmiEditMapGeneralDataDoc *generalDataDoc)
{
    if(mapViewDecsTop)
    {
        auto iter = mapViewDecsTop->DrawParamListVector()->Index(mapViewDecsTop->ActiveViewRow() + mapViewDecsTop->MapRowStartingIndex() - 1);
        NFmiDrawParamList *aList = iter.CurrentPtr();
        if(aList)
        {
            if(aList->FindActive())
            {
                auto activeDrawParam = aList->Current();
                if(activeDrawParam)
                {
                    auto zoomedArea = mapViewDecsTop->MapHandler()->Area();
                    if(zoomedArea)
                    {
                        std::string str;
                        checkedVector<boost::shared_ptr<NFmiFastQueryInfo> > infoVector;
                        generalDataDoc->MakeDrawedInfoVectorForMapView(infoVector, activeDrawParam, zoomedArea);
                        for(auto &info : infoVector)
                        {
                            if(info && !info->Grid() && !info->HasLatlonInfoInData())
                            {
                                for(info->ResetLocation(); info->NextLocation(); )
                                {
                                    auto location = info->Location();
                                    if(zoomedArea->IsInside(location->GetLocation()))
                                    {
                                        str += gCalculationPointStartStr + ::MakeLatlonStr(*location) + "\n";
                                    }
                                }
                            }
                        }
                        if(!str.empty())
                            return str;
                    }
                }
            }
        }
    }
    return "Unable to retrieve calculationpoint info for active observation data.";
}

void CSmartMetDoc::OnAcceleratorCalculationpointInfoToClipboard()
{
    ::PutTextToClipboard(::CalculationPointInfo(itsData->MapViewDescTop(itsMapViewDescTopIndex), itsData));
}

static std::string ActiveRowDataFilePaths(NFmiMapViewDescTop *mapViewDecsTop, NFmiEditMapGeneralDataDoc *generalDataDoc)
{
    if(mapViewDecsTop)
    {
        auto iter = mapViewDecsTop->DrawParamListVector()->Index(mapViewDecsTop->ActiveViewRow() + mapViewDecsTop->MapRowStartingIndex() - 1);
        NFmiDrawParamList *aList = iter.CurrentPtr();
        if(aList)
        {
            std::string str;
            for(aList->Reset(); aList->Next(); )
            {
                auto drawParam = aList->Current();
                if(drawParam)
                {
                    auto zoomedArea = mapViewDecsTop->MapHandler()->Area();
                    if(zoomedArea)
                    {
                        checkedVector<boost::shared_ptr<NFmiFastQueryInfo> > infoVector;
                        generalDataDoc->MakeDrawedInfoVectorForMapView(infoVector, drawParam, zoomedArea);
                        for(auto &info : infoVector)
                        {
                            if(info)
                            {
                                str += NFmiFastInfoUtils::GetTotalDataFilePath(info) + "\n";
                            }
                        }
                    }
                }
            }
            if(!str.empty())
                return str;
        }
    }
    return "Unable to retrieve active row's data file paths.";
}

void CSmartMetDoc::OnAcceleratorDataFilepathsToClipboard()
{
    ::PutTextToClipboard(::ActiveRowDataFilePaths(itsData->MapViewDescTop(itsMapViewDescTopIndex), itsData));
}

const int gManyRowsCount = 10;

void CSmartMetDoc::OnAcceleratorMoveManyMapRowsUp()
{
    if(itsData->ScrollViewRow(itsMapViewDescTopIndex, -gManyRowsCount))
        UpdateAllViewsAndDialogs("Jumping through main map view rows");
}

void CSmartMetDoc::OnAcceleratorMoveManyMapRowsDown()
{
    if(itsData->ScrollViewRow(itsMapViewDescTopIndex, gManyRowsCount))
        UpdateAllViewsAndDialogs("Jumping through main map view rows");
}

void CSmartMetDoc::OnAcceleratorApplyStartupViewMacro()
{
    GetData()->ApplyStartupViewMacro();
}


void CSmartMetDoc::OnAcceleratorToggleWmsMapMode()
{
    itsData->UseWmsMaps(!itsData->UseWmsMaps());
    UpdateAllViewsAndDialogs("Toggle WMS map mode");
}
