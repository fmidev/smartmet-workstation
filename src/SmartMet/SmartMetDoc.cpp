
// SmartMetDoc.cpp : implementation of the CSmartMetDoc class
//
#ifdef _MSC_VER
#pragma warning(disable : 4786) // poistaa n kpl VC++ k��nt�j�n varoitusta
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
#include "FmiWindTableDlg.h"
#include "NFmiWindTableSystem.h"
#include "FmiLocationFinderDlg.h"
#include "NFmiAutoComplete.h"
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
#include "QueryDataToLocalCacheLoaderThread.h"
#include "FmiBetaProductTabControlDialog.h"
#include "FmiWin32TemplateHelpers.h"
#include "FmiLogViewer.h"
#include "CtrlViewFunctions.h"
#include "NFmiFastInfoUtils.h"
#include "HakeMessage/Main.h"
#include "HakeMessage/HakeSystemConfigurations.h"
#include "FmiParameterSelectionDlg.h"
#include "SpecialDesctopIndex.h"
#include "CtrlViewTimeConsumptionReporter.h"
#include "QueryDataReading.h"
#include "SmartMetDocumentInterface.h"
#include "ApplicationInterface.h"
#include "FmiWarningMessageOptionsDlg.h"
#include "NFmiMacroParamDataCache.h"
#include "FmiSoundingDataServerConfigurationsDlg.h"
#include "NFmiApplicationDataBase.h"
#include "CFmiVisualizationSettings.h"
#include "FmiMacroParamDataGeneratorDlg.h"

#ifndef DISABLE_CPPRESTSDK
#include "WmsSupport.h"
#endif // DISABLE_CPPRESTSDK

#include "AnimationProfiler.h"

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
	ON_UPDATE_COMMAND_UI(ID_EDIT_UNDO, OnUpdateEditUndo)
	ON_UPDATE_COMMAND_UI(ID_EDIT_REDO, OnUpdateEditRedo)
	ON_COMMAND(ID_CHANGE_PARAM_WINDOW_POSITION_FORWARD, OnChangeParamWindowPositionForward)
	ON_COMMAND(ID_CHANGE_PARAM_WINDOW_POSITION_BACKWARD, OnChangeParamWindowPositionBackward)
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
	ON_COMMAND(ID_ACCELERATOR_BORROW_PARAMS_6, OnAcceleratorBorrowParams6)
	ON_COMMAND(ID_ACCELERATOR_BORROW_PARAMS_7, OnAcceleratorBorrowParams7)
	ON_COMMAND(ID_ACCELERATOR_BORROW_PARAMS_8, OnAcceleratorBorrowParams8)
	ON_COMMAND(ID_ACCELERATOR_BORROW_PARAMS_9, OnAcceleratorBorrowParams9)
	ON_COMMAND(ID_ACCELERATOR_BORROW_PARAMS_10, OnAcceleratorBorrowParams10)

	ON_UPDATE_COMMAND_UI(ID_BUTTON_DATA_TO_DATABASE, OnUpdateButtonDataToDatabase)
	ON_UPDATE_COMMAND_UI(ID_BUTTON_STORE_DATA, OnUpdateButtonStoreData)
	ON_UPDATE_COMMAND_UI(ID_BUTTON_SELECTION_TOOL_DLG, OnUpdateButtonSelectionToolDlg)
	ON_UPDATE_COMMAND_UI(ID_SELECT_ALL, OnUpdateSelectAll)
	ON_UPDATE_COMMAND_UI(ID_MENUITEM_SAVE_AS, OnUpdateMenuitemSaveAs)
	ON_UPDATE_COMMAND_UI(ID_MENUITEM_GRIDDING_OPTIONS, OnUpdateMenuitemGriddingOptions)
	ON_UPDATE_COMMAND_UI(ID_DESELECT_ALL, OnUpdateDeselectAll)
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
	ON_COMMAND(ID_ACCELERATOR_TOGGLE_OVERMAP_FORE_BACK_GROUND, OnToggleOverMapBackForeGround)
	ON_COMMAND(ID_ACCELERATOR_TOGGLE_TOOLTIP, OnAcceleratorToggleTooltip)
	ON_COMMAND(ID_ACCELERATOR_TOGGLE_KEEP_MAP_RATIO, OnAcceleratorToggleKeepMapRatio)
	ON_COMMAND(ID_MENUITEM_HELP_EDITOR_SETTINGS, OnMenuitemHelpEditorModeSettings)
	ON_COMMAND(ID_BUTTON_HELP_EDITOR_MODE, OnButtonHelpEditorMode)
	ON_UPDATE_COMMAND_UI(ID_BUTTON_HELP_EDITOR_MODE, OnUpdateButtonHelpEditorMode)
	ON_COMMAND(ID_VIEW_SET_WARNING_CENTER_DLG_PLACE_TO_DEFAULT, OnViewSetWarningCenterDlgPlaceToDefault)
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
    ON_COMMAND(ID_VIEW_SET_PARAM_SELECTION_VIEW_PLACE_TO_DEFAULT, OnSetParameterSelectionDlgPlaceToDefault)
    ON_COMMAND(ID_HELP_EXCEPTIONTEST, &CSmartMetDoc::OnHelpExceptiontest)
    ON_COMMAND(ID_ACCELERATOR_CP_SELECT_NEXT, &CSmartMetDoc::OnAcceleratorCpSelectNext)
    ON_COMMAND(ID_ACCELERATOR_CP_SELECT_PREVIOUS, &CSmartMetDoc::OnAcceleratorCpSelectPrevious)
    ON_COMMAND(ID_ACCELERATOR_CP_SELECT_LEFT, &CSmartMetDoc::OnAcceleratorCpSelectLeft)
    ON_COMMAND(ID_ACCELERATOR_CP_SELECT_RIGHT, &CSmartMetDoc::OnAcceleratorCpSelectRight)
    ON_COMMAND(ID_ACCELERATOR_CP_SELECT_UP, &CSmartMetDoc::OnAcceleratorCpSelectUp)
    ON_COMMAND(ID_ACCELERATOR_CP_SELECT_DOWN, &CSmartMetDoc::OnAcceleratorCpSelectDown)
    ON_COMMAND(ID_EDIT_SOUNDING_DATA_FROM_SERVER_SETTINGS, &CSmartMetDoc::OnEditSoundingDataFromServerSettings)
    ON_COMMAND(ID_VIEW_SET_SOUNDINGS_FROM_SERVER_SETTINGS_PLACE_TO_DEFAULT, &CSmartMetDoc::OnViewSetSoundingsFromServerSettingsPlaceToDefault)
    ON_COMMAND(ID_BUTTON_BETA_PRODUCT_DIALOG, &CSmartMetDoc::OnButtonBetaProductDialog)
    ON_COMMAND(ID_ACCELERATOR_DO_VISUALIZATION_PROFILING, &CSmartMetDoc::OnAcceleratorDoVisualizationProfiling)
    ON_COMMAND(ID_VIEW_SET_LOG_VIEWER_DLG_PLACE_TO_DEFAULT, &CSmartMetDoc::OnViewSetLogViewerDlgPlaceToDefault)
	ON_COMMAND(ID_VIEW_SET_CASE_STUDY_DLG_PLACE_TO_DEFAULT, &CSmartMetDoc::OnViewSetCaseStudyDlgPlaceToDefault)
	ON_COMMAND(ID_VIEW_SET_BETA_PRODUCTION_DLG_PLACE_TO_DEFAULT, &CSmartMetDoc::OnViewSetBetaProductionDlgPlaceToDefault)
	ON_COMMAND(ID_EDIT_VISUALIZATIONSETTINGS, &CSmartMetDoc::OnEditVisualizationsettings)
	ON_COMMAND(ID_MOVEVIEWSVISIBLE_VISUALIZATIONSETTINGSPOSITION, &CSmartMetDoc::OnMoveviewsvisibleVisualizationsettingsposition)
		ON_COMMAND(ID_ACCELERATOR_MOVE_TIME_BOX_LOCATION, &CSmartMetDoc::OnAcceleratorMoveTimeBoxLocation)
		ON_COMMAND(ID_EDIT_GENERATE_NEW_WMS_SYSTEM, &CSmartMetDoc::OnEditGenerateNewWmsSystem)
		ON_UPDATE_COMMAND_UI(ID_EDIT_GENERATE_NEW_WMS_SYSTEM, &CSmartMetDoc::OnUpdateEditGenerateNewWmsSystem)
		ON_COMMAND(ID_ACCELERATOR_TOGGLE_VIRTUAL_TIME_MODE, &CSmartMetDoc::OnAcceleratorToggleVirtualTimeMode)
		ON_COMMAND(ID_EDIT_VIRTUAL_TIME_MODE, &CSmartMetDoc::OnEditVirtualTimeMode)
		ON_UPDATE_COMMAND_UI(ID_EDIT_VIRTUAL_TIME_MODE, &CSmartMetDoc::OnUpdateEditVirtualTimeMode)
		ON_COMMAND(ID_VIEW_MACROPARAMDATAGENERATION, &CSmartMetDoc::OnViewMacroparamdatageneration)
		ON_COMMAND(ID_MOVEVIEWSVISIBLE_MACROPARAMDATAGENERATORPOSITION, &CSmartMetDoc::OnMoveviewsvisibleMacroparamdatageneratorposition)
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
,itsWindTableDlg(nullptr)
,itsDataQualityCheckerDialog(nullptr)
,itsMapViewDescTopIndex(0)
,itsExtraMapViewDlg1(nullptr)
,itsExtraMapViewDlg2(nullptr)
,itsIgnoreStationsDlg(nullptr)
,itsCaseStudyDlg(nullptr)
,itsBetaProductDialog(nullptr)
,itsLogViewer(nullptr)
,itsSoundingDataServerConfigurationsDlg(nullptr)
,itsParameterSelectionDlg(nullptr)
,itsGriddingOptionsDlg(nullptr)
,itsVisualizationSettings(nullptr)
,itsMacroParamDataGeneratorDlg(nullptr)
{
	CSmartMetApp *app = (CSmartMetApp *)AfxGetApp();
	itsData = app->GeneralDocData();

    // Pit�� luoda n�m� ikkunant heti kun SmartMet aukeaa. T�rm�sin ongelmiin, kun poistin luonnin t�st�,
    // ett� satunnaisen oloisesti eri n�ytt�makrojen lataus saattoi sotkea eri n�ytt�jen ikkuna koot ja paikat,
    // niin ett� (satunnaiset?) n�yt�t yrittiv�t ottaa yhden ja saman CRect olion arvon itselleen.
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

static void DestroyModalessDialog(CDialog **theDlg) // annetaan pointterin pointteri sis��n, ett� pointteri voidaan lopuksi nollata
{
	if((*theDlg) != 0)
	{
		// t�ll� DestroyWindow-jutulla ei tainnut olla vaikutusta, mutta viel� modaalittomat dialogit vuotavat jotain sanoo deleaker-ohjelma
//		(*theDlg)->DestroyWindow(); // t�m� tuhoaa modaless dialogin sisukset
		delete (*theDlg); // t�m� tuhoaa sen kuoret
		(*theDlg) = 0; // t�ss� viel� asetetaan kyseisen dialogin pointteri 0:ksi
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
	::DestroyModalessDialog((CDialog **)(&itsWindTableDlg));
	::DestroyModalessDialog((CDialog **)(&itsDataQualityCheckerDialog));
	::DestroyModalessDialog((CDialog **)(&itsExtraMapViewDlg1));
	::DestroyModalessDialog((CDialog **)(&itsExtraMapViewDlg2));
	::DestroyModalessDialog((CDialog **)(&itsIgnoreStationsDlg));
    ::DestroyModalessDialog((CDialog **)(&itsCaseStudyDlg));
    ::DestroyModalessDialog((CDialog **)(&itsBetaProductDialog));
    ::DestroyModalessDialog((CDialog **)(&itsGriddingOptionsDlg));

    // Better clear callback function before logviewer is destroyed, just in case
    CatLog::setLogViewerUpdateCallback(std::function<void()>());
    ::DestroyModalessDialog((CDialog **)(&itsLogViewer));
	::DestroyModalessDialog((CDialog**)(&itsSoundingDataServerConfigurationsDlg));
    ::DestroyModalessDialog((CDialog **)(&itsParameterSelectionDlg));
	::DestroyModalessDialog((CDialog **)(&itsVisualizationSettings));
	::DestroyModalessDialog((CDialog**)(&itsMacroParamDataGeneratorDlg));
}

BOOL CSmartMetDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: add reinitialization code here
	// (SDI documents will reuse this document)
/*
	// HUOM! OTA POIS T�M� ALKU TESTIEN J�LKEEN!!!!!!
	CFmiImageFromWeb imageFromWeb;
	imageFromWeb.GetImage();
	// HUOM! OTA POIS T�M� ALKU TESTIEN J�LKEEN!!!!!!
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
	itsData->LogMessage("Opening Zoom dialog.", CatLog::Severity::Info, CatLog::Category::Operational);
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
			itsData->LogMessage("Closing time serial view.", CatLog::Severity::Info, CatLog::Category::Operational);
		}
		else
		{ // laitetaan aikasarja ikkuna p��lle
			itsTimeSerialDataEditorDlg->ShowWindow(SW_SHOW);	// Vaihdoin SW_RESTOREN, muistaa ikkunan muutetun koon.
			itsTimeSerialDataEditorDlg->SetActiveWindow();
			itsTimeSerialDataEditorDlg->Update();
			itsData->LogMessage("Opening time serial view.", CatLog::Severity::Info, CatLog::Category::Operational);
		}
		itsData->TimeSerialDataViewOn(!itsData->TimeSerialDataViewOn());
	}
}

void CSmartMetDoc::OnUpdateButtonTimeEditValuesDlg(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(itsData->TimeSerialDataViewOn());
}

void CSmartMetDoc::ActivateParameterSelectionDlg(unsigned int theDesktopIndex)
{
    if(!itsParameterSelectionDlg)
        CreateParameterSelectionDlg(itsData);

    itsParameterSelectionDlg->ShowWindow(SW_SHOW);	// Vaihdoin SW_RESTOREN, muistaa ikkunan muutetun koon.
    itsParameterSelectionDlg->SetActiveWindow();
	itsParameterSelectionDlg->SetIndexes(theDesktopIndex);
	itsData->LogMessage("Parameter Selection dialog on", CatLog::Severity::Info, CatLog::Category::Operational);
}


// Jos datan area ja kartta ovat "samanlaisia", laitetaan zoomiksi editoiavan datan alue
// muuten laitetaan kurrentti kartta kokonaisuudessaan n�kyviin.
void CSmartMetDoc::OnButtonDataArea()
{
	itsData->GetCombinedMapHandler()->onButtonDataArea(itsMapViewDescTopIndex);
}

void CSmartMetDoc::LoadDataOnStartUp(void)
{
    CtrlViewUtils::FmiSmartMetEditingMode oldMode = itsData->SmartMetEditingMode();
    // asetetaan alkulatauksen ajaksi sen oma moodi p��lle
	itsData->SmartMetEditingMode(CtrlViewUtils::kFmiEditingModeStartUpLoading, false); // ei muutet� asetusta setting:eihin asti (false), koska t�m� on vain v�liaikainen muutos, lopussa palautetaan arvo takaisin
	try
	{
		OnButtonLoadData();
	}
	catch(std::exception &e)
	{
		std::string errStr("Error while doing CSmartMetDoc::LoadDataOnStartUp:\n");
		errStr += e.what();
		itsData->LogAndWarnUser(errStr, "Error while loading data at startup", CatLog::Severity::Error, CatLog::Category::Data, false);
	}
	catch(...)
	{
		std::string errStr("Unknown error while doing CSmartMetDoc::LoadDataOnStartUp");
		itsData->LogAndWarnUser(errStr, "Error while loading data at startup", CatLog::Severity::Error, CatLog::Category::Data, false);
	}
	itsData->SmartMetEditingMode(oldMode, false);
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
	bool fWasOnNormalMode = !itsData->IsOperationalModeOn();
	auto editorModeDataWCTR = itsData->EditorModeDataWCTR();
	if(fWasOnNormalMode)
	{ 
		// Ik�v�� kikka vitoislua, t�ll� estet��n kun ollaan normaali moodissa, ja viel� lataus dialogissa
		// ettei SmartMet mene laittamaan datan lataus threadeja p��lle.
		editorModeDataWCTR->InNormalModeStillInDataLoadDialog(true);
	}

	bool oldValue = editorModeDataWCTR->UseNormalModeForAWhile();
	editorModeDataWCTR->UseNormalModeForAWhile(false); // pit�� laittaa varmuuden vuoksi pois p��lt� latauksen ajaksi
	bool isViewMode = (itsData->SmartMetEditingMode() != CtrlViewUtils::kFmiEditingModeNormal);
	if(isViewMode || dialog.DoModal() == IDOK) // jos ollaan ns. view-moodissa, ei dialogia avata vaan mennaan oletus arvoilla eteenpain
	{
		editorModeDataWCTR->InNormalModeStillInDataLoadDialog(false);
		CWaitCursor cursor;
		SetCursor(AfxGetApp()->LoadStandardCursor(IDC_WAIT)); // �l� poista, muuten ei tule jostain syyst� tiimalasia!?!?!?
		itsData->LoadData(dialog.RemoveThundersOnLoad());
		if(!GetPathName().IsEmpty())
			SetPathName(_TEXT("_"));
		((CMainFrame*)AfxGetMainWnd())->StartDataLoadingWorkingThread(); // edell� load datassa ladattiin vain itse data, t�ss� k�ynnistet��n apudatan lataus heti
		UpdateAllViewsAndDialogs(std::string(__FUNCTION__) + ": edited data loaded");
	}
	else
	{
		editorModeDataWCTR->InNormalModeStillInDataLoadDialog(false);
		editorModeDataWCTR->UseNormalModeForAWhile(oldValue); // jos cancelilla ulos, voidaan laittaa vanha moodi p��lle takaisin

		if(fWasOnNormalMode)
		{
			// T�M� pikaviritys palauttaa vain timebag/editormode sekoilut paikallee, jos on ollut
			// normaalisti dataa ladattuna ja painaa Lataa-nappulaa ja painaa Peruuta-nappia
			boost::shared_ptr<NFmiFastQueryInfo> smart = itsData->EditedSmartInfo();
			if(smart)
			{
				string dataFileName(smart->DataFileName());
				string dataFilePattern(smart->DataFilePattern());
				bool dataWasDeleted = false;
				itsData->AddQueryData(smart->RefQueryData()->Clone(), dataFileName, dataFilePattern, NFmiInfoData::kEditable, "", dynamic_cast<NFmiSmartInfo*>(smart.get())->LoadedFromFile(), dataWasDeleted);
			}
			UpdateAllViewsAndDialogs(std::string(__FUNCTION__) + ": loading edited data canceled"); // n�ytt�� pit�� p�ivitt��, muuten ohjelma kaatuu kun drawparamit on tuhottu alta ja n�ytt�j� ei ole p�ivitetty oikein
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
	boost::shared_ptr<NFmiFastQueryInfo> smart = itsData->EditedSmartInfo();
	BOOL status = TRUE;
	if(smart != 0 && dynamic_cast<NFmiSmartInfo*>(smart.get())->IsDirty()) //jos on dataa jo ladattuna ja sit� on muutettu, talletetaan ensin
	{
		status = StoreEditedData();
	}
	return status;
}

BOOL CSmartMetDoc::StoreEditedData(void)
{
	boost::shared_ptr<NFmiFastQueryInfo> smart = itsData->EditedSmartInfo();
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
	itsData->LogMessage("Saving edited data.", CatLog::Severity::Info, CatLog::Category::Editing);
	StoreEditedData();
}

BOOL CSmartMetDoc::StoreWorkingData(boost::shared_ptr<NFmiFastQueryInfo> &smart, bool askForSave)
{
	NFmiDataLoadingInfo& info = itsData->GetUsedDataLoadingInfo();
	info.InitFileNameLists();
	NFmiString fileNameWithPath(info.CreateWorkingFileName(info.LatestWorkingVersion()+1));
	return StoreData(fileNameWithPath, smart, askForSave);
}

void CSmartMetDoc::DoAutoSave(void)
{
	itsData->DoAutoSaveData();
}

BOOL CSmartMetDoc::StoreDataBaseDataMarko(boost::shared_ptr<NFmiFastQueryInfo> &smart)
{
	((CFrameWnd*)AfxGetMainWnd())->SetMessageText(_TEXT("Sending data to database."));

	CSaveToDataBaseDlg dlg(SmartMetDocumentInterface::GetSmartMetDocumentInterfaceImplementation());
	if(dlg.DoModal() == IDOK)
	{
		CWaitCursor waitCursor;
		string forecasterId = CT2A(dlg.ForecasterIDStr());
		itsData->DataToDBCheckMethod(dlg.DataCheckMethod());
		bool success = itsData->StoreDataToDataBase(forecasterId);
		if(success && itsData->IsOperationalModeOn())
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
	boost::shared_ptr<NFmiFastQueryInfo> info = itsData->EditedSmartInfo();
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

// HUOM!!! laita t�m� kutsumaan NFmiDoc:in metodia!!!!!!
BOOL CSmartMetDoc::StoreData(const NFmiString& theFileName, boost::shared_ptr<NFmiFastQueryInfo> &theSmartInfo, bool askForSave)
{
	CSaveDataDlg dlg;
	if(askForSave) // lis�sin askForSaven, ett� ei aina kysytt�isi talletetaanko
	{
		if(dlg.DoModal() != IDOK)
			return FALSE;
	}
	if(theSmartInfo && (theFileName != "")) // MIT� JOS EI OLE NIME�!!!!!!!
	{
		boost::shared_ptr<NFmiQueryData> data = theSmartInfo->DataReference();
		bool status = StoreData(theFileName, data) == TRUE;
		if(status)
			dynamic_cast<NFmiSmartInfo*>(theSmartInfo.get())->Dirty(false);
		return status;
	}
	return FALSE;
}

// HUOM!!! T�m� funktio pit�� poistaa ja siirty� k�ytt�m��n NFmiDoc:in metodia!!!!/Marko
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
	COptionsDlg dlg;
	bool oldDoCacheSetting = itsData->HelpDataInfoSystem()->UseQueryDataCache();
	dlg.SetData(SmartMetDocumentInterface::GetSmartMetDocumentInterfaceImplementation());
	if(dlg.DoModal() == IDOK)
	{
		itsData->StoreOptionsData();
		auto loadDataAtStartUp = itsData->ApplicationWinRegistry().ConfigurationRelatedWinRegistry().LoadDataAtStartUp();
		auto autoLoadNewCacheDataMode = itsData->ApplicationWinRegistry().ConfigurationRelatedWinRegistry().AutoLoadNewCacheData();
		auto* helpDataInfoSystem = itsData->HelpDataInfoSystem();
		QueryDataToLocalCacheLoaderThread::UpdateSettings(*helpDataInfoSystem, loadDataAtStartUp, autoLoadNewCacheDataMode);
		itsData->GetCombinedMapHandler()->mapViewDirty(itsMapViewDescTopIndex, true, true, true, false, false, false);
		bool cacheSettingChanged = oldDoCacheSetting != helpDataInfoSystem->UseQueryDataCache();
		if(cacheSettingChanged)
		{
			CFmiCombineDataThread::InitCombineDataInfos(*helpDataInfoSystem, itsData->ApplicationDataBase().GetDecodedApplicationDirectory()); // kun cache asetukset muuttuvat, pit�� yhdistelm� data asetuksia p�vitt��
			if(helpDataInfoSystem->UseQueryDataCache())
				((CMainFrame*)AfxGetMainWnd())->StartQDataCacheThreads();
			else
				((CMainFrame*)AfxGetMainWnd())->StopQDataCacheThreads();
		}
		UpdateAllViewsAndDialogs(std::string(__FUNCTION__) + ": Settings dialog closed with Ok");
	}
}

void CSmartMetDoc::OnSelectAll()
{
	if(itsData->SelectAllLocations(true))
	{
		itsData->LogMessage("Select all grid points for editing.", CatLog::Severity::Info, CatLog::Category::Editing);
		itsData->GetCombinedMapHandler()->mapViewDirty(itsMapViewDescTopIndex, false, false, true, false, false, false); // t�m� pit�isi hoitaa j�rkev�mmin, sill� t�m� asetetaan true:ksi edellisess� funktiokutsussa
		ApplyUpdatedViewsFlag(SmartMetViewId::AllMapViews | SmartMetViewId::TimeSerialView);
		UpdateAllViewsAndDialogs(std::string(__FUNCTION__) + ": selected all edited data grid points");
	}
}

void CSmartMetDoc::OnDeselectAll()
{
	if(itsData->SelectAllLocations(false))
	{
		itsData->GetCombinedMapHandler()->mapViewDirty(itsMapViewDescTopIndex, false, false, true, false, false, false); // t�m� pit�isi hoitaa j�rkev�mmin, sill� t�m� asetetaan true:ksi edellisess� funktiokutsussa
		ApplyUpdatedViewsFlag(SmartMetViewId::AllMapViews | SmartMetViewId::TimeSerialView);
		UpdateAllViewsAndDialogs(std::string(__FUNCTION__) + ": deselected all edited data grid points");
	}
}

void CSmartMetDoc::OnEditUndo()
{
	itsData->OnButtonUndo();
}

void CSmartMetDoc::OnEditRedo()
{
	itsData->OnButtonRedo();
}

void CSmartMetDoc::OnButtonSelectionToolDlg()
{
	if(!itsLocationSelectionToolDlg)
		CreateLocationSelectionToolDialog(itsData);
	if(itsLocationSelectionToolDlg->ShowWindow(SW_RESTORE ))	// Vaihdoin SW_RESTOREN, muistaa ikkunan muutetun koon.
		itsLocationSelectionToolDlg->SetActiveWindow();
	itsData->LogMessage("Opening editing grid point selector tool.", CatLog::Severity::Info, CatLog::Category::Operational);
}

void CSmartMetDoc::OnMenuitemSaveAs()
{
	boost::shared_ptr<NFmiFastQueryInfo> info = itsData->EditedSmartInfo();
	if(info)
	{
		CFileDialog dlg(FALSE, _TEXT("sqd")); //muutin sqd:ksi
        dlg.m_ofn.lpstrInitialDir = CA2T(itsData->FileDialogDirectoryMemory().c_str());
		if(dlg.DoModal() == IDOK)
		{
			NFmiString fileName = CT2A(dlg.GetPathName());
			BOOL status = StoreData(fileName,info);
			itsData->LogMessage("Save as edited data...", CatLog::Severity::Info, CatLog::Category::Data);
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

void CSmartMetDoc::CreateParameterSelectionDlg(NFmiEditMapGeneralDataDoc *theDoc)
{
    CreateModalessDialog(&itsParameterSelectionDlg, IDD_DIALOG_PARAM_ADDING, SmartMetDocumentInterface::GetSmartMetDocumentInterfaceImplementation());
}

void CSmartMetDoc::CreateMacroParamDataGeneratorDlg()
{
	CreateModalessDialog(&itsMacroParamDataGeneratorDlg, IDD_DIALOG_MACRO_PARAM_DATA_GENERATOR, SmartMetDocumentInterface::GetSmartMetDocumentInterfaceImplementation(), false);
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

	POSITION pos = GetFirstViewPosition();	// annetaan aikasarjaikkunalle cview parentti-ikkunaksi n�yt�n p�ivityksi� varten
	CView* pView = GetNextView(pos);		// ei toimi jos tulevaisuudessa vaihdetaan n�ytt�� kesken kaiken (esim. multi-doc-view)

	std::string titleStr = ::GetDictionaryString("Case study dialog");
    itsCaseStudyDlg = new CFmiCaseStudyDlg(&theDoc->GetSmartMetDocumentInterface(), titleStr, pView);
	BOOL status = itsCaseStudyDlg->Create(pView->GetDesktopWindow());
}

void CSmartMetDoc::CreateBetaProductDialog(SmartMetDocumentInterface *smartMetDocumentInterface)
{
    if(itsBetaProductDialog)
        return ; // Ei luoda dialogia uudestaan, t�t� kutsutaan my�s GeneralDataDocista, joka ei tie� onko dialogi jo olemassa

    POSITION pos = GetFirstViewPosition();	// annetaan aikasarjaikkunalle cview parentti-ikkunaksi n�yt�n p�ivityksi� varten
    CView* pView = GetNextView(pos);		// ei toimi jos tulevaisuudessa vaihdetaan n�ytt�� kesken kaiken (esim. multi-doc-view)

    itsBetaProductDialog = new CFmiBetaProductTabControlDialog(smartMetDocumentInterface, pView);
    BOOL status = itsBetaProductDialog->Create(pView->GetDesktopWindow());
}

void CSmartMetDoc::CreateLogViewer(NFmiEditMapGeneralDataDoc *theDoc)
{
    if(itsLogViewer)
        return; // Ei luoda dialogia uudestaan, t�t� kutsutaan my�s GeneralDataDocista, joka ei tie� onko dialogi jo olemassa

    auto *desktopView = ApplicationInterface::GetSmartMetView()->GetDesktopWindow();
    itsLogViewer = new CFmiLogViewer(theDoc->ApplicationWinRegistry());
    BOOL status = itsLogViewer->Create(IDD_DIALOG_LOG_VIEWER, desktopView);
    if(status)
    {
        auto refreshCallback = [this]() { this->itsLogViewer->StartRefreshTimer(); };
        CatLog::setLogViewerUpdateCallback(refreshCallback);
    }
}

void CSmartMetDoc::CreateSoundingDataServerConfigurationsDlg()
{
    if(itsSoundingDataServerConfigurationsDlg)
        return; // Ei luoda dialogia uudestaan

    CSmartMetView *smartmetView = ApplicationInterface::GetSmartMetView();
    itsSoundingDataServerConfigurationsDlg = new CFmiSoundingDataServerConfigurationsDlg(&itsData->GetSmartMetDocumentInterface(), smartmetView);
    BOOL status = itsSoundingDataServerConfigurationsDlg->Create(IDD_DIALOG_SOUNDING_DATA_SERVER_CONFIGURATIONS, smartmetView->GetDesktopWindow());
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
			itsData->LogMessage("Closes Ignore stations dialog.", CatLog::Severity::Info, CatLog::Category::Operational);
		}
		else
		{ // laitetaan aikasarja ikkuna p��lle
			itsIgnoreStationsDlg->ShowWindow(SW_SHOW);	// Vaihdoin SW_RESTOREN, muistaa ikkunan muutetun koon.
			itsIgnoreStationsDlg->SetActiveWindow();
			itsData->IgnoreStationsData().IgnoreStationsDialogOn(true);
//			itsIgnoreStationsDlg->Update();
			itsData->LogMessage("Open Ignore station dialog.", CatLog::Severity::Info, CatLog::Category::Operational);
		}
		itsData->TimeSerialDataViewOn(!itsData->TimeSerialDataViewOn());
	}
}

// t�m� create dialogi todella luo uuden dialogin ja palauttaa
// sen toisin kuin muut vastaavat dialogit, jotka luovat haluttuun member-data-pointteriin dialogin
CFmiExtraMapViewDlg* CSmartMetDoc::CreateExtraMapViewDlg(NFmiEditMapGeneralDataDoc *theDoc, unsigned int theMapViewDescTopIndex)
{
	POSITION pos = GetFirstViewPosition();	// annetaan aikasarjaikkunalle cview parentti-ikkunaksi n�yt�n p�ivityksi� varten
	CView* pView = GetNextView(pos);		// ei toimi jos tulevaisuudessa vaihdetaan n�ytt�� kesken kaiken (esim. multi-doc-view)

	CFmiExtraMapViewDlg *extraMapViewDlg = new CFmiExtraMapViewDlg(SmartMetDocumentInterface::GetSmartMetDocumentInterfaceImplementation(), theMapViewDescTopIndex);
	BOOL status = extraMapViewDlg->Create(IDD_DIALOG_MAP_VIEW_EXTRA, pView->GetDesktopWindow());
	if(status)
	{
		// asetetaan viel� karttan�yt�n otsikko
		string magickWord("ID_EXTRA_MAP_VIEW_");
		magickWord += NFmiStringTools::Convert<unsigned int>(theMapViewDescTopIndex);
		magickWord += "_TOOLTIP"; // otetaan extrakarttan�yt�n otsikko extrakarttan�yt�n toolbar-buttonin tooltip arvosta
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
		delete itsSynopPlotSettingsDlg; // jos joskus pit�� v�lill� tuhota aikasarjaeditori, t�m� ei riit�, koska t�m�n j�lkeen luotu dialogi tulee olemaan aina p��ohjelman p��ll�?!?

	POSITION pos = GetFirstViewPosition();	// annetaan aikasarjaikkunalle cview parentti-ikkunaksi n�yt�n p�ivityksi� varten
	CView* pView = GetNextView(pos);		// ei toimi jos tulevaisuudessa vaihdetaan n�ytt�� kesken kaiken (esim. multi-doc-view)

	itsSynopPlotSettingsDlg = new CFmiSynopPlotSettingsDlg(SmartMetDocumentInterface::GetSmartMetDocumentInterfaceImplementation(), pView);
	BOOL status = itsSynopPlotSettingsDlg->Create(IDD_DIALOG_SYNOP_PLOT_SETTINGS, pView->GetDesktopWindow());
}

void CSmartMetDoc::CreateLocationSelectionToolDialog(NFmiEditMapGeneralDataDoc * theDoc)
{
	if(itsLocationSelectionToolDlg)
		delete itsLocationSelectionToolDlg; // jos joskus pit�� v�lill� tuhota aikasarjaeditori, t�m� ei riit�, koska t�m�n j�lkeen luotu dialogi tulee olemaan aina p��ohjelman p��ll�?!?

	POSITION pos = GetFirstViewPosition();	// annetaan aikasarjaikkunalle cview parentti-ikkunaksi n�yt�n p�ivityksi� varten
	CView* pView = GetNextView(pos);		// ei toimi jos tulevaisuudessa vaihdetaan n�ytt�� kesken kaiken (esim. multi-doc-view)

	itsLocationSelectionToolDlg = new CFmiLocationSelectionToolDlg2(SmartMetDocumentInterface::GetSmartMetDocumentInterfaceImplementation());
	BOOL status = itsLocationSelectionToolDlg->Create(IDD_DIALOG_LOCATION_SELECTION_TOOL2, pView->GetDesktopWindow());
}

void CSmartMetDoc::CreateZoomDlg(NFmiEditMapGeneralDataDoc * theDoc)
{
	if(itsZoomDlg)
		delete itsZoomDlg; // jos joskus pit�� v�lill� tuhota zoomi-dialogi, t�m� ei riit�, koska t�m�n j�lkeen luotu dialogi tulee olemaan aina p��ohjelman p��ll�?!?

	POSITION pos = GetFirstViewPosition();	// annetaan aikasarjaikkunalle cview parentti-ikkunaksi n�yt�n p�ivityksi� varten
	CView* pView = GetNextView(pos);		// ei toimi jos tulevaisuudessa vaihdetaan n�ytt�� kesken kaiken (esim. multi-doc-view)

	itsZoomDlg = new CZoomDlg(SmartMetDocumentInterface::GetSmartMetDocumentInterfaceImplementation(), pView);
	BOOL status = itsZoomDlg->Create(IDD_DIALOG_ZOOM_MAP, pView->GetDesktopWindow());
}

void CSmartMetDoc::OnButtonSelectEuropeMap()
{
	itsData->LogMessage("Map 3 selected.", CatLog::Severity::Info, CatLog::Category::Operational);
	itsData->GetCombinedMapHandler()->setSelectedMapHandler(itsMapViewDescTopIndex, 2);
	UpdateAllViewsAndDialogs("Map area 3 selected in main map view");
}

void CSmartMetDoc::OnButtonSelectFinlandMap()
{
	itsData->LogMessage("Map 1 selected.", CatLog::Severity::Info, CatLog::Category::Operational);
	itsData->GetCombinedMapHandler()->setSelectedMapHandler(itsMapViewDescTopIndex, 0);
	UpdateAllViewsAndDialogs("Map area 1 selected in main map view");
}

void CSmartMetDoc::OnButtonSelectScandinaviaMap()
{
	itsData->LogMessage("Map 2 selected.", CatLog::Severity::Info, CatLog::Category::Operational);
	itsData->GetCombinedMapHandler()->setSelectedMapHandler(itsMapViewDescTopIndex, 1);
	UpdateAllViewsAndDialogs("Map area 2 selected in main map view");
}

bool CSmartMetDoc::IsMapSelected(int theMapIndex)
{
    return itsData->ApplicationWinRegistry().ConfigurationRelatedWinRegistry().MapView(itsMapViewDescTopIndex)->SelectedMapIndex() == theMapIndex;
}

void CSmartMetDoc::OnUpdateButtonSelectScandinaviaMap(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(IsMapSelected(1));
}

void CSmartMetDoc::OnButtonGlobe()
{
	itsData->LogMessage("Map 4 selected.", CatLog::Severity::Info, CatLog::Category::Operational);
	itsData->GetCombinedMapHandler()->setSelectedMapHandler(itsMapViewDescTopIndex, 3);
	UpdateAllViewsAndDialogs("Map area 4 selected in main map view");
}

void CSmartMetDoc::OnButtonDataToDatabase()
{
	boost::shared_ptr<NFmiFastQueryInfo> editedInfo = itsData->EditedSmartInfo();
	NFmiSmartInfo *smart = dynamic_cast<NFmiSmartInfo*>(editedInfo.get());
	if(smart == 0)
	{
		CNoSaveInfoDlg dlg;
		if(dlg.DoModal() == IDOK)
			return;
	}
	else
	{
		NFmiMetTime origTime(1); // l�hetett�ess� data tietokantaan, tehd��n datalle minuutin tarkuudella origin-aika
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
			if(itsData->CheckEditedDataForStartUpLoadErrors(MB_OKCANCEL | MB_ICONERROR))
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
    dlg.m_ofn.lpstrInitialDir = CA2T(itsData->FileDialogDirectoryMemory().c_str());

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
				itsData->MakeAndStoreFileDialogDirectoryMemory(fileName);
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
        std::unique_ptr<NFmiQueryData> data = QueryDataReading::ReadDataFromFile(theFileName, itsData->ApplicationWinRegistry().ConfigurationRelatedWinRegistry().DroppedDataEditable() == false);
        if(data)
        {
			bool dataWasDeleted = false;
			itsData->AddQueryData(data.release(), theFileName, "", NFmiInfoData::kEditable, "", true, dataWasDeleted);
            SetPathName(CA2T(theFileName)); // pit�� laittaa AddData:n j�lkeen, muuten tiedoston nimi ei tule oikein ohjelman otsikkoon.
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

// l�hinn� undo- ja redo-nappuloiden p�ivitys (aikasarjadialogissa ja filtter�intidialogissa)
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
    doc->GetCombinedMapHandler()->mapViewDirty(mapViewIndex, false, false, true, false, false, false);
    if(extraMapView)
        extraMapView->Update();
}

bool CSmartMetDoc::UpdateAllViewsAndDialogsIsAllowed()
{
    // printatessa ruutujen p�ivitys voi aiheuttaa ongelmia
    if(itsData->Printing())
        return false;
    // Uusien datan muokkaus rutiinien kanssa ruutujen p�ivitys voi aiheuttaa ongelmia 
    // Multi-thread ja/tai progress/cancel (-> erillinen muokkaus threadi) systeemit menev�t jotenkin sekaisin
    if(itsData->DataModificationInProgress())
        return false;

    return true;
}

static void MakeGeneralUpdateTraceLogging(const std::string &totalLogMessage)
{
    // OBS! Don't do the CatLog::doTraceLevelLogging check here, because we want that updateCounter keeps incremented through out all application life time.

    CtrlViewUtils::CtrlViewTimeConsumptionReporter::increaseCurrentUpdateId();
    // HUOM! ei kannata mitata koko UpdateAllViewsAndDialogs metodin k�ytt�m�� aikaa, koska se vain tekee p�ivitys pyynt�j� eri ikkunoille,
    // joille win32 systeemi jakaa k�skyj� miten haluaa. Eli kokonais SmartMet ikkunoiden p�ivitys aikaa on mahdoton koostaa suoraan.
    CtrlViewUtils::CtrlViewTimeConsumptionReporter::makeSeparateTraceLogging(totalLogMessage, nullptr);
}

void CSmartMetDoc::UpdateAllViewsAndDialogs(const std::string &reasonForUpdate, bool fUpdateOnlyMapViews)
{
    if(UpdateAllViewsAndDialogsIsAllowed())
    {
		itsData->ApplicationWinRegistry().VisualizationSpaceoutSettings().doViewUpdateWarningLogsIfNeeded();
        if(HasUpdatedViewsFlagSignificantValue())
        {
            // Jos haluttiin vain tietyt n�yt�t p�ivitykseen, tehd��n se t�ss�
            UpdateAllViewsAndDialogs(reasonForUpdate, GetAndResetUpdatedViewsFlag());
        }
        else
        {
            ::MakeGeneralUpdateTraceLogging(std::string("***** ") + __FUNCTION__ + " ***** {Reason: " + reasonForUpdate + "}");
            if(fUpdateOnlyMapViews)
                CtrlViewUtils::CtrlViewTimeConsumptionReporter::makeSeparateTraceLogging(std::string(__FUNCTION__) + ": only map-views are updated", nullptr);

            itsData->SetLatestMacroParamErrorText("Starting view updates, no errors."); // 'nollataan' macroParam virhetekstiaina piirron aluksi, ettei j�� vanhoja muistiin

            if(fUpdateOnlyMapViews == false)
                ::UpdateModalessDialog(itsZoomDlg);

            ::UpdateMapView(itsData, ApplicationInterface::GetSmartMetView(), 0);
            ::UpdateMapView(itsData, itsExtraMapViewDlg1, 1);
            ::UpdateMapView(itsData, itsExtraMapViewDlg2, 2);

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
				MakeNextUpdateOnSynopDataGridViewDlgForced();
				::UpdateModalessDialog(itsSynopDataGridViewDlg);
                ::UpdateModalessDialog(itsTrajectoryDlg);
                ::UpdateModalessDialog(itsWarningCenterDlg);
                ::UpdateModalessDialog(itsWindTableDlg);
                //            ::UpdateModalessDialog(itsDataQualityCheckerDialog);
                ::UpdateModalessDialog(itsBetaProductDialog);
                ::UpdateModalessDialog(itsViewMacroDlg);
                ::UpdateModalessDialog(itsParameterSelectionDlg);
            }
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

void CSmartMetDoc::ApplyUpdatedViewsFlag(SmartMetViewId updatedViewsFlag) 
{ 
    // Lis�t��n jo olemassa olevaan maskiin parametrina annettu maski OR operandilla (voidaan vain lis�t�, nollaamiseen on oma metodi)
    itsUpdatedViewsFlag = itsUpdatedViewsFlag | updatedViewsFlag;
}

SmartMetViewId CSmartMetDoc::GetAndResetUpdatedViewsFlag() 
{ 
    SmartMetViewId returnedValue = itsUpdatedViewsFlag;
    itsUpdatedViewsFlag = SmartMetViewId::NoViews; 
    return returnedValue;
}

void CSmartMetDoc::UpdateAllViewsAndDialogs(const std::string& reasonForUpdate, SmartMetViewId updatedViewsFlag)
{
    // Otetaan GetAndResetUpdatedViewsFlag:ist� my�s mahdollinen dirty-mask mukaan lopulliseen p�ivitykseen (ja nollataan se samalla)
    updatedViewsFlag = updatedViewsFlag | GetAndResetUpdatedViewsFlag();
    if(UpdateAllViewsAndDialogsIsAllowed())
    {
        ::MakeGeneralUpdateTraceLogging(std::string("***** ") + __FUNCTION__ + "-v2 ***** {Reason: " + reasonForUpdate + "}");
        ::MakeUpdatedViewsTraceLogging(__FUNCTION__, updatedViewsFlag);

        itsData->SetLatestMacroParamErrorText("Starting view updates, no errors."); // 'nollataan' macroParam virhetekstiaina piirron aluksi, ettei j�� vanhoja muistiin

        if(SmartMetViewIdFlagCheck(updatedViewsFlag, SmartMetViewId::ZoomDlg))
            ::UpdateModalessDialog(itsZoomDlg);
        if(SmartMetViewIdFlagCheck(updatedViewsFlag, SmartMetViewId::MainMapView))
            ::UpdateMapView(itsData, ApplicationInterface::GetSmartMetView(), 0);
        if(SmartMetViewIdFlagCheck(updatedViewsFlag, SmartMetViewId::MapView2))
            ::UpdateMapView(itsData, itsExtraMapViewDlg1, 1);
        if(SmartMetViewIdFlagCheck(updatedViewsFlag, SmartMetViewId::MapView3))
            ::UpdateMapView(itsData, itsExtraMapViewDlg2, 2);
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
		{
			MakeNextUpdateOnSynopDataGridViewDlgForced();
			::UpdateModalessDialog(itsSynopDataGridViewDlg);
		}
        if(SmartMetViewIdFlagCheck(updatedViewsFlag, SmartMetViewId::TrajectoryView))
            ::UpdateModalessDialog(itsTrajectoryDlg);
        if(SmartMetViewIdFlagCheck(updatedViewsFlag, SmartMetViewId::WarningCenterDlg))
            ::UpdateModalessDialog(itsWarningCenterDlg);
        if(SmartMetViewIdFlagCheck(updatedViewsFlag, SmartMetViewId::WindTableDlg))
            ::UpdateModalessDialog(itsWindTableDlg);
        //            ::UpdateModalessDialog(itsDataQualityCheckerDialog);
        if(SmartMetViewIdFlagCheck(updatedViewsFlag, SmartMetViewId::BetaProductionDlg))
            ::UpdateModalessDialog(itsBetaProductDialog);
        if(SmartMetViewIdFlagCheck(updatedViewsFlag, SmartMetViewId::ViewMacroDlg))
            ::UpdateModalessDialog(itsViewMacroDlg);
        if(SmartMetViewIdFlagCheck(updatedViewsFlag, SmartMetViewId::ParameterSelectionDlg))
            ::UpdateModalessDialog(itsParameterSelectionDlg);
    }
}

void CSmartMetDoc::UpdateOnlyExtraMapViews(bool updateMap1, bool updateMap2)
{
	if(updateMap1)
        ::UpdateMapView(itsData, itsExtraMapViewDlg1, 1);

	if(updateMap2)
        ::UpdateMapView(itsData, itsExtraMapViewDlg2, 2);
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
	if(itsData->SmartMetEditingMode() == CtrlViewUtils::kFmiEditingModeNormal) // eli jos ollaan edit-moodissa, asetetaan nappulan tilaa
	{
		boost::shared_ptr<NFmiFastQueryInfo> editedInfo = itsData->EditedSmartInfo();
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
	if(itsData->SmartMetEditingMode() == CtrlViewUtils::kFmiEditingModeNormal) // eli jos ollaan edit-moodissa, asetetaan nappulan tilaa
	{
		boost::shared_ptr<NFmiFastQueryInfo> editedInfo = itsData->EditedSmartInfo();
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

void CSmartMetDoc::OnChangeParamWindowPositionForward()
{
	itsData->GetCombinedMapHandler()->onChangeParamWindowPosition(itsMapViewDescTopIndex, true);
}

void CSmartMetDoc::OnChangeParamWindowPositionBackward()
{
	itsData->GetCombinedMapHandler()->onChangeParamWindowPosition(itsMapViewDescTopIndex, false);
}

void CSmartMetDoc::OnMakeGridFile()
{
	CWaitCursor cursor;
	itsData->MakeGridFile("grid.txt");
	itsData->LogMessage("Storing active grid data to file.", CatLog::Severity::Debug, CatLog::Category::Data);
}

void CSmartMetDoc::OnButtonMakeEditedDataCopy()
{
	itsData->UpdateEditedDataCopy();
	itsData->LogMessage("Copy active grid data.", CatLog::Severity::Debug, CatLog::Category::Editing);
	UpdateAllViewsAndDialogs("Updated edited data copy");
}

void CSmartMetDoc::OnButtonFilterDialog()
{
	if(!itsFilterDlg)
		CreateFilterDlg(itsData);
	if(itsFilterDlg->ShowWindow(SW_RESTORE ))	// Vaihdoin SW_RESTOREN, muistaa ikkunan muutetun koon.
		itsFilterDlg->SetActiveWindow();
	itsData->FilterDialogUpdateStatus(2); // 2 = p�ivit� koko ruutu
	itsFilterDlg->Update();
	itsData->LogMessage("Opening Data Filter tool.", CatLog::Severity::Info, CatLog::Category::Editing);
}

void CSmartMetDoc::CreateFilterDlg(NFmiEditMapGeneralDataDoc * theDoc)
{
	if(itsFilterDlg)
		delete itsFilterDlg;

	POSITION pos = GetFirstViewPosition();	// annetaan aikasarjaikkunalle cview parentti-ikkunaksi n�yt�n p�ivityksi� varten
	CView* pView = GetNextView(pos);		// ei toimi jos tulevaisuudessa vaihdetaan n�ytt�� kesken kaiken (esim. multi-doc-view)

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

// t�m� hoitaa sek� multi-file drag'n dropin ja file-open dialog jutun
void CSmartMetDoc::LoadDataFromFilesAndAdd(std::vector<std::string> &theFileList)
{
	NFmiInfoData::Type datatype = NFmiInfoData::kEditable; // 1. data on aina editable
    if(theFileList.size())
    {
        for(size_t i = 0; i < theFileList.size(); i++)
        {
            LoadDataFromFileAndAdd(theFileList[i], datatype, itsData->ApplicationWinRegistry().ConfigurationRelatedWinRegistry().DroppedDataEditable() == false);
            datatype = NFmiInfoData::kViewable; // loput ovat ns. apudatoja eli kViewable:ja (LoadDataFromFileAndAdd metodissa viel� lis�� tarkasteluja)

            // Jos 1. data, joka laitetaan  editoitavaksi dataksi ja muitakin datoja on tiputettu,
            // laitetaan 1. data my�s viewable-dataksi. T�ll�in jos tiputat esim. Hirlamin pinta+painepinta ja mallipinta
            // datat editoriin, p��tyv�t ne kaikki my�s hirlam-valikoihin, eik� 1. j�� vain editoitavaksi dataksi.
            if(i == 0)
            {
                SetPathName(CA2T(theFileList[i].c_str())); // pit�� laittaa AddData:n j�lkeen, muuten tiedoston nimi ei tule oikein ohjelman otsikkoon.

                if(theFileList.size() > 1)
                    LoadDataFromFileAndAdd(theFileList[i], datatype, true);
            }
        }
        UpdateAllViewsAndDialogs("Data files where drag'n dropped on SmartMet");
    }
}

// T�m� on tehty ainakin file drop datan yhteydess� k�ytett�v�ksi.
// Oletus, t�t� ei kutsuta mist��n muualta kuin LoadDataFromFilesAndAdd metodista ja siell� tehd��n yhteinen UpdateAllViewsAndDialogs kutsu.
void CSmartMetDoc::LoadDataFromFileAndAdd(const std::string &theFileName, NFmiInfoData::Type theDatatype, bool useMemoryMapping)
{
    CString errorStringU_;
    try
	{
        std::unique_ptr<NFmiQueryData> data = QueryDataReading::ReadDataFromFile(theFileName, useMemoryMapping);
        if(data)
        {
            if(theDatatype == NFmiInfoData::kViewable)
            { // tarkistetaan ett� onko data hybridi dataa, jos on, laitetaan data tyypiksi hybridi
                if(::IsHybridData(data.get()))
                    theDatatype = NFmiInfoData::kHybridData;
                // sitten tarkistetaan onko se mahdollisesti havainto dataa
                if(data->Info() && data->Info()->IsGrid() == false)
                    theDatatype = NFmiInfoData::kObservations;
            }

            std::string filePattern = theFileName; // annetaan filepatterniksi tiputettujen datojen yhteydess� tiedoston nimi, muuten homma ei toimi kunnolla infoorganizerissa
			bool dataWasDeleted = false;
			itsData->AddQueryData(data.release(), theFileName, filePattern, theDatatype, "", true, dataWasDeleted);

            if(theDatatype == NFmiInfoData::kEditable)
            {
				SetPathName(CA2T(theFileName.c_str())); // pit�� laittaa AddData:n j�lkeen, muuten tiedoston nimi ei tule oikein ohjelman otsikkoon.
            }
        }
	}
	catch(char *msg)
	{
		itsData->LogMessage(msg, CatLog::Severity::Error, CatLog::Category::Data);
        errorStringU_ = CA2T(msg);
        CDataLoadingProblemsDlg dlg(errorStringU_);
		if(dlg.DoModal() == IDOK)
			return;
	}
	catch(exception &e)
	{
		itsData->LogMessage(e.what() , CatLog::Severity::Error, CatLog::Category::Data);
        errorStringU_ = CA2T(e.what());
        CDataLoadingProblemsDlg dlg(errorStringU_);
		if(dlg.DoModal() == IDOK)
			return;
	}
	catch(...)
	{
        errorStringU_ = _TEXT("Unknown error while readin data file: ");
		itsData->LogMessage(std::string(CT2A(errorStringU_)), CatLog::Severity::Error, CatLog::Category::Data);
        errorStringU_ += CA2T(theFileName.c_str());
        CDataLoadingProblemsDlg dlg(errorStringU_);
		if(dlg.DoModal() == IDOK)
			return ;
	}

}

// laittaa sivellinty�kalun p��lle/pois
void CSmartMetDoc::OnButtonBrushToolDlg()
{
	if(itsData->ModifyToolMode() != CtrlViewUtils::kFmiEditorModifyToolModeBrush)
	{
		itsData->LogMessage("Opening brush tool.", CatLog::Severity::Info, CatLog::Category::Editing);

		itsData->ModifyToolMode(CtrlViewUtils::kFmiEditorModifyToolModeBrush);
		if(!itsBrushToolDlg)
			CreateBrushToolDlg(itsData);
		itsBrushToolDlg->ShowWindow(SW_RESTORE);
	}
	else // Editor mode back to normal and hide dialog. This is not done when brush tool is closed from it's own window's close button!
	{
		itsData->LogMessage("Closing brush tool.", CatLog::Severity::Info, CatLog::Category::Editing);
		itsData->ModifyToolMode(CtrlViewUtils::kFmiEditorModifyToolModeNormal);
		if(itsBrushToolDlg)
			itsBrushToolDlg->ShowWindow(SW_HIDE);
	}
}

void CSmartMetDoc::CreateBrushToolDlg(NFmiEditMapGeneralDataDoc* theDoc)
{
	if(itsBrushToolDlg)
		delete itsBrushToolDlg;

	POSITION pos = GetFirstViewPosition();	// annetaan aikasarjaikkunalle cview parentti-ikkunaksi n�yt�n p�ivityksi� varten
	CView* pView = GetNextView(pos);		// ei toimi jos tulevaisuudessa vaihdetaan n�ytt�� kesken kaiken (esim. multi-doc-view)

	itsBrushToolDlg = new CFmiBrushToolDlg(&theDoc->GetSmartMetDocumentInterface(), pView);
	BOOL status = itsBrushToolDlg->Create();
}

void CSmartMetDoc::CreateLocationFinderDlg(NFmiEditMapGeneralDataDoc* theDoc)
{
	if(itsLocationFinderDlg)
		delete itsLocationFinderDlg;

	POSITION pos = GetFirstViewPosition();	// annetaan aikasarjaikkunalle cview parentti-ikkunaksi n�yt�n p�ivityksi� varten
	CView* pView = GetNextView(pos);		// ei toimi jos tulevaisuudessa vaihdetaan n�ytt�� kesken kaiken (esim. multi-doc-view)

	itsLocationFinderDlg = new CFmiLocationFinderDlg(SmartMetDocumentInterface::GetSmartMetDocumentInterfaceImplementation(), pView);
	BOOL status = itsLocationFinderDlg->Create();
}

void CSmartMetDoc::OnUpdateButtonBrushToolDlg(CCmdUI* pCmdUI)
{
	if(itsData->SmartMetEditingMode() == CtrlViewUtils::kFmiEditingModeNormal) // eli jos ollaan edit-moodissa, asetetaan nappulan tilaa
		pCmdUI->SetCheck(itsData->ModifyToolMode() == CtrlViewUtils::kFmiEditorModifyToolModeBrush);
	else	// jos ollaan ns. view-moodissa, disabloidaan koko nappula
		pCmdUI->Enable(FALSE);
}

void CSmartMetDoc::OnButtonOpenSmartToolDlg()
{
	itsData->LogMessage("Opening smarttool dialog.", CatLog::Severity::Info, CatLog::Category::Editing);

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

void CSmartMetDoc::CreateTempDlg()
{
    if(!itsTempDialog)
    {
        POSITION pos = GetFirstViewPosition();	// annetaan aikasarjaikkunalle cview parentti-ikkunaksi n�yt�n p�ivityksi� varten
        CView* pView = GetNextView(pos);		// ei toimi jos tulevaisuudessa vaihdetaan n�ytt�� kesken kaiken (esim. multi-doc-view)

        itsTempDialog = new CFmiTempDlg(SmartMetDocumentInterface::GetSmartMetDocumentInterfaceImplementation(), pView);
        BOOL status = itsTempDialog->Create(IDD_TEMP_DIALOG, pView->GetDesktopWindow());
    }
}

void CSmartMetDoc::CreateSmartToolDlg(NFmiEditMapGeneralDataDoc* theDoc)
{
	if(itsSmartToolDlg)
		delete itsSmartToolDlg;

	POSITION pos = GetFirstViewPosition();	// annetaan aikasarjaikkunalle cview parentti-ikkunaksi n�yt�n p�ivityksi� varten
	CView* pView = GetNextView(pos);		// ei toimi jos tulevaisuudessa vaihdetaan n�ytt�� kesken kaiken (esim. multi-doc-view)

	itsSmartToolDlg = new CFmiSmartToolDlg(SmartMetDocumentInterface::GetSmartMetDocumentInterfaceImplementation(), pView);
	BOOL status = itsSmartToolDlg->Create(IDD_DIALOG_SMART_TOOL, pView->GetDesktopWindow());
}

void CSmartMetDoc::CreateSmartToolsTabControlDlg(NFmiEditMapGeneralDataDoc* theDoc)
{
    if(itsSmarttoolsTabControlDlg)
        delete itsSmarttoolsTabControlDlg;

    POSITION pos = GetFirstViewPosition();	// annetaan aikasarjaikkunalle cview parentti-ikkunaksi n�yt�n p�ivityksi� varten
    CView* pView = GetNextView(pos);		// ei toimi jos tulevaisuudessa vaihdetaan n�ytt�� kesken kaiken (esim. multi-doc-view)

    itsSmarttoolsTabControlDlg = new CFmiSmarttoolsTabControlDlg(SmartMetDocumentInterface::GetSmartMetDocumentInterfaceImplementation(), pView);
    BOOL status = itsSmarttoolsTabControlDlg->Create(pView->GetDesktopWindow());
}

void CSmartMetDoc::MakeNextUpdateOnSynopDataGridViewDlgForced()
{
    if(itsSynopDataGridViewDlg)
        itsSynopDataGridViewDlg->MakeNextUpdateForced();
}

void CSmartMetDoc::OnButtonRefresh()
{
    MakeNextUpdateOnSynopDataGridViewDlgForced();
	itsData->OnButtonRefresh("Refreshing all views (F5)");
}

void CSmartMetDoc::OnButtonReloadAllDynamicHelpData()
{
    MakeNextUpdateOnSynopDataGridViewDlgForced();
	itsData->ReloadAllDynamicHelpData();
	// Ensin tyhjennyksen j�lkeen pit�� ladata editoitava data muistiin
	LoadDataOnStartUp();
	// Merkit��n taas aluksi luetut datat 'vanhoiksi'
	CFmiDataLoadingThread2::ResetFirstTimeGoingThroughState();
	// Sitten laitetaan normidatanlataus threadi taas toimimaan
	CFmiDataLoadingThread2::ResetTimeStamps();
	CFmiDataLoadingThread2::LoadDataNow();
	UpdateAllViewsAndDialogs("Reloading all the dynamic data");
}

void CSmartMetDoc::CaseStudyLoadingActions(const NFmiMetTime &theUsedTime, const std::string &updateReasonText)
{
	itsSynopDataGridViewDlg->MinMaxRangeStartTime(theUsedTime);
	CFmiDataLoadingThread2::SettingsChanged(*itsData->HelpDataInfoSystem(), true); // Datan lataus threadille laitetaan t�ss� uusi CaseStudy-helpDataInfoSetting-olio k�ytt��n
	CFmiDataLoadingThread2::ResetTimeStamps();
	CFmiDataLoadingThread2::LoadDataNow();
	itsData->GetCombinedMapHandler()->mapViewDirty(CtrlViewUtils::kDoAllMapViewDescTopIndex, false, true, true, false, false, false); // laitetaan kaikki kartta n�yt�t likaiseksi
	itsData->MacroParamDataCache().clearAllLayers();
	UpdateAllViewsAndDialogs(updateReasonText);
}

void CSmartMetDoc::CaseStudyToNormalModeActions(void)
{
	NFmiMetTime currentTime;
    CaseStudyLoadingActions(currentTime, "Coming back from case study mode to normal mode");
}

void CSmartMetDoc::SetAllViewIconsDynamically(void)
{
	auto usedIcons = ApplicationInterface::GetApplicationInterfaceImplementation()->GetUsedIcons();
	CFmiWin32Helpers::SetWindowIconDynamically(AfxGetMainWnd(), usedIcons);

	CFmiWin32Helpers::SetWindowIconDynamically(itsTimeSerialDataEditorDlg, usedIcons);
	CFmiWin32Helpers::SetWindowIconDynamically(itsLocationSelectionToolDlg, usedIcons);
	CFmiWin32Helpers::SetWindowIconDynamically(itsZoomDlg, usedIcons);
	CFmiWin32Helpers::SetWindowIconDynamically(itsFilterDlg, usedIcons);
	CFmiWin32Helpers::SetWindowIconDynamically(itsBrushToolDlg, usedIcons);
	CFmiWin32Helpers::SetWindowIconDynamically(itsLocationFinderDlg, usedIcons);
	CFmiWin32Helpers::SetWindowIconDynamically(itsSmartToolDlg, usedIcons);
	CFmiWin32Helpers::SetWindowIconDynamically(itsTempDialog, usedIcons);
	CFmiWin32Helpers::SetWindowIconDynamically(itsShortCutsDialog, usedIcons);
	CFmiWin32Helpers::SetWindowIconDynamically(itsViewMacroDlg, usedIcons);
	CFmiWin32Helpers::SetWindowIconDynamically(itsCrossSectionDlg, usedIcons);
	CFmiWin32Helpers::SetWindowIconDynamically(itsSynopPlotSettingsDlg, usedIcons);
	CFmiWin32Helpers::SetWindowIconDynamically(itsTrajectoryDlg, usedIcons);
	CFmiWin32Helpers::SetWindowIconDynamically(itsWarningCenterDlg, usedIcons);
	CFmiWin32Helpers::SetWindowIconDynamically(itsExtraMapViewDlg1, usedIcons);
	CFmiWin32Helpers::SetWindowIconDynamically(itsExtraMapViewDlg2, usedIcons);
	CFmiWin32Helpers::SetWindowIconDynamically(itsWindTableDlg, usedIcons);
	CFmiWin32Helpers::SetWindowIconDynamically(itsDataQualityCheckerDialog, usedIcons);
	CFmiWin32Helpers::SetWindowIconDynamically(itsIgnoreStationsDlg, usedIcons);
    CFmiWin32Helpers::SetWindowIconDynamically(itsCaseStudyDlg, usedIcons);
    CFmiWin32Helpers::SetWindowIconDynamically(itsBetaProductDialog, usedIcons);
    CFmiWin32Helpers::SetWindowIconDynamically(itsLogViewer, usedIcons);
	CFmiWin32Helpers::SetWindowIconDynamically(itsSoundingDataServerConfigurationsDlg, usedIcons);
    CFmiWin32Helpers::SetWindowIconDynamically(itsParameterSelectionDlg, usedIcons);
	CFmiWin32Helpers::SetWindowIconDynamically(itsVisualizationSettings, usedIcons);
	CFmiWin32Helpers::SetWindowIconDynamically(itsMacroParamDataGeneratorDlg, usedIcons);
}

// piti tehd� uuden karttaruudukon valinnan lis�ksi paikka mist�
// aukeaa vanha dialogi, koska citrix-linux yhdistelm�ll� uusi ruudukon valinta systeemi ei toimi
void CSmartMetDoc::OnMenuitemViewGridSelectionDlg2()
{
	auto* combinedMapHandler = itsData->GetCombinedMapHandler();
	NFmiPoint oldSize(combinedMapHandler->getMapViewDescTop(itsMapViewDescTopIndex)->ViewGridSize());
	CFmiViewGridSelectorDlg dlg(itsMapViewDescTopIndex, SmartMetDocumentInterface::GetSmartMetDocumentInterfaceImplementation());
	if(dlg.DoModal() == IDOK)
	{
		if(combinedMapHandler->setMapViewGrid(itsMapViewDescTopIndex, dlg.SelectedGridSize()))
			UpdateAllViewsAndDialogs("Changing main map view grid");
	}
}

void CSmartMetDoc::OnEditSpaceOut()
{
	itsData->GetCombinedMapHandler()->onEditSpaceOut(itsMapViewDescTopIndex);
}

void CSmartMetDoc::OnButtonValidateData()
{
	CWaitCursor cursor;
	itsData->LogMessage("Validation checking to edited data.", CatLog::Severity::Info, CatLog::Category::Editing);
	itsData->MakeDataValiditation();
	UpdateAllViewsAndDialogs("Edited data validitation modifications");
}

void CSmartMetDoc::OnUpdateButtonValidateData(CCmdUI* pCmdUI)
{
	if(itsData->SmartMetEditingMode() != CtrlViewUtils::kFmiEditingModeNormal) // jos ollaan ns. view-moodissa, disabloidaan koko nappula
		pCmdUI->Enable(FALSE);
}

void CSmartMetDoc::OnChangeMapType()
{
	itsData->OnChangeMapType(itsMapViewDescTopIndex, true);
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
	itsData->GetCombinedMapHandler()->onShowTimeString(itsMapViewDescTopIndex);
}

void CSmartMetDoc::OnShowGridPoints()
{
	itsData->GetCombinedMapHandler()->onShowGridPoints(itsMapViewDescTopIndex);
}

void CSmartMetDoc::OnToggleGridPointColor()
{
	itsData->GetCombinedMapHandler()->onToggleGridPointColor(itsMapViewDescTopIndex);
}

void CSmartMetDoc::OnToggleGridPointSize()
{
	itsData->GetCombinedMapHandler()->onToggleGridPointSize(itsMapViewDescTopIndex);
}

void CSmartMetDoc::OnToggleShowNamesOnMap()
{
	itsData->GetCombinedMapHandler()->onToggleShowNamesOnMap(itsMapViewDescTopIndex, true);
}

// vaihtaaa "n�yt� maskit kartalla" -tilaa ja p�ivitt�� ruudut
void CSmartMetDoc::OnShowMasksOnMap()
{
	itsData->OnShowMasksOnMap(itsMapViewDescTopIndex);
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
    ::SetViewPlaceToDefault(this, itsCrossSectionDlg, "Cross section view set to default size and position");
}

void CSmartMetDoc::OnSetParameterSelectionDlgPlaceToDefault()
{
    ::SetViewPlaceToDefault(this, itsParameterSelectionDlg, "Parameter Selection dialog set to default size and position");
}

void CSmartMetDoc::OnMoveviewsvisibleMacroparamdatageneratorposition()
{
	::SetViewPlaceToDefault(this, itsMacroParamDataGeneratorDlg, "MacroParam data generator dialog set to default size and position");
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
    ::SetViewPlaceToDefault(this, itsTrajectoryDlg, "Trajectory view set to default size and position");
}

void CSmartMetDoc::OnViewSetDataQualityCheckerDlgPlaceToDefault()
{
	if(itsDataQualityCheckerDialog)
	{
		itsDataQualityCheckerDialog->SetDefaultValues();
		itsDataQualityCheckerDialog->SetActiveWindow();
	}
}

void CSmartMetDoc::OnViewSetSoundingsFromServerSettingsPlaceToDefault()
{
    ::SetViewPlaceToDefault(this, itsSoundingDataServerConfigurationsDlg, "Sounding data from server settings dialog set to default size and position");
}

void CSmartMetDoc::OnViewSetLogViewerDlgPlaceToDefault()
{
	::SetViewPlaceToDefault(this, itsLogViewer, "Log viewer dialog set to default size and position");
}


void CSmartMetDoc::OnViewSetCaseStudyDlgPlaceToDefault()
{
	::SetViewPlaceToDefault(this, itsCaseStudyDlg, "Case study dialog set to default size and position");
}


void CSmartMetDoc::OnViewSetBetaProductionDlgPlaceToDefault()
{
	::SetViewPlaceToDefault(this, itsBetaProductDialog, "Beta production dialog set to default size and position");
}

void CSmartMetDoc::OnMoveviewsvisibleVisualizationsettingsposition()
{
	::SetViewPlaceToDefault(this, itsVisualizationSettings, "Visualization settings dialog set to default size and position");
}

void CSmartMetDoc::OnFileOpen()
{
	OnDataLoadFromFile();
}

void CSmartMetDoc::OnFileNew()
{
// ei tehd� mit��n/Marko
	int x = 0;
}

void CSmartMetDoc::OnButtonAnimation()
{
	itsData->ToggleTimeControlAnimationView(itsMapViewDescTopIndex);
}

void CSmartMetDoc::OnButtonEditorControlPointMode()
{
	itsData->MetEditorOptionsData().ControlPointMode(!itsData->MetEditorOptionsData().ControlPointMode());
	if(itsData->MetEditorOptionsData().ControlPointMode())
		itsData->LogMessage("Opening Control point tool.", CatLog::Severity::Info, CatLog::Category::Editing);
	else
		itsData->LogMessage("Closing Control point tool.", CatLog::Severity::Info, CatLog::Category::Editing);

	itsData->GetCombinedMapHandler()->mapViewDirty(itsMapViewDescTopIndex, false, false, true, false, false, false);
	ApplyUpdatedViewsFlag(SmartMetViewId::AllMapViews | SmartMetViewId::TimeSerialView);
	UpdateAllViewsAndDialogs("Control point mode changed");
}

void CSmartMetDoc::OnUpdateButtonEditorControlPointMode(CCmdUI* pCmdUI)
{
	if(itsData->SmartMetEditingMode() == CtrlViewUtils::kFmiEditingModeNormal) // eli jos ollaan edit-moodissa, asetetaan nappulan tilaa
	{
		pCmdUI->SetCheck(itsData->MetEditorOptionsData().ControlPointMode());
	}
	else	// jos ollaan ns. view-moodissa, disabloidaan koko nappula
		pCmdUI->Enable(FALSE);

}

void CSmartMetDoc::OnButtonDelete()
{
	if(itsData->MetEditorOptionsData().ControlPointMode())
	{
		itsData->CPManager()->RemoveCP();
		itsData->GetCombinedMapHandler()->mapViewDirty(itsMapViewDescTopIndex, false, false, true, false, false, false);
		itsData->LogMessage("Deleting control point.", CatLog::Severity::Debug, CatLog::Category::Editing);
        ApplyUpdatedViewsFlag(SmartMetViewId::AllMapViews | SmartMetViewId::TimeSerialView);
        UpdateAllViewsAndDialogs("Deleting control point");
	}
}

void CSmartMetDoc::StoreBitmapImageToFile(const std::string &callingFunctionName, const std::string &imageViewName, CBitmap *bitmap, const NFmiRect *theRelativeOutputArea)
{
    BOOL status = SaveViewToFilePicture(callingFunctionName, bitmap, theRelativeOutputArea);
    if(status)
		itsData->LogMessage(std::string("Stored ") + imageViewName + " to image file.", CatLog::Severity::Info, CatLog::Category::Operational);
    else
		itsData->LogMessage(std::string("Storing ") + imageViewName + " to image file failed.", CatLog::Severity::Error, CatLog::Category::Operational);
}

void CSmartMetDoc::OnDataStoreViewToPictureFile()
{
    StoreBitmapImageToFile(__FUNCTION__, "map view", ApplicationInterface::GetSmartMetView()->FinalMapViewImageBitmap(), &itsData->GetCombinedMapHandler()->getMapViewDescTop(itsMapViewDescTopIndex)->RelativeMapRect());
}

void CSmartMetDoc::OnUpdateDataStoreViewToPictureFile(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(TRUE);
}

void CSmartMetDoc::OnDataStoreExtraMapViewToPictureFile(CFmiExtraMapViewDlg *theExtraMapViewDlg)
{
	if(theExtraMapViewDlg)
	{
        StoreBitmapImageToFile(__FUNCTION__, "help map view", theExtraMapViewDlg->FinalMapViewImageBitmap(), &itsData->GetCombinedMapHandler()->getMapViewDescTop(theExtraMapViewDlg->MapViewDescTopIndex())->RelativeMapRect());
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
		// Kun 1. kerran avataan shortcuts dialogi, pit�� siin� oleva teksti
		// deselectoida, koska se on jostain mfc-jutun takia selectoitu.
		firstTime = false;
		itsShortCutsDialog->DeselectTexts();
	}
	itsData->LogMessage("Opening shortcuts dialog.", CatLog::Severity::Info, CatLog::Category::Operational);
}

void CSmartMetDoc::OnShowProjectionLines()
{
	itsData->OnShowProjectionLines();
}

void CSmartMetDoc::OnMenuitemProjectionLineSetup()
{
	NFmiProjectionCurvatureInfo* projInfo = itsData->GetCombinedMapHandler()->projectionCurvatureInfo();
	if(projInfo)
	{
		CFmiProjectionLineSetupDlg dlg(projInfo);
		if(dlg.DoModal() == IDOK)
		{
			projInfo->StoreToSettings();
			itsData->GetCombinedMapHandler()->mapViewDirty(itsMapViewDescTopIndex, false, true, true, false, false, false);
			ApplyUpdatedViewsFlag(SmartMetViewId::AllMapViews);
			UpdateAllViewsAndDialogs("Projection line drawing setup changed");
		}
	}
}

void CSmartMetDoc::OnButtonViewSelectParamDialog()
{
    ActivateParameterSelectionDlg();
}

void CSmartMetDoc::OnButtonBetaProductDialog()
{
    OnViewBetaproduction();
}

void CSmartMetDoc::OnDataStoreTimeserialviewToPictureFile()
{
	if(itsTimeSerialDataEditorDlg)
	{
        StoreBitmapImageToFile(__FUNCTION__, "time serial view", itsTimeSerialDataEditorDlg->ViewMemoryBitmap(), nullptr);
	}
}

void CSmartMetDoc::OnDataStoreTrajectoryViewToPictureFile()
{
	if(itsTrajectoryDlg)
	{
        StoreBitmapImageToFile(__FUNCTION__, "trajectory view", itsTrajectoryDlg->MemoryBitmap(), nullptr);
	}
}

NFmiApplicationWinRegistry& CSmartMetDoc::ApplicationWinRegistry()
{
    return itsData->ApplicationWinRegistry();
}

void CSmartMetDoc::CreateGriddingOptionsDialog(SmartMetDocumentInterface *smartMetDocumentInterface)
{
    if(!itsGriddingOptionsDlg)
    {
        CreateModalessDialog(&itsGriddingOptionsDlg, IDD_DIALOG_GRIDDING_OPTIONS, SmartMetDocumentInterface::GetSmartMetDocumentInterfaceImplementation(), false);
    }
}

void CSmartMetDoc::OnMenuitemGriddingOptions()
{
    CreateGriddingOptionsDialog(SmartMetDocumentInterface::GetSmartMetDocumentInterfaceImplementation());
    itsGriddingOptionsDlg->ShowWindow(SW_RESTORE);
    itsGriddingOptionsDlg->SetActiveWindow();
	itsData->LogMessage("Opening ToolMaster gridding options dialog", CatLog::Severity::Info, CatLog::Category::Editing);
}

void CSmartMetDoc::OnUpdateButtonFilterDialog(CCmdUI* pCmdUI)
{
	if(itsData->SmartMetEditingMode() != CtrlViewUtils::kFmiEditingModeNormal) // jos ollaan ns. view-moodissa, disabloidaan koko nappula
		pCmdUI->Enable(FALSE);
}

void CSmartMetDoc::OnButtonTempDlg()
{
	if(itsData)
	{
		if(!itsTempDialog)
			CreateTempDlg();
		if(itsData->GetMTATempSystem().TempViewOn())
		{ // jos luotaus dialogi oli p��ll�, laitetaan se pois p��lt�
			itsTempDialog->ShowWindow(SW_HIDE);
			itsData->LogMessage("Opening sounding view.", CatLog::Severity::Info, CatLog::Category::Operational);
		}
		else
		{ // muuten laitetaan se p��lle
			itsTempDialog->ShowWindow(SW_SHOW);	// Vaihdoin SW_RESTOREN, muistaa ikkunan muutetun koon.
			itsTempDialog->SetActiveWindow();
			itsData->LogMessage("Closing sounding view.", CatLog::Severity::Info, CatLog::Category::Operational);
		}
		itsData->GetMTATempSystem().TempViewOn(!itsData->GetMTATempSystem().TempViewOn());
		itsData->GetMTATempSystem().ShowMapMarkers(itsData->GetMTATempSystem().TempViewOn());
		// p�ivitet��n kartta ja muutkin n�yt�t, koska luotaus asemien kolmioiden kartta piirto riippuu t�st�
		itsData->GetCombinedMapHandler()->mapViewDirty(itsMapViewDescTopIndex, false, false, true, false, false, false); // laitetaan viela kaikki ajat likaisiksi cachesta
        ApplyUpdatedViewsFlag(SmartMetViewId::AllMapViews | SmartMetViewId::SoundingView);
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
		{ // poikkileikkaus n�ytt� oli p��ll�, laitetaan se pois p��lt�
			itsCrossSectionDlg->ShowWindow(SW_HIDE);
			itsData->CrossSectionSystem()->CrossSectionSystemActive(false);
			itsData->LogMessage("Opening cross section view.", CatLog::Severity::Info, CatLog::Category::Operational);
		}
		else
		{ // laitetaan poikkileikkuas ikkuna p��lle
			itsCrossSectionDlg->ShowWindow(SW_SHOW);	// Vaihdoin SW_RESTOREN, muistaa ikkunan muutetun koon.
			itsData->CrossSectionSystem()->CrossSectionSystemActive(true);
			itsCrossSectionDlg->SetActiveWindow();
			itsData->LogMessage("Closing cross section view.", CatLog::Severity::Info, CatLog::Category::Operational);
		}
		itsData->CrossSectionSystem()->CrossSectionViewOn(!itsData->CrossSectionSystem()->CrossSectionViewOn());
		itsData->GetCombinedMapHandler()->mapViewDirty(itsMapViewDescTopIndex, false, false, true, false, false, false);
        ApplyUpdatedViewsFlag(SmartMetViewId::AllMapViews | SmartMetViewId::CrossSectionView);
        ApplicationInterface::GetApplicationInterfaceImplementation()->RefreshApplicationViewsAndDialogs("CSmartMetDoc: Opening cross section view", TRUE);
	}
}

void CSmartMetDoc::OnUpdateButtonShowCrossSection(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(itsData->CrossSectionSystem()->CrossSectionViewOn());
}

void CSmartMetDoc::OnButtonObservationComparisonMode()
{
	itsData->ObsComparisonInfo().NextComparisonMode();
	itsData->GetCombinedMapHandler()->mapViewDirty(itsMapViewDescTopIndex, false, true, true, false, false, false);
	itsData->LogMessage("Toggling observation comparison mode.", CatLog::Severity::Info, CatLog::Category::Operational);
    ApplyUpdatedViewsFlag(SmartMetViewId::AllMapViews);
    UpdateAllViewsAndDialogs("Changed Obs comparison mode");
}

void CSmartMetDoc::OnUpdateButtonObservationComparisonMode(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(itsData->ObsComparisonInfo().ComparisonMode() > 0);
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
			itsData->LogMessage("Opening synop data table view.", CatLog::Severity::Info, CatLog::Category::Operational);
		}
		else
		{ // jos dialogi oli p��ll�, laitetaan se pois
			itsSynopDataGridViewDlg->ShowWindow(SW_HIDE);	// Vaihdoin SW_RESTOREN, muistaa ikkunan muutetun koon.
			itsData->LogMessage("Closing synop data table view.", CatLog::Severity::Info, CatLog::Category::Operational);
		}
		itsData->SynopDataGridViewOn(!itsData->SynopDataGridViewOn());
		if(itsData->SynopDataGridViewOn())
			itsSynopDataGridViewDlg->Update();

	}
}

void CSmartMetDoc::OnUpdateSynopDataGridView(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(itsData->SynopDataGridViewOn());
}

void CSmartMetDoc::OnAcceleratorObsComparisonChangeSymbol()
{
	itsData->ObsComparisonInfo().NextSymbolType();
	itsData->GetCombinedMapHandler()->mapViewDirty(itsMapViewDescTopIndex, false, true, true, false, false, false);
    ApplyUpdatedViewsFlag(SmartMetViewId::AllMapViews);
    UpdateAllViewsAndDialogs("Changed Obs comparison mode symbol");
}

void CSmartMetDoc::OnAcceleratorObsComparisonChangeSymbolSize()
{
	itsData->ObsComparisonInfo().NextSymbolSize();
	itsData->GetCombinedMapHandler()->mapViewDirty(itsMapViewDescTopIndex, false, true, true, false, false, false);
    ApplyUpdatedViewsFlag(SmartMetViewId::AllMapViews);
    UpdateAllViewsAndDialogs("Changed Obs comparison symbol size");
}

void CSmartMetDoc::OnAcceleratorObsComparisonToggleBorderDraw()
{
	itsData->ObsComparisonInfo().DrawBorders(!itsData->ObsComparisonInfo().DrawBorders());
	itsData->GetCombinedMapHandler()->mapViewDirty(itsMapViewDescTopIndex, false, true, true, false, false, false);
    ApplyUpdatedViewsFlag(SmartMetViewId::AllMapViews);
    UpdateAllViewsAndDialogs("Changed Obs comparison symbol border draw mode");
}

void CSmartMetDoc::OnButtonShowSynopPlotSettings()
{
	if(!itsSynopPlotSettingsDlg)
		CreateSynopPlotSettingsDlg(itsData);
	itsSynopPlotSettingsDlg->Update();
	itsSynopPlotSettingsDlg->ShowWindow(SW_SHOW);	// Vaihdoin SW_RESTOREN, muistaa ikkunan muutetun koon.
	itsSynopPlotSettingsDlg->SetActiveWindow();

	itsData->LogMessage("Opening synop plot settings dialog.", CatLog::Severity::Info, CatLog::Category::Operational);
}

void CSmartMetDoc::OnDataStoreTempviewToPictureFile()
{
	if(itsTempDialog)
	{
        StoreBitmapImageToFile(__FUNCTION__, "sounding view", itsTempDialog->ViewMemoryBitmap(), nullptr);
	}
}

void CSmartMetDoc::OnDataStoreCrosssectionviewToPictureFile()
{
	if(itsCrossSectionDlg)
	{
        StoreBitmapImageToFile(__FUNCTION__, "cross section view", itsCrossSectionDlg->MemoryBitmap(), nullptr);
	}
}

void CSmartMetDoc::OnToggleLandBorderDrawColor()
{
	itsData->GetCombinedMapHandler()->onToggleLandBorderDrawColor(itsMapViewDescTopIndex);
}

void CSmartMetDoc::OnToggleLandBorderPenSize()
{
	itsData->GetCombinedMapHandler()->onToggleLandBorderPenSize(itsMapViewDescTopIndex);
}

void CSmartMetDoc::OnAcceleratorBorrowParams1()
{
	itsData->GetCombinedMapHandler()->onAcceleratorBorrowParams(itsMapViewDescTopIndex, 1);
}
void CSmartMetDoc::OnAcceleratorBorrowParams2()
{
	itsData->GetCombinedMapHandler()->onAcceleratorBorrowParams(itsMapViewDescTopIndex, 2);
}
void CSmartMetDoc::OnAcceleratorBorrowParams3()
{
	itsData->GetCombinedMapHandler()->onAcceleratorBorrowParams(itsMapViewDescTopIndex, 3);
}
void CSmartMetDoc::OnAcceleratorBorrowParams4()
{
	itsData->GetCombinedMapHandler()->onAcceleratorBorrowParams(itsMapViewDescTopIndex, 4);
}
void CSmartMetDoc::OnAcceleratorBorrowParams5()
{
	itsData->GetCombinedMapHandler()->onAcceleratorBorrowParams(itsMapViewDescTopIndex, 5);
}
void CSmartMetDoc::OnAcceleratorBorrowParams6()
{
	itsData->GetCombinedMapHandler()->onAcceleratorBorrowParams(itsMapViewDescTopIndex, 6);
}
void CSmartMetDoc::OnAcceleratorBorrowParams7()
{
	itsData->GetCombinedMapHandler()->onAcceleratorBorrowParams(itsMapViewDescTopIndex, 7);
}
void CSmartMetDoc::OnAcceleratorBorrowParams8()
{
	itsData->GetCombinedMapHandler()->onAcceleratorBorrowParams(itsMapViewDescTopIndex, 8);
}
void CSmartMetDoc::OnAcceleratorBorrowParams9()
{
	itsData->GetCombinedMapHandler()->onAcceleratorBorrowParams(itsMapViewDescTopIndex, 9);
}
void CSmartMetDoc::OnAcceleratorBorrowParams10()
{
	itsData->GetCombinedMapHandler()->onAcceleratorBorrowParams(itsMapViewDescTopIndex, 10);
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
    // ** h�lytyskeskusn�yt�n sijainti **
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
    // ** trajektorin�yt�n sijainti **
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
    // ** luotausn�yt�n sijainti **
    auto createViewFunction = [this]() {this->CreateTempDlg(); };
    if(::GenerateViewIfNeededAndContinueToInitializations(theViewMacro.GetTempView(), &itsTempDialog, createViewFunction))
    {
        CFmiWin32Helpers::SetWindowSettings(itsTempDialog, ::NFmiRect2CRect(theViewMacro.GetTempView().AbsolutRect()), theViewMacro.GetTempView().ViewStatus(), theStartCornerCounter);
        itsTempDialog->Update();
    }
}

void CSmartMetDoc::LoadViewMacroSettingsForCrossSectionDlg(NFmiViewSettingMacro &theViewMacro, int &theStartCornerCounter)
{
    // ** poikkileikkausn�yt�n sijainti **
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
    // ** aikasarjan�yt�n sijainti **
    auto createViewFunction = [this]() {this->CreateTimeEditor(true); };
    if(::GenerateViewIfNeededAndContinueToInitializations(theViewMacro.GetTimeView(), &itsTimeSerialDataEditorDlg, createViewFunction))
    {
        CFmiWin32Helpers::SetWindowSettings(itsTimeSerialDataEditorDlg, ::NFmiRect2CRect(theViewMacro.GetTimeView().AbsolutRect()), theViewMacro.GetTimeView().ViewStatus(), theStartCornerCounter);
		itsData->TimeSerialDataViewOn(theViewMacro.GetTimeView().ViewStatus().ShowWindow());
        itsTimeSerialDataEditorDlg->fUseMaskInTimeSerialViews = theViewMacro.GetMaskSettings().UseMasksInTimeSerialViews();
        itsTimeSerialDataEditorDlg->UpdateData(FALSE);
    }
}

void CSmartMetDoc::LoadViewMacroWindowsSettings(NFmiViewSettingMacro& theViewMacro)
{
	if(itsData->MetEditorOptionsData().DisableWindowManipulations())
		return;
	try
	{
		if(theViewMacro.ViewMacroWasCorrupted())
			return; // ei tehd� mit��n, jos korruptoitunut viewMacro
		int startCornerCounter = 0;
		// ** kartta eli mainframen sijainti **
		NFmiViewSettingMacro::MapViewDescTop& mapViewDescTop0 = theViewMacro.ExtraMapViewDescTops()[0];
		CFmiWin32Helpers::SetWindowSettings(AfxGetMainWnd(), ::NFmiRect2CRect(mapViewDescTop0.AbsolutRect()), mapViewDescTop0.ViewStatus(), startCornerCounter);

		LoadViewMacroSettingsForTimeSerialDataView(theViewMacro, startCornerCounter);
		LoadViewMacroSettingsForTempDialog(theViewMacro, startCornerCounter);

		if(theViewMacro.OriginalLoadVersionNumber() > 1.)
		{ // tietyt asetukset tehd��n vain jos originaali macro-versio on tarpeeksi suuri
			LoadViewMacroSettingsForTrajectoryDlg(theViewMacro, startCornerCounter);
			LoadViewMacroSettingsForCrossSectionDlg(theViewMacro, startCornerCounter);

			// ** synop-plot asetukset **
			if(!itsSynopPlotSettingsDlg)
				CreateSynopPlotSettingsDlg(itsData);
			itsSynopPlotSettingsDlg->InitFromDoc();

			// ** obs-comparison asetukset **
			// ei obscomparison n�ytt� asetuksia

			LoadViewMacroSettingsForWarningCenterView(theViewMacro, startCornerCounter);
			LoadViewMacroSettingsForSynopDataGridView(theViewMacro, startCornerCounter);
			LoadViewMacroSettingsForExtraMapViewDlg(&itsExtraMapViewDlg1, 1, theViewMacro, startCornerCounter);
			LoadViewMacroSettingsForExtraMapViewDlg(&itsExtraMapViewDlg2, 2, theViewMacro, startCornerCounter);
		}
		MakeViewActivationAfterLoadingViewMacro();
	}
	catch(exception& e)
	{
		std::string errStr("Unable to load current view-macro, reason: \n");
		errStr += e.what();
		// pit�isik� raportoida message boxin avulla?
		itsData->LogAndWarnUser(errStr, "ViewMacro loading problem", CatLog::Severity::Error, CatLog::Category::Macro, false);
	}
}

void CSmartMetDoc::MakeViewActivationAfterLoadingViewMacro()
{
    // Lopuksi aktivoidaan viewMacro-dialogi, koska se on j��nyt luultavasti kaikkien ikkunoiden alle,
    // tai jos sit� ei ole edes luotu viel� (kun ladataan esim. backup-macro viime ajosta) niin p��ikkuna (kuten ennen tehtiin)
    if(itsViewMacroDlg)
        itsViewMacroDlg->SetActiveWindow();
    else
        AfxGetMainWnd()->SetActiveWindow();
}

// CTRL+I avaa nyky��n vain n�ytt�makro dialogin, ei muuta, t�ss� vanha harhaan johtava nimi
void CSmartMetDoc::OnAcceleratorStoreViewMacro()
{
	if(!itsViewMacroDlg)
		CreateViewMacroDlg(itsData); // pakko luoda t�ss�, koska muuten dialogin hndl j�� nollaksi, nyt sitten j�� leijumaan p��n�yt�n p��lle
	itsViewMacroDlg->ShowWindow(SW_SHOW);	// Vaihdoin SW_RESTOREN, muistaa ikkunan muutetun koon.
	itsViewMacroDlg->SetActiveWindow();
	itsData->LogMessage("Opening view macro dialog.", CatLog::Severity::Info, CatLog::Category::Operational);
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
// Aiemmin emme v�litt�neet min/max tiloista, mutta nyt niin halutaan. En halua kuitenkaan k�ytt�� muita tiloja,
// koska en tied� sivuvaikutuksia (erilaisia tiloja on n. 10 kpl). T�m� mahdollistaa sen, ett� jos showCommand on 0, 
// toimitaan kuten ennenkin (se arvo on nyky��n jokaisessa n�ytt�makrossa), muuten palautetaan showCommand arvo.
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

// t�m� on hankala, t�t� kutsutaan viewwmakro-dialogista zeditmapview-otuksen kautta
void CSmartMetDoc::StoreViewMacroWindowsSettings(NFmiViewSettingMacro &theViewMacro)
{
	NFmiRect windowRect;
	bool showWindow = false;

	std::vector<NFmiViewSettingMacro::MapViewDescTop>& extraMapViewDescTops = theViewMacro.ExtraMapViewDescTops();
	extraMapViewDescTops.resize(itsData->GetCombinedMapHandler()->getMapViewDescTops().size());
    auto &applicationWinRegistry = itsData->ApplicationWinRegistry();

	// ** p��kartta eli mainframen sijainti **
    StoreViewRectToMacro(dynamic_cast<CMainFrame*>(AfxGetMainWnd()), extraMapViewDescTops[0], applicationWinRegistry, 0);

	// ** aikasarjan�yt�n sijainti **
    StoreViewRectToMacro(itsTimeSerialDataEditorDlg, theViewMacro.GetTimeView(), applicationWinRegistry, 0);
	// ** luotausn�yt�n sijainti **
    StoreViewRectToMacro(itsTempDialog, theViewMacro.GetTempView(), applicationWinRegistry, 0);
	// ** trajektorin�yt�n sijainti **
    if(itsTrajectoryDlg)
    	itsTrajectoryDlg->UpdateTrajectorySystem(); // n�in otetaan trajektori-dialogissa tehdyt muutokset mukaan
    StoreViewRectToMacro(itsTrajectoryDlg, theViewMacro.GetTrajectoryView(), applicationWinRegistry, 0);
	// ** poikkileikkausn�yt�n sijainti **
    StoreViewRectToMacro(itsCrossSectionDlg, theViewMacro.GetCrossSectionView(), applicationWinRegistry, 0);

	// ** synop-plot asetukset **
	// ei ole mit��n n�ytt� asetuksia

	// ** obs-comparison asetukset **
	// ei ole mit��n n�ytt� asetuksia

	// ** h�lytyskeskusn�yt�n sijainti **
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
	if(itsData->SmartMetEditingMode() != CtrlViewUtils::kFmiEditingModeNormal) // jos ollaan ns. view-moodissa, disabloidaan koko nappula
		pCmdUI->Enable(FALSE);
}

void CSmartMetDoc::OnUpdateButtonStoreData(CCmdUI *pCmdUI)
{
	if(itsData->SmartMetEditingMode() != CtrlViewUtils::kFmiEditingModeNormal) // jos ollaan ns. view-moodissa, disabloidaan koko nappula
		pCmdUI->Enable(FALSE);
}

void CSmartMetDoc::OnUpdateButtonSelectionToolDlg(CCmdUI *pCmdUI)
{
	if(itsData->SmartMetEditingMode() != CtrlViewUtils::kFmiEditingModeNormal) // jos ollaan ns. view-moodissa, disabloidaan koko nappula
		pCmdUI->Enable(FALSE);
}

void CSmartMetDoc::OnUpdateSelectAll(CCmdUI *pCmdUI)
{
	if(itsData->SmartMetEditingMode() != CtrlViewUtils::kFmiEditingModeNormal) // jos ollaan ns. view-moodissa, disabloidaan koko nappula
		pCmdUI->Enable(FALSE);
}

void CSmartMetDoc::OnUpdateMenuitemSaveAs(CCmdUI *pCmdUI)
{
	if(itsData->SmartMetEditingMode() != CtrlViewUtils::kFmiEditingModeNormal) // jos ollaan ns. view-moodissa, disabloidaan koko nappula
		pCmdUI->Enable(FALSE);
}

void CSmartMetDoc::OnUpdateMenuitemGriddingOptions(CCmdUI *pCmdUI)
{
	if(itsData->SmartMetEditingMode() != CtrlViewUtils::kFmiEditingModeNormal) // jos ollaan ns. view-moodissa, disabloidaan koko nappula
		pCmdUI->Enable(FALSE);
}

void CSmartMetDoc::OnUpdateDeselectAll(CCmdUI *pCmdUI)
{
	if(itsData->SmartMetEditingMode() != CtrlViewUtils::kFmiEditingModeNormal) // jos ollaan ns. view-moodissa, disabloidaan koko nappula
		pCmdUI->Enable(FALSE);
}

void CSmartMetDoc::OnUpdateButtonMakeEditedDataCopy(CCmdUI *pCmdUI)
{
	if(itsData->SmartMetEditingMode() != CtrlViewUtils::kFmiEditingModeNormal) // jos ollaan ns. view-moodissa, disabloidaan koko nappula
		pCmdUI->Enable(FALSE);
}

void CSmartMetDoc::OnUpdateMakeGridFile(CCmdUI *pCmdUI)
{
	if(itsData->SmartMetEditingMode() != CtrlViewUtils::kFmiEditingModeNormal) // jos ollaan ns. view-moodissa, disabloidaan koko nappula
		pCmdUI->Enable(FALSE);
}

void CSmartMetDoc::OnUpdateEditPaste(CCmdUI *pCmdUI)
{
	if(itsData->SmartMetEditingMode() != CtrlViewUtils::kFmiEditingModeNormal) // jos ollaan ns. view-moodissa, disabloidaan koko nappula
		pCmdUI->Enable(FALSE);
}

void CSmartMetDoc::OnUpdateEditCopy(CCmdUI *pCmdUI)
{
	if(itsData->SmartMetEditingMode() != CtrlViewUtils::kFmiEditingModeNormal) // jos ollaan ns. view-moodissa, disabloidaan koko nappula
		pCmdUI->Enable(FALSE);
}

void CSmartMetDoc::OnAcceleratorCrossSectionMode()
{
	itsData->LogMessage("Set cross section mode on map view.", CatLog::Severity::Info, CatLog::Category::Operational);
	itsData->CrossSectionSystem()->CrossSectionSystemActive(!itsData->CrossSectionSystem()->CrossSectionSystemActive());
	itsData->GetCombinedMapHandler()->mapViewDirty(itsMapViewDescTopIndex, false, false, true, false, false, false);
    ApplyUpdatedViewsFlag(SmartMetViewId::MainMapView);
    UpdateAllViewsAndDialogs("Set cross section mode on map view (F4)");
}

void CSmartMetDoc::OnAcceleratorMapRow1()
{
	itsData->GetCombinedMapHandler()->onAcceleratorMapRow(itsMapViewDescTopIndex, 1);
}

void CSmartMetDoc::OnAcceleratorMapRow2()
{
	itsData->GetCombinedMapHandler()->onAcceleratorMapRow(itsMapViewDescTopIndex, 2);
}

void CSmartMetDoc::OnAcceleratorMapRow3()
{
	itsData->GetCombinedMapHandler()->onAcceleratorMapRow(itsMapViewDescTopIndex, 3);
}

void CSmartMetDoc::OnAcceleratorMapRow4()
{
	itsData->GetCombinedMapHandler()->onAcceleratorMapRow(itsMapViewDescTopIndex, 4);
}

void CSmartMetDoc::OnAcceleratorMapRow5()
{
	itsData->GetCombinedMapHandler()->onAcceleratorMapRow(itsMapViewDescTopIndex, 5);
}

void CSmartMetDoc::OnAcceleratorMapRow6()
{
	itsData->GetCombinedMapHandler()->onAcceleratorMapRow(itsMapViewDescTopIndex, 6);
}

void CSmartMetDoc::OnAcceleratorMapRow7()
{
	itsData->GetCombinedMapHandler()->onAcceleratorMapRow(itsMapViewDescTopIndex, 7);
}

void CSmartMetDoc::OnAcceleratorMapRow8()
{
	itsData->GetCombinedMapHandler()->onAcceleratorMapRow(itsMapViewDescTopIndex, 8);
}

void CSmartMetDoc::OnAcceleratorMapRow9()
{
	itsData->GetCombinedMapHandler()->onAcceleratorMapRow(itsMapViewDescTopIndex, 9);
}

void CSmartMetDoc::OnAcceleratorMapRow10()
{
	itsData->GetCombinedMapHandler()->onAcceleratorMapRow(itsMapViewDescTopIndex, 10);
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

// Tehd��n map, jossa on kaikkien eri karttan�ytt�jen Win-rekisteri avaimet ja default-ikkuna rect stringin�
std::map<std::string, std::string> CSmartMetDoc::MakeMapWindowPosMap(void)
{
    std::map<std::string, std::string> windowPosMap;

    // P��karttan�ytt�
    MakeMakeWindowPosMapInsert<CMainFrame>(windowPosMap);

    // apukarttan�yt�ille pit�� antaa avaimeen tuleva indeksi, koska sit� ei voi kysy� suoraan luokan staattiselta funktiolta
    MakeMakeWindowPosMapInsert<CFmiExtraMapViewDlg>(windowPosMap, 2);
    MakeMakeWindowPosMapInsert<CFmiExtraMapViewDlg>(windowPosMap, 3);

    return windowPosMap;
}

// Tehd��n map, jossa on kaikkien eri ei-karttan�ytt�jen Win-rekisteri avaimet ja default-ikkuna rect stringin�.
// Ks. CSmartMetDoc::SaveViewPositionsToRegistry, jossa talletetaan oikeasti t��ll� m��ritellyt n�yt�t.
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
    MakeMakeWindowPosMapInsert<CFmiWindTableDlg>(windowPosMap);
    MakeMakeWindowPosMapInsert<CFmiDataQualityCheckerDialog>(windowPosMap);
    MakeMakeWindowPosMapInsert<CFmiCaseStudyDlg>(windowPosMap);
    MakeMakeWindowPosMapInsert<CFmiSmartToolDlg>(windowPosMap);
    MakeMakeWindowPosMapInsert<CFmiBetaProductTabControlDialog>(windowPosMap);
    MakeMakeWindowPosMapInsert<CFmiLogViewer>(windowPosMap);
    MakeMakeWindowPosMapInsert<CFmiSoundingDataServerConfigurationsDlg>(windowPosMap);
    MakeMakeWindowPosMapInsert<CFmiParameterSelectionDlg>(windowPosMap);
	MakeMakeWindowPosMapInsert<CFmiWarningMessageOptionsDlg>(windowPosMap);
	MakeMakeWindowPosMapInsert<CFmiVisualizationSettings>(windowPosMap);
	MakeMakeWindowPosMapInsert<CFmiMacroParamDataGeneratorDlg>(windowPosMap);

    return windowPosMap;
}

template<class TView>
static void SaveViewPositionToRegistry(TView *theView, NFmiApplicationWinRegistry &theApplicationWinRegistry, unsigned int theMapViewDescTopIndex)
{
    if(theView)
        Persist2::WriteWindowRectToWinRegistry(theApplicationWinRegistry, theView->MakeUsedWinRegistryKeyStr(theMapViewDescTopIndex), theView);
}

// ks. CSmartMetDoc::MakeOtherWindowPosMap, jossa tehd��n lista talletettavista n�yt�ist�.
void CSmartMetDoc::SaveViewPositionsToRegistry(void)
{
	// t�ss� pit�� tallettaa 'permanent' dialogien sijainnit rekisteriin.
	// piti laittaa t�nne, koska aikasarja-ikkuna ja jotkin dialogit toimivat
	// my�s CDOCin destruktorista, mutta jotkin dialogit eiv�t, joten laitan
	// talletukset t�h�n metodiin.
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
    ::SaveViewPositionToRegistry(itsWindTableDlg, applicationWinRegistry, dummyMapDescTopIndex);
    ::SaveViewPositionToRegistry(itsExtraMapViewDlg1, applicationWinRegistry, 2);
    ::SaveViewPositionToRegistry(itsExtraMapViewDlg2, applicationWinRegistry, 3);
    ::SaveViewPositionToRegistry(itsDataQualityCheckerDialog, applicationWinRegistry, dummyMapDescTopIndex);
    ::SaveViewPositionToRegistry(itsCaseStudyDlg, applicationWinRegistry, dummyMapDescTopIndex);
    ::SaveViewPositionToRegistry(itsSmartToolDlg, applicationWinRegistry, dummyMapDescTopIndex);
    ::SaveViewPositionToRegistry(itsBetaProductDialog, applicationWinRegistry, dummyMapDescTopIndex);
    ::SaveViewPositionToRegistry(itsLogViewer, applicationWinRegistry, dummyMapDescTopIndex);
	::SaveViewPositionToRegistry(itsSoundingDataServerConfigurationsDlg, applicationWinRegistry, dummyMapDescTopIndex);
    ::SaveViewPositionToRegistry(itsParameterSelectionDlg, applicationWinRegistry, dummyMapDescTopIndex);
	::SaveViewPositionToRegistry(itsVisualizationSettings, applicationWinRegistry, dummyMapDescTopIndex);
	::SaveViewPositionToRegistry(itsMacroParamDataGeneratorDlg, applicationWinRegistry, dummyMapDescTopIndex);

    // Talletetaan my�s tiettyj� GeneralDocissa olevia juttuja aika-ajoin WinRekisteriin
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
		itsData->LogAndWarnUser(problemStr, "Problems when saving settings.", CatLog::Severity::Error, CatLog::Category::Configuration, false);
	}
	catch(...)
	{
		std::string problemStr("Problems when trying to save the settings to the settings files:\n");
		itsData->LogAndWarnUser(problemStr, "Problems when saving settings.", CatLog::Severity::Error, CatLog::Category::Configuration, false);
	}

	CDocument::OnCloseDocument();
}

// T�m� on pikkuinen pikaviritys.
// T�t� kutsutaan CMainFrame::OnClose -metodissa ennen kuin kutsutaan 
// NFmiEditMapGeneralDataDoc::StoreSupplementaryData -metodia.
// Ideaali paikka muuten tehd� tietyt sulkemis jutut olisi OnCloseDocument-metodissa, mutta
// se kutsutaan vasta NFmiEditMapGeneralDataDoc::StoreSupplementaryData -metodin j�lkeen. 
// Enk� halua muuttaa en�� ohjelman rakenteita kovinkaan radikaalisti.
void CSmartMetDoc::DoOnClose(void)
{
	// Jos caseStudy-dialogissa on tehty perus infoihin muutoksia, eik� dialogia ole suljettu, 
	// pit�� arvot ottaa erikseen talteen dokumentin olioon ennen arvojen talletuksia tiedostoon.
	if(itsCaseStudyDlg)
	{
		itsCaseStudyDlg->GetBasicInfoFromDialog(); 
	}

	// Samoin macroParam data generator dialogille
	if(itsMacroParamDataGeneratorDlg)
	{
		itsMacroParamDataGeneratorDlg->StoreControlValuesToDocument();
	}

	// Samoin beta-tuote dialogille
	if(itsBetaProductDialog)
	{
		itsBetaProductDialog->StoreControlValuesToDocument();
	}
}

void CSmartMetDoc::OnOhjeVirhetesti()
{
	DoCrashTest();
}

void CSmartMetDoc::OnMenuitemMuokkaaKieli()
{
	itsData->SelectLanguage();
}

void CSmartMetDoc::OnButtonTrajectory()
{
	if(!itsTrajectoryDlg)
		CreateTrajectoryDlg(itsData);
	if(itsData->TrajectorySystem()->TrajectoryViewOn())
	{ // jos trajektori dialogi oli p��ll�, laitetaan se pois p��lt�
		itsTrajectoryDlg->ShowWindow(SW_HIDE);
		itsData->LogMessage("Closing trajectory dialog.", CatLog::Severity::Info, CatLog::Category::Operational);
	}
	else
	{ // muuten laitetaan se p��lle
		itsTrajectoryDlg->ShowWindow(SW_SHOW);	// Vaihdoin SW_RESTOREN, muistaa ikkunan muutetun koon.
		itsTrajectoryDlg->SetActiveWindow();
		itsData->LogMessage("Opening trajectory dialog.", CatLog::Severity::Info, CatLog::Category::Operational);
	}
	itsData->TrajectorySystem()->TrajectoryViewOn(!itsData->TrajectorySystem()->TrajectoryViewOn());
	// p�ivitet��n kartta ja muutkin n�yt�t, koska luotaus asemien kolmioiden kartta piirto riippuu t�st�
	itsData->GetCombinedMapHandler()->mapViewDirty(itsMapViewDescTopIndex, false, true, true, false, false, false); // laitetaan viela kaikki ajat likaisiksi cachesta
	ApplyUpdatedViewsFlag(SmartMetViewId::AllMapViews | SmartMetViewId::TrajectoryView);
	UpdateAllViewsAndDialogs("Opening/closing trajectory view");
}

void CSmartMetDoc::OnButtonDataQualityChecker()
{
	if(!itsDataQualityCheckerDialog)
		CreateDataQualityCheckerDialog(itsData);
	if(itsData->DataQualityChecker().ViewOn())
	{ // jos dialogi oli p��ll�, laitetaan se pois p��lt�
		itsDataQualityCheckerDialog->ShowWindow(SW_HIDE);
		itsData->LogMessage("DataQualityChecker Dialog off.", CatLog::Severity::Info, CatLog::Category::Operational);
	}
	else
	{ // muuten laitetaan se p��lle
		itsDataQualityCheckerDialog->ShowWindow(SW_SHOW);	// Vaihdoin SW_RESTOREN, muistaa ikkunan muutetun koon.
		itsDataQualityCheckerDialog->SetActiveWindow();
		itsData->LogMessage("DataQualityChecker Dialog on.", CatLog::Severity::Info, CatLog::Category::Operational);
	}
	itsData->DataQualityChecker().ViewOn(!itsData->DataQualityChecker().ViewOn());
}

void CSmartMetDoc::OnExtraMapView(unsigned int theMapViewDescTopIndex, CFmiExtraMapViewDlg** theExtraMapViewDlg)
{
	if(*theExtraMapViewDlg == 0)
		*theExtraMapViewDlg = CreateExtraMapViewDlg(itsData, theMapViewDescTopIndex);

	auto* mapViewDescTop = itsData->GetCombinedMapHandler()->getMapViewDescTop(theMapViewDescTopIndex);
	if(mapViewDescTop->DescTopOn())
	{ // jos dialogi oli p��ll�, laitetaan se pois p��lt�
		(*theExtraMapViewDlg)->ShowWindow(SW_HIDE);
		itsData->LogMessage(string("Put ExtraMapViewDlg-") + NFmiStringTools::Convert<unsigned int>(theMapViewDescTopIndex) + " off.", CatLog::Severity::Info, CatLog::Category::Operational);
	}
	else
	{ // muuten laitetaan se p��lle
		(*theExtraMapViewDlg)->ShowWindow(SW_SHOW);	// Vaihdoin SW_RESTOREN, muistaa ikkunan muutetun koon.
		(*theExtraMapViewDlg)->SetActiveWindow();
		itsData->LogMessage(string("Put ExtraMapViewDlg-") + NFmiStringTools::Convert<unsigned int>(theMapViewDescTopIndex) + " on.", CatLog::Severity::Info, CatLog::Category::Operational);
	}
	mapViewDescTop->DescTopOn(!mapViewDescTop->DescTopOn());
	// p�ivitet��n kartta ja muutkin n�yt�t, koska luotaus asemien kolmioiden kartta piirto riippuu t�st�
	itsData->GetCombinedMapHandler()->mapViewDirty(theMapViewDescTopIndex, true, true, true, false, false, true); // laitetaan viela kaikki ajat likaisiksi cachesta
	ApplyUpdatedViewsFlag(GetWantedMapViewIdFlag(theMapViewDescTopIndex));
	UpdateAllViewsAndDialogs("Opening/closing map view " + std::to_string(theMapViewDescTopIndex + 1));
}

void CSmartMetDoc::OnExtraMapView1(void)
{
	OnExtraMapView(1, &itsExtraMapViewDlg1);
}

void CSmartMetDoc::OnUpdateOnExtraMapView1(CCmdUI *pCmdUI)
{
	try
	{
	pCmdUI->SetCheck(itsData->GetCombinedMapHandler()->getMapViewDescTop(1)->DescTopOn());
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
	pCmdUI->SetCheck(itsData->GetCombinedMapHandler()->getMapViewDescTop(2)->DescTopOn());
	}
	catch(...)
	{
	}
}

void CSmartMetDoc::OnUpdateButtonTrajectory(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(itsData->TrajectorySystem()->TrajectoryViewOn());
}

void CSmartMetDoc::OnViewEditorLog()
{
	std::string commandStr("notepad ");
	commandStr += CatLog::currentLogFilePath();
	CFmiProcessHelpers::ExecuteCommandInSeparateProcess(commandStr);
}

void CSmartMetDoc::OnToggleOverMapBackForeGround()
{
	itsData->GetCombinedMapHandler()->onToggleOverMapBackForeGround(itsMapViewDescTopIndex);
}

void CSmartMetDoc::OnAcceleratorToggleTooltip()
{
	itsData->MetEditorOptionsData().ShowToolTipsOnMapView(!itsData->MetEditorOptionsData().ShowToolTipsOnMapView());
}

void CSmartMetDoc::OnAcceleratorToggleKeepMapRatio()
{
	itsData->GetCombinedMapHandler()->onAcceleratorToggleKeepMapRatio();
}

void CSmartMetDoc::DoCrashTest(void)
{
    CatLog::logMessage(std::string(__FUNCTION__) + ": user starts crash test that will crash SmartMet", CatLog::Severity::Warning, CatLog::Category::Operational, true);
	// t�m�n on vain tarkoitus testata editorin virhe raportointia
//	throw std::runtime_error("Can you intersept this!!!"); // poikkeuksien k�sittely n�emm� tuottaa ongelmia crashrpt:lle
	NFmiFastQueryInfo *dummy = 0;
	dummy->Area();
	int x = 5;
	int y = 0;
	int voivoi = x%y;

	NFmiDataMatrix<float> bs;
	bs[10][20] = 4.6f;
	bs.At(10, 20) = 4.6f;
}

void CSmartMetDoc::OnHelpExceptiontest()
{
    // Yleinen poikkeuksien k�sittely hoidetaan nyt applikaation Run metodissa, jossa niist� lokitetaan viesti�, mutta jatketaan.
    std::string exceptionMessage = "Test exception is thrown from ";
    exceptionMessage += __FUNCTION__;
    throw std::runtime_error(exceptionMessage); 
}

void CSmartMetDoc::OnMenuitemHelpEditorModeSettings()
{
	CFmiHelpEditorSettingsDlg dlg(itsData->HelpEditorSystem());
	if(dlg.DoModal() == IDOK)
	{
		itsData->HelpEditorSystem().StoreSettings(false);
		itsData->GetCombinedMapHandler()->mapViewDirty(itsMapViewDescTopIndex, false, false, true, false, false, false);
        ApplyUpdatedViewsFlag(GetWantedMapViewIdFlag(itsMapViewDescTopIndex));
        UpdateAllViewsAndDialogs("Help edit mode settings changed");
	}
}

void CSmartMetDoc::OnButtonHelpEditorMode()
{
	itsData->HelpEditorSystem().HelpEditor(!itsData->HelpEditorSystem().HelpEditor());
	itsData->GetCombinedMapHandler()->mapViewDirty(itsMapViewDescTopIndex, false, false, true, false, false, false);
    ApplyUpdatedViewsFlag(GetWantedMapViewIdFlag(itsMapViewDescTopIndex));
    UpdateAllViewsAndDialogs("Help edit mode changed");
}

void CSmartMetDoc::OnUpdateButtonHelpEditorMode(CCmdUI *pCmdUI)
{
	if(itsData->HelpEditorSystem().Use())
	{
		pCmdUI->Enable(TRUE);
		pCmdUI->SetCheck(itsData->HelpEditorSystem().HelpEditor());
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

void CSmartMetDoc::OnButtonWindTableDlg()
{
	if(!itsWindTableDlg)
		CreateWindTableDlg(itsData);
	if(itsData->WindTableSystem().ViewVisible())
	{ // jos dialogi oli p��ll�, laitetaan se pois p��lt�
		itsWindTableDlg->ShowWindow(SW_HIDE);
		itsData->LogMessage("Hide WindTable view.", CatLog::Severity::Info, CatLog::Category::Operational);
	}
	else
	{ // muuten laitetaan se p��lle
		itsWindTableDlg->ShowWindow(SW_SHOW);	// Vaihdoin SW_RESTOREN, muistaa ikkunan muutetun koon.
		itsWindTableDlg->SetActiveWindow();
		itsData->LogMessage("Show Wind table view.", CatLog::Severity::Info, CatLog::Category::Operational);
	}
	itsData->WindTableSystem().ViewVisible(!itsData->WindTableSystem().ViewVisible());
	// p�ivitet��n kartta ja muutkin n�yt�t, koska luotaus asemien kolmioiden kartta piirto riippuu t�st�
	itsData->GetCombinedMapHandler()->mapViewDirty(itsMapViewDescTopIndex, false, true, true, false, false, false); // laitetaan viela kaikki ajat likaisiksi cachesta
	ApplyUpdatedViewsFlag(SmartMetViewId::AllMapViews | SmartMetViewId::WindTableDlg);
	UpdateAllViewsAndDialogs("Opening/closing Wind table view");
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
	itsData->OnChangeMapType(itsMapViewDescTopIndex, false);
}

void CSmartMetDoc::OnToggleShowPreviousNamesOnMap()
{
	itsData->GetCombinedMapHandler()->onToggleShowNamesOnMap(itsMapViewDescTopIndex, false);
}

void CSmartMetDoc::OpenLocationFinderTool(CWnd *parentView)
{
    if(itsData->AutoComplete().Use())
    {
        if(itsLocationFinderDlg == 0)
            CreateLocationFinderDlg(itsData);
		itsData->AutoComplete().AutoCompleteDialogOn(true);
        itsLocationFinderDlg->ActivateDialog(parentView);
    }
}

void CSmartMetDoc::OnAcceleratorLocationFinderTool()
{
    OpenLocationFinderTool(ApplicationInterface::GetSmartMetView());
}

void CSmartMetDoc::SetMacroErrorText(const std::string &theErrorStr)
{
	if(itsSmartToolDlg)
		itsSmartToolDlg->SetMacroErrorText(theErrorStr);
}


void CSmartMetDoc::OnUpdateButtonDataQualityChecker(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(itsData->DataQualityChecker().ViewOn());
}

// F7 -pressed
void CSmartMetDoc::OnAcceleratorToggleHelpCursorOnMap()
{
	itsData->ShowMouseHelpCursorsOnMap(!itsData->ShowMouseHelpCursorsOnMap());
    ApplicationInterface::GetApplicationInterfaceImplementation()->ForceDrawOverBitmapThings(itsMapViewDescTopIndex, true, true);
}

// F9 -pressed
void CSmartMetDoc::OnButtonWarningCenterDlg()
{
#ifndef DISABLE_CPPRESTSDK

	if(!itsWarningCenterDlg)
		CreateWarningCenterDlg(itsData);
	if(itsData->WarningCenterSystem().getLegacyData().WarningCenterViewOn())
	{ // jos dialogi oli p��ll�, laitetaan se pois p��lt�
		itsWarningCenterDlg->ShowWindow(SW_HIDE);
		itsData->LogMessage("Closing Warning Center dialog.", CatLog::Severity::Info, CatLog::Category::Operational);
	}
	else
	{ // muuten laitetaan se p��lle
		itsWarningCenterDlg->ShowWindow(SW_SHOW);	// Vaihdoin SW_RESTOREN, muistaa ikkunan muutetun koon.
		itsWarningCenterDlg->SetActiveWindow();
		itsData->LogMessage("Opening Warning Center dialog.", CatLog::Severity::Info, CatLog::Category::Operational);
	}
	itsData->WarningCenterSystem().getLegacyData().WarningCenterViewOn(!itsData->WarningCenterSystem().getLegacyData().WarningCenterViewOn());
	// p�ivitet��n kartta ja muutkin n�yt�t, koska luotaus asemien kolmioiden kartta piirto riippuu t�st�
	itsData->GetCombinedMapHandler()->mapViewDirty(itsMapViewDescTopIndex, false, true, true, false, false, false); // laitetaan viela kaikki ajat likaisiksi cachesta
	ApplyUpdatedViewsFlag(SmartMetViewId::AllMapViews | SmartMetViewId::WarningCenterDlg);
	UpdateAllViewsAndDialogs("Opening/closing Warning center dialog");

#endif // DISABLE_CPPRESTSDK
}

void CSmartMetDoc::OnViewCaseStudyDialog()
{
	if(itsCaseStudyDlg == 0)
	{
		CreateCaseStudyDlg(itsData);
	}
	itsCaseStudyDlg->ShowWindow(SW_SHOW);	// Vaihdoin SW_RESTOREN, muistaa ikkunan muutetun koon.
	itsCaseStudyDlg->SetActiveWindow();

	itsData->LogMessage("Case Study Dialog on.", CatLog::Severity::Info, CatLog::Category::Operational);
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
    ::PutTextToClipboard(::MapAreaInfo(itsData->GetCombinedMapHandler()->getMapViewDescTop(itsMapViewDescTopIndex)));
}


void CSmartMetDoc::OnTooltipstatusCrosssectionview()
{
	itsData->CrossSectionSystem()->ShowTooltipOnCrossSectionView(!itsData->CrossSectionSystem()->ShowTooltipOnCrossSectionView());
}


void CSmartMetDoc::OnUpdateTooltipstatusCrosssectionview(CCmdUI *pCmdUI)
{
    pCmdUI->SetCheck(itsData->CrossSectionSystem()->ShowTooltipOnCrossSectionView());
}


void CSmartMetDoc::OnTooltipstatusMapviews()
{
	itsData->MetEditorOptionsData().ShowToolTipsOnMapView(!itsData->MetEditorOptionsData().ShowToolTipsOnMapView());
}


void CSmartMetDoc::OnUpdateTooltipstatusMapviews(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(itsData->MetEditorOptionsData().ShowToolTipsOnMapView());
}


void CSmartMetDoc::OnTooltipstatusSoundingview()
{
	itsData->ShowToolTipTempView(!itsData->ShowToolTipTempView());
}


void CSmartMetDoc::OnUpdateTooltipstatusSoundingview(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(itsData->ShowToolTipTempView());
}


void CSmartMetDoc::OnTooltipstatusTimeserialview()
{
	itsData->ShowToolTipTimeView(!itsData->ShowToolTipTimeView());
}


void CSmartMetDoc::OnUpdateTooltipstatusTimeserialview(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(itsData->ShowToolTipTimeView());
}

// HUOM! T�ss� k�ytet��n F12 pikan�pp�int� laukaisemaan t�m� toiminto.
// Kun Visual C++:ssa ajaa debug versiota ohjelmasta ja painaa F12:sta, t�ll�in
// ohjelman suoritus keskeytet��n ja tulee n�kyviin dialogi, joka ilmoittaa ett� on tehty 
// pakotettu breakpoint. Jos t�ll�in painaa continue -nappulaa, ohjelma jatkaa normaalia
// toimintaansa. Release versio ei ole moksiskaan kun F12 painaa.
// T�t� breakpoint toimintoa ei voi mitenk��n est��, se kuuluu VC++:n toimintaan.
void CSmartMetDoc::OnAcceleratorApplyBackupViewMacro()
{
	itsData->ApplyBackupViewMacro(true);
}


void CSmartMetDoc::OnAcceleratorApplyCrashBackupViewMacro()
{
	itsData->ApplyBackupViewMacro(false);
}

void CSmartMetDoc::OnReloaddataReloadfailedsatelliteimages()
{
	itsData->ReloadFailedSatelliteImages();
}


void CSmartMetDoc::OnAcceleratorDevelopedFunctionTesting()
{
    // T��ll� voi testata mit� tahansa kehitett�v�� funktionaalisuutta, mik� laukaistaan
    // testimieless� painamalla n�pp�imist�n Insert -nappulaa.

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
        itsData->CurrentTime(itsMapViewDescTopIndex, currentTime);
        itsData->MapView()->DoOffScreenDraw(mapScreenBitmap);
        
        CFmiGdiPlusHelpers::SaveMfcBitmapToFile(&mapScreenBitmap, imageFileName, &(itsData->MapViewDescTop(itsMapViewDescTopIndex)->RelativeMapRect()));
        currentTime.ChangeByMinutes(timeStepInMinutes);
    }

    mapScreenBitmap.DeleteObject();
    itsData->ChangeTime(0, kBackward, 1, itsMapViewDescTopIndex, timeLenghtInMinutes); // asetus takaisin alkuaikaan
    */
}


void CSmartMetDoc::OnViewBetaproduction()
{
	if(itsBetaProductDialog == 0)
		CreateBetaProductDialog(SmartMetDocumentInterface::GetSmartMetDocumentInterfaceImplementation());
	itsBetaProductDialog->ShowWindow(SW_SHOW);	// Vaihdoin SW_RESTOREN, muistaa ikkunan muutetun koon.
	itsBetaProductDialog->SetActiveWindow();
	itsBetaProductDialog->Update();

	itsData->LogMessage("Beta Production Dialog on.", CatLog::Severity::Info, CatLog::Category::Operational);
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

void CSmartMetDoc::DoOffScreenDraw(BetaProductViewIndex selectedViewRadioButtonIndex, CBitmap &theDrawedScreenBitmapOut)
{
    if(selectedViewRadioButtonIndex == BetaProductViewIndex::MainMapView)
        CFmiWin32TemplateHelpers::DoOffScreenDraw(ApplicationInterface::GetSmartMetView(), SmartMetDocumentInterface::GetSmartMetDocumentInterfaceImplementation(), theDrawedScreenBitmapOut);
    else if(selectedViewRadioButtonIndex == BetaProductViewIndex::MapView2)
        CFmiWin32TemplateHelpers::DoOffScreenDraw(itsExtraMapViewDlg1->MapView(), SmartMetDocumentInterface::GetSmartMetDocumentInterfaceImplementation(), theDrawedScreenBitmapOut);
    else if(selectedViewRadioButtonIndex == BetaProductViewIndex::MapView3)
        CFmiWin32TemplateHelpers::DoOffScreenDraw(itsExtraMapViewDlg2->MapView(), SmartMetDocumentInterface::GetSmartMetDocumentInterfaceImplementation(), theDrawedScreenBitmapOut);
    else if(selectedViewRadioButtonIndex == BetaProductViewIndex::TimeSerialView)
        CFmiWin32TemplateHelpers::DoOffScreenDrawForNonMapView(itsTimeSerialDataEditorDlg->GetTimeEditValuesView(), SmartMetDocumentInterface::GetSmartMetDocumentInterfaceImplementation(), theDrawedScreenBitmapOut);
    else if(selectedViewRadioButtonIndex == BetaProductViewIndex::SoundingView)
    {
        // Lets set all possible temp info times to the main map's time
        SmartMetDocumentInterface::GetSmartMetDocumentInterfaceImplementation()->GetMTATempSystem().SetAllTempTimes(SmartMetDocumentInterface::GetSmartMetDocumentInterfaceImplementation()->CurrentTime(0));
        CFmiWin32TemplateHelpers::DoOffScreenDrawForNonMapView(itsTempDialog->GetTempView(), SmartMetDocumentInterface::GetSmartMetDocumentInterfaceImplementation(), theDrawedScreenBitmapOut);
    }
    else if(selectedViewRadioButtonIndex == BetaProductViewIndex::CrossSectionView)
        CFmiWin32TemplateHelpers::DoOffScreenDrawForNonMapView(itsCrossSectionDlg->GetCrossSectionView(), SmartMetDocumentInterface::GetSmartMetDocumentInterfaceImplementation(), theDrawedScreenBitmapOut);
}

void CSmartMetDoc::UpdateViewForOffScreenDraw(BetaProductViewIndex selectedViewRadioButtonIndex)
{
    if(selectedViewRadioButtonIndex == BetaProductViewIndex::MainMapView)
        ApplicationInterface::GetSmartMetView()->Update();
    else if(selectedViewRadioButtonIndex == BetaProductViewIndex::MapView2)
        itsExtraMapViewDlg1->Update(true);
    else if(selectedViewRadioButtonIndex == BetaProductViewIndex::MapView3)
        itsExtraMapViewDlg2->Update(true);
    else if(selectedViewRadioButtonIndex == BetaProductViewIndex::TimeSerialView)
        CreateTimeEditor(true); // This creates dialog, if not created, and then updates it. 
    else if(selectedViewRadioButtonIndex == BetaProductViewIndex::SoundingView)
    {
        CreateTempDlg(); // This creates dialog, if not created
        itsTempDialog->Update();
    }
    else if(selectedViewRadioButtonIndex == BetaProductViewIndex::CrossSectionView)
        CreateCrossSectionDlg(true); // This creates dialog, if not created, and then updates it. 
}

void CSmartMetDoc::OnAcceleratorResetTimeFilterTimes()
{
	itsData->ResetTimeFilterTimes();
    ApplyUpdatedViewsFlag(SmartMetViewId::AllMapViews | SmartMetViewId::DataFilterToolDlg);
    UpdateAllViewsAndDialogs("Reset time filter times (data editing related)", true);
}


void CSmartMetDoc::OnAcceleratorLogViewer()
{
    if(!itsLogViewer)
        CreateLogViewer(itsData);

    itsLogViewer->ShowWindow(SW_SHOW);
    itsLogViewer->SetActiveWindow();
    itsLogViewer->Update();

	itsData->LogMessage("Log Viewer on.", CatLog::Severity::Info, CatLog::Category::Operational);
}

void CSmartMetDoc::OpenLogViewer()
{
	OnAcceleratorLogViewer();
}

void CSmartMetDoc::OnEditSoundingDataFromServerSettings()
{
    CreateSoundingDataServerConfigurationsDlg();

    itsSoundingDataServerConfigurationsDlg->ShowWindow(SW_SHOW);
    itsSoundingDataServerConfigurationsDlg->SetActiveWindow();
    itsSoundingDataServerConfigurationsDlg->Update();

	itsData->LogMessage("Sounding data server configurations Dialog on.", CatLog::Severity::Info, CatLog::Category::Operational);
}

static const std::string gCalculationPointStartStr = "calculationpoint = ";
static const std::string gMaxLatlonLengthStr = "-123.123,-12.123";

static std::string MakeLatlonStr(const NFmiLocation &theLocation)
{
    std::string str = NFmiValueString::GetStringWithMaxDecimalsSmartWay(theLocation.GetLatitude(), 3);
    str += ",";
    str += NFmiValueString::GetStringWithMaxDecimalsSmartWay(theLocation.GetLongitude(), 3);
    // Tehd��n kaikista stringeist� samanpituisia, ennen kuin lis�t��n aseman nimi lainausmerkeiss�
    // per��n, n�in listasta tulee siistimm�n n�k�ist� katsella.
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
        auto iter = mapViewDecsTop->DrawParamListVector()->Index(mapViewDecsTop->AbsoluteActiveViewRow());
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
                        std::vector<boost::shared_ptr<NFmiFastQueryInfo> > infoVector;
                        generalDataDoc->GetCombinedMapHandler()->makeDrawedInfoVectorForMapView(infoVector, activeDrawParam, zoomedArea);
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
    ::PutTextToClipboard(::CalculationPointInfo(itsData->GetCombinedMapHandler()->getMapViewDescTop(itsMapViewDescTopIndex), itsData));
}

static std::string ActiveRowDataFilePaths(NFmiMapViewDescTop *mapViewDecsTop, NFmiEditMapGeneralDataDoc *generalDataDoc)
{
    if(mapViewDecsTop)
    {
        auto iter = mapViewDecsTop->DrawParamListVector()->Index(mapViewDecsTop->AbsoluteActiveViewRow());
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
                        std::vector<boost::shared_ptr<NFmiFastQueryInfo> > infoVector;
                        generalDataDoc->GetCombinedMapHandler()->makeDrawedInfoVectorForMapView(infoVector, drawParam, zoomedArea);
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
    ::PutTextToClipboard(::ActiveRowDataFilePaths(itsData->GetCombinedMapHandler()->getMapViewDescTop(itsMapViewDescTopIndex), itsData));
}

const int gManyRowsCount = 10;

void CSmartMetDoc::OnAcceleratorMoveManyMapRowsUp()
{
    if(itsData->GetCombinedMapHandler()->scrollViewRow(itsMapViewDescTopIndex, -gManyRowsCount))
        UpdateAllViewsAndDialogs("Jumping through main map view rows");
}

void CSmartMetDoc::OnAcceleratorMoveManyMapRowsDown()
{
    if(itsData->GetCombinedMapHandler()->scrollViewRow(itsMapViewDescTopIndex, gManyRowsCount))
        UpdateAllViewsAndDialogs("Jumping through main map view rows");
}

void CSmartMetDoc::OnAcceleratorApplyStartupViewMacro()
{
	itsData->ApplyStartupViewMacro();
}


void CSmartMetDoc::OnAcceleratorToggleWmsMapMode()
{
	auto& combinedMapHandler = *itsData->GetCombinedMapHandler();
	combinedMapHandler.useCombinedMapMode(!combinedMapHandler.useCombinedMapMode());
    UpdateAllViewsAndDialogs("Toggle combined local and WMS map mode");
}


void CSmartMetDoc::OnAcceleratorCpSelectNext()
{
    HandleCpAccelerator(ControlPointAcceleratorActions::Next, "Main map view: Select next Control-point"s);
}


void CSmartMetDoc::OnAcceleratorCpSelectPrevious()
{
    HandleCpAccelerator(ControlPointAcceleratorActions::Previous, "Main map view: Select previous Control-point"s);
}


void CSmartMetDoc::OnAcceleratorCpSelectLeft()
{
    HandleCpAccelerator(ControlPointAcceleratorActions::Left, "Main map view: Select nearest left Control-point"s);
}


void CSmartMetDoc::OnAcceleratorCpSelectRight()
{
    HandleCpAccelerator(ControlPointAcceleratorActions::Right, "Main map view: Select nearest right Control-point"s);
}


void CSmartMetDoc::OnAcceleratorCpSelectUp()
{
    HandleCpAccelerator(ControlPointAcceleratorActions::Up, "Main map view: Select nearest upward Control-point"s);
}


void CSmartMetDoc::OnAcceleratorCpSelectDown()
{
    HandleCpAccelerator(ControlPointAcceleratorActions::Down, "Main map view: Select nearest downward Control-point"s);
}

void CSmartMetDoc::HandleCpAccelerator(ControlPointAcceleratorActions action, const std::string &updateMessage)
{
    if(itsData->MakeControlPointAcceleratorAction(action, updateMessage))
    {
    }
}

void CSmartMetDoc::OnAcceleratorDoVisualizationProfiling()
{
	itsData->StartProfiling();
}

void CSmartMetDoc::CreateVisualizationSettingsDlg()
{
	if(itsVisualizationSettings)
		return; // Ei luoda dialogia uudestaan

	CWnd* desktopWindow = ApplicationInterface::GetSmartMetView()->GetDesktopWindow();
	itsVisualizationSettings = new CFmiVisualizationSettings(desktopWindow);
	BOOL status = itsVisualizationSettings->Create(IDD_DIALOG_VISUALIZATION_SETTINGS, desktopWindow);
}

void CSmartMetDoc::OnEditVisualizationsettings()
{
	CreateVisualizationSettingsDlg();

	itsVisualizationSettings->ShowWindow(SW_SHOW);
	itsVisualizationSettings->SetActiveWindow();
	//itsVisualizationSettings->Update();

	itsData->LogMessage("Visualization Settings dialog on", CatLog::Severity::Info, CatLog::Category::Operational);
}

template<typename MapView>
void SetTooltipDelay(MapView *mapView, bool doRestoreAction, int delayInMilliSeconds)
{
	if(mapView)
	{
		CFmiWin32Helpers::SetTooltipDelay(*(mapView->ToolTipControl()), doRestoreAction, delayInMilliSeconds);
	}
}


void CSmartMetDoc::SetAllMapViewTooltipDelays(bool doRestoreAction, int delayInMilliSeconds)
{
	SetTooltipDelay(ApplicationInterface::GetSmartMetView(), doRestoreAction, delayInMilliSeconds);
	SetTooltipDelay(itsExtraMapViewDlg1->MapView(), doRestoreAction, delayInMilliSeconds);
	SetTooltipDelay(itsExtraMapViewDlg2->MapView(), doRestoreAction, delayInMilliSeconds);
}


void CSmartMetDoc::OnAcceleratorMoveTimeBoxLocation()
{
	itsData->GetCombinedMapHandler()->onMoveTimeBoxLocation(itsMapViewDescTopIndex);
}


void CSmartMetDoc::OnEditGenerateNewWmsSystem()
{
	itsData->GetCombinedMapHandler()->startWmsSupportRenewalProcess(true);
}


void CSmartMetDoc::OnUpdateEditGenerateNewWmsSystem(CCmdUI* pCmdUI)
{
#ifndef DISABLE_CPPRESTSDK
	if(itsData->GetCombinedMapHandler()->getWmsSupport()->getCapabilitiesHaveBeenRetrieved())
		pCmdUI->Enable(TRUE);
	else
		pCmdUI->Enable(FALSE);
#endif // DISABLE_CPPRESTSDK
}


void CSmartMetDoc::OnAcceleratorToggleVirtualTimeMode()
{
	std::string viewName = "Main-map-view";
	CFmiWin32TemplateHelpers::OnAcceleratorToggleVirtualTimeMode(itsData, viewName);
}


void CSmartMetDoc::OnEditVirtualTimeMode()
{
	OnAcceleratorToggleVirtualTimeMode();
}


void CSmartMetDoc::OnUpdateEditVirtualTimeMode(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(itsData->VirtualTimeUsed());
}


void CSmartMetDoc::OnViewMacroparamdatageneration()
{
	if(!itsMacroParamDataGeneratorDlg)
		CreateMacroParamDataGeneratorDlg();
	itsMacroParamDataGeneratorDlg->ShowWindow(SW_SHOW);
	itsMacroParamDataGeneratorDlg->SetActiveWindow();
	itsData->LogMessage("Opening MacroParam data generator dialog.", CatLog::Severity::Info, CatLog::Category::Operational);
}

void CSmartMetDoc::OpenMacroParamInSmarttoolDialog(const std::string& absoluteMacroParamPath)
{
	OnButtonOpenSmartToolDlg();
	if(itsSmartToolDlg)
	{
		itsSmartToolDlg->OpenMacroParamForEditing(absoluteMacroParamPath);
	}
}
