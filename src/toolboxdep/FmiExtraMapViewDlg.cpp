// CFmiExtraMapViewDlg.cpp : implementation file
//

#include "stdafx.h"
#include "FmiExtraMapViewDlg.h"
#include "FmiWin32TemplateHelpers.h"
#include "FmiExtraMapView.h"
#include "SmartMetDocumentInterface.h"
#include "NFmiMapViewDescTop.h"
#include "FmiViewGridSelectorDlg.h"
#include "NFmiFastQueryInfo.h"
#include "NFmiGdiPlusImageMapHandler.h"
#include "NFmiMetEditorOptionsData.h"
#include "NFmiDictionaryFunction.h"
#include "FmiMapViewSettingsDlg.h"
#include "FmiWin32Helpers.h"
#include "CloneBitmap.h"
#include "NFmiApplicationWinRegistry.h"
#include "NFmiStationViewHandler.h"
#include "CtrlViewFunctions.h"
#include "CtrlViewKeyboardFunctions.h"
#include "persist2.h"
#include "ApplicationInterface.h"
#include "CombinedMapHandlerInterface.h"

// CFmiExtraMapViewDlg dialog

const NFmiViewPosRegistryInfo CFmiExtraMapViewDlg::s_ViewPosRegistryInfo(CRect(230, 160, 700, 800), "\\MapView");

IMPLEMENT_DYNAMIC(CFmiExtraMapViewDlg, CDialogEx2)
CFmiExtraMapViewDlg::CFmiExtraMapViewDlg(SmartMetDocumentInterface *smartMetDocumentInterface, unsigned int theMapViewDescTopIndex, CWnd* pParent /*=NULL*/)
:CDialogEx2(CFmiExtraMapViewDlg::IDD, pParent)
,itsView(0)
,itsSmartMetDocumentInterface(smartMetDocumentInterface)
,itsMapViewDescTopIndex(theMapViewDescTopIndex)
,m_SizeWnd(CtrlViewUtils::MaxViewGridXSize, CtrlViewUtils::MaxViewGridYSize, CtrlViewUtils::MaxViewGridXSize, CtrlViewUtils::MaxViewGridYSize)
,m_hAccel(0)
{
}

CFmiExtraMapViewDlg::~CFmiExtraMapViewDlg()
{
}

void CFmiExtraMapViewDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx2::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CFmiExtraMapViewDlg, CDialogEx2)
	ON_WM_SIZE()
	ON_WM_ERASEBKGND()
ON_COMMAND(ID_BUTTON_SELECT_FINLAND_MAP_EXTRA_MAP, OnButtonSelectFinlandMap)
ON_UPDATE_COMMAND_UI(ID_BUTTON_SELECT_FINLAND_MAP_EXTRA_MAP, OnUpdateButtonSelectFinlandMap)
ON_COMMAND(ID_BUTTON_SELECT_EUROPE_MAP_EXTRA_MAP, OnButtonSelectEuropeMap)
ON_UPDATE_COMMAND_UI(ID_BUTTON_SELECT_EUROPE_MAP_EXTRA_MAP, OnUpdateButtonSelectEuropeMap)
ON_COMMAND(ID_BUTTON_SELECT_SCANDINAVIA_MAP_EXTRA_MAP, OnButtonSelectScandinaviaMap)
ON_UPDATE_COMMAND_UI(ID_BUTTON_SELECT_SCANDINAVIA_MAP_EXTRA_MAP, OnUpdateButtonSelectScandinaviaMap)
ON_COMMAND(ID_BUTTON_GLOBE_EXTRA_MAP, OnButtonGlobe)
ON_UPDATE_COMMAND_UI(ID_BUTTON_GLOBE_EXTRA_MAP, OnUpdateButtonGlobe)
ON_COMMAND(ID_MENUITEM_VIEW_GRID_SELECTION_DLG_EXTRA_MAP, OnMenuitemViewGridSelectionDlg)
ON_COMMAND(ID_BUTTON_DATA_AREA_EXTRA_MAP, OnButtonDataArea)
ON_COMMAND(ID_BUTTON_MAP_VIEW_SETTINGS_EXTRA_MAP, OnButtonMapViewSettings)
ON_WM_CLOSE()
ON_COMMAND(ID_ACCELERATOR_SHOW_GRID_POINTS_EXTRA_MAP, OnShowGridPoints)
ON_COMMAND(ID_ACCELERATOR_GRID_POINT_COLOR_EXTRA_MAP, OnToggleGridPointColor)
ON_COMMAND(ID_ACCELERATOR_GRID_POINT_SIZE_EXTRA_MAP, OnToggleGridPointSize)
ON_COMMAND(ID_ACCELERATOR_SPACE_OUT_TEMP_WINDS, OnEditSpaceOut)
ON_COMMAND(ID_CHANGE_PARAM_WINDOW_POSITION_FORWARD_EXTRA_MAP, OnChangeParamWindowPositionForward)
ON_COMMAND(ID_CHANGE_PARAM_WINDOW_POSITION_BACKWARD_EXTRA_MAP, OnChangeParamWindowPositionBackward)
ON_COMMAND(ID_ACCELERATOR_SHOW_TIME_STRING_EXTRA_MAP, OnShowTimeString)
ON_COMMAND(ID_ACCELERATOR_REFRESH_EXTRA_MAP, OnButtonRefresh)
ON_COMMAND(ID_ACCELERATOR_CHANGE_MAP_TYPE_EXTRA_MAP, OnChangeMapType)
ON_COMMAND(ID_ACCELERATOR_TOGGLE_SHOW_NAMES_ON_MAP_EXTRA_MAP, OnToggleShowNamesOnMap)
ON_COMMAND(ID_ACCELERATOR_SHOW_MASKS_ON_MAP_EXTRA_MAP, OnShowMasksOnMap)
ON_COMMAND(ID_ACCELERATOR_SHOW_PROJECTION_LINES_EXTRA_MAP, OnShowProjectionLines)
ON_COMMAND(ID_ACCELERATOR_TOGGLE_LAND_BORDER_DRAW_COLOR_EXTRA_MAP, OnToggleLandBorderDrawColor)
ON_COMMAND(ID_ACCELERATOR_TOGGLE_LAND_BORDER_PEN_SIZE_EXTRA_MAP, OnToggleLandBorderPenSize)
ON_COMMAND(ID_ACCELERATOR_BORROW_PARAMS_EXTRA_MAP_1, OnAcceleratorBorrowParams1)
ON_COMMAND(ID_ACCELERATOR_BORROW_PARAMS_EXTRA_MAP_2, OnAcceleratorBorrowParams2)
ON_COMMAND(ID_ACCELERATOR_BORROW_PARAMS_EXTRA_MAP_3, OnAcceleratorBorrowParams3)
ON_COMMAND(ID_ACCELERATOR_BORROW_PARAMS_EXTRA_MAP_4, OnAcceleratorBorrowParams4)
ON_COMMAND(ID_ACCELERATOR_BORROW_PARAMS_EXTRA_MAP_5, OnAcceleratorBorrowParams5)
ON_COMMAND(ID_ACCELERATOR_BORROW_PARAMS_EXTRA_MAP_6, OnAcceleratorBorrowParams6)
ON_COMMAND(ID_ACCELERATOR_BORROW_PARAMS_EXTRA_MAP_7, OnAcceleratorBorrowParams7)
ON_COMMAND(ID_ACCELERATOR_BORROW_PARAMS_EXTRA_MAP_8, OnAcceleratorBorrowParams8)
ON_COMMAND(ID_ACCELERATOR_BORROW_PARAMS_EXTRA_MAP_9, OnAcceleratorBorrowParams9)
ON_COMMAND(ID_ACCELERATOR_BORROW_PARAMS_EXTRA_MAP_10, OnAcceleratorBorrowParams10)
ON_COMMAND(ID_ACCELERATOR_EXTRA_MAP_ROW_1, OnAcceleratorMapRow1)
ON_COMMAND(ID_ACCELERATOR_EXTRA_MAP_ROW_2, OnAcceleratorMapRow2)
ON_COMMAND(ID_ACCELERATOR_EXTRA_MAP_ROW_3, OnAcceleratorMapRow3)
ON_COMMAND(ID_ACCELERATOR_EXTRA_MAP_ROW_4, OnAcceleratorMapRow4)
ON_COMMAND(ID_ACCELERATOR_EXTRA_MAP_ROW_5, OnAcceleratorMapRow5)
ON_COMMAND(ID_ACCELERATOR_EXTRA_MAP_ROW_6, OnAcceleratorMapRow6)
ON_COMMAND(ID_ACCELERATOR_EXTRA_MAP_ROW_7, OnAcceleratorMapRow7)
ON_COMMAND(ID_ACCELERATOR_EXTRA_MAP_ROW_8, OnAcceleratorMapRow8)
ON_COMMAND(ID_ACCELERATOR_EXTRA_MAP_ROW_9, OnAcceleratorMapRow9)
ON_COMMAND(ID_ACCELERATOR_EXTRA_MAP_ROW_10, OnAcceleratorMapRow10)
ON_COMMAND(ID_ACCELERATOR_TOGGLE_HELP_CURSOR_ON_EXTRA_MAP, OnAcceleratorToggleHelpCursorOnMap)
ON_COMMAND(ID_ACCELERATOR_TOGGLE_OVERMAP_FORE_BACK_GROUND_EXTRA_MAP, OnAcceleratorToggleOvermapForeBackGround)
ON_COMMAND(ID_ACCELERATOR_TOGGLE_TEMP_VIEW_TOOLTIP, OnAcceleratorToggleTooltip)
ON_COMMAND(ID_ACCELERATOR_TOGGLE_KEEP_EXTRA_MAP_RATIO, OnAcceleratorToggleKeepMapRatio)
ON_NOTIFY_EX_RANGE(TTN_NEEDTEXTW, 0, 0xFFFF, OnToolTipText)
ON_NOTIFY_EX_RANGE(TTN_NEEDTEXTA, 0, 0xFFFF, OnToolTipText)
ON_COMMAND(ID_BUTTON_VIEW_SELECT_PARAM_DIALOG_EXTRA_MAP, OnButtonSelectParamDialogExtraMap)
ON_COMMAND(ID_BUTTON_ZOOM_DIALOG_EXTRA_MAP, OnButtonZoomDialog)
ON_COMMAND(ID_TOGGLE_EXTRA_MAP_VIEW_DISPLAY_MODE, OnToggleMapViewDisplayMode)
ON_COMMAND(IDC_BUTTON_PRINT, OnButtonExtraMapPrint)
ON_COMMAND(ID_ACCELERATOR_LOCK_TIME_TO_MAIN_MAP, OnAcceleratorLockTimeToMainMap)
ON_COMMAND(ID_ACCELERATOR_MAKE_SWAP_BASE_AREA_EXTRA_MAP, OnAcceleratorMakeSwapBaseArea)
ON_COMMAND(ID_ACCELERATOR_SWAP_AREA_EXTRA_MAP, OnAcceleratorSwapArea)
ON_COMMAND(ID_ACCELERATOR_SWAP_AREA_SECONDARY_KEY_EXTRA_MAP, OnAcceleratorSwapAreaSecondaryKey)
ON_COMMAND(ID_EDIT_COPY, &CFmiExtraMapViewDlg::OnEditCopy)
ON_COMMAND(ID_EDIT_PASTE, &CFmiExtraMapViewDlg::OnEditPaste)
ON_COMMAND(ID_ACCELERATOR_COPY_ALL_EXTRA_MAP_VIEW_PARAMS, &CFmiExtraMapViewDlg::OnAcceleratorCopyAllMapViewParams)
ON_COMMAND(ID_ACCELERATOR_PASTE_ALL_EXTRA_MAP_VIEW_PARAMS, &CFmiExtraMapViewDlg::OnAcceleratorPasteAllMapViewParams)
ON_COMMAND(ID_ACCELERATOR_TOGGLE_ANIMATION_VIEW_EXTRA_MAP, &CFmiExtraMapViewDlg::OnAcceleratorToggleAnimationView)
ON_COMMAND(ID_ACCELERATOR_SET_HOME_TIME_EXTRA_MAP, &CFmiExtraMapViewDlg::OnAcceleratorSetHomeTime)
ON_COMMAND(ID_TOGGLE_SHOW_PREVIOUS_NAMES_ON_EXTRA_MAP, &CFmiExtraMapViewDlg::OnToggleShowPreviousNamesOnMap)
ON_COMMAND(ID_ACCELERATOR_TIME_VIEW_TOGGLE_HELP_DATA3, &CFmiExtraMapViewDlg::OnChangePreviousMapType)
ON_COMMAND(ID_BUTTON_ANIMATION_EXTRA_MAP, &CFmiExtraMapViewDlg::OnButtonAnimation)
ON_COMMAND(ID_ACCELERATOR_EXTRA_MAP_PAN_DOWN, &CFmiExtraMapViewDlg::OnAcceleratorExtraMapPanDown)
ON_COMMAND(ID_ACCELERATOR_EXTRA_MAP_PAN_LEFT, &CFmiExtraMapViewDlg::OnAcceleratorExtraMapPanLeft)
ON_COMMAND(ID_ACCELERATOR_EXTRA_MAP_PAN_RIGHT, &CFmiExtraMapViewDlg::OnAcceleratorExtraMapPanRight)
ON_COMMAND(ID_ACCELERATOR_EXTRA_MAP_PAN_UP, &CFmiExtraMapViewDlg::OnAcceleratorExtraMapPanUp)
ON_COMMAND(ID_ACCELERATOR_EXTRA_MAP_ZOOM_IN, &CFmiExtraMapViewDlg::OnAcceleratorExtraMapZoomIn)
ON_COMMAND(ID_ACCELERATOR_EXTRA_MAP_ZOOM_OUT, &CFmiExtraMapViewDlg::OnAcceleratorExtraMapZoomOut)
ON_WM_GETMINMAXINFO()
ON_COMMAND(ID_ACCELERATOR_EXTRA_MAP_MOVE_MANY_MAP_ROWS_UP, &CFmiExtraMapViewDlg::OnAcceleratorMoveManyMapRowsUp)
ON_COMMAND(ID_ACCELERATOR_EXTRA_MAP_MOVE_MANY_MAP_ROWS_DOWN, &CFmiExtraMapViewDlg::OnAcceleratorMoveManyMapRowsDown)
ON_COMMAND(ID_ACCELERATOR_LOCK_ROW_TO_MAIN_MAP, &CFmiExtraMapViewDlg::OnAcceleratorLockRowToMainMap)
ON_COMMAND(ID_ACCELERATOR_EXTRA_MAP_LOCATION_FINDER_TOOL, &CFmiExtraMapViewDlg::OnAcceleratorExtraMapLocationFinderTool)
ON_COMMAND(ID_ACCELERATOR_CHANGE_TIME_BY_STEP1_FORWARD, &CFmiExtraMapViewDlg::OnAcceleratorChangeTimeByStep1Forward)
ON_COMMAND(ID_ACCELERATOR_CHANGE_TIME_BY_STEP1_BACKWARD, &CFmiExtraMapViewDlg::OnAcceleratorChangeTimeByStep1Backward)
ON_COMMAND(ID_ACCELERATOR_CHANGE_TIME_BY_STEP2_FORWARD, &CFmiExtraMapViewDlg::OnAcceleratorChangeTimeByStep2Forward)
ON_COMMAND(ID_ACCELERATOR_CHANGE_TIME_BY_STEP2_BACKWARD, &CFmiExtraMapViewDlg::OnAcceleratorChangeTimeByStep2Backward)
ON_COMMAND(ID_ACCELERATOR_CHANGE_TIME_BY_STEP3_FORWARD, &CFmiExtraMapViewDlg::OnAcceleratorChangeTimeByStep3Forward)
ON_COMMAND(ID_ACCELERATOR_CHANGE_TIME_BY_STEP3_BACKWARD, &CFmiExtraMapViewDlg::OnAcceleratorChangeTimeByStep3Backward)
ON_COMMAND(ID_ACCELERATOR_CHANGE_TIME_BY_STEP4_FORWARD, &CFmiExtraMapViewDlg::OnAcceleratorChangeTimeByStep4Forward)
ON_COMMAND(ID_ACCELERATOR_CHANGE_TIME_BY_STEP4_BACKWARD, &CFmiExtraMapViewDlg::OnAcceleratorChangeTimeByStep4Backward)
ON_COMMAND(ID_EDIT_VISUALIZATIONSETTINGS_EXTRA_MAP, &CFmiExtraMapViewDlg::OnEditVisualizationsettingsExtraMap)
ON_COMMAND(ID_ACCELERATOR_EM_CHANGE_ALL_MODEL_DATA_ON_ROW_TO_PREVIOUS_MODEL_RUN, &CFmiExtraMapViewDlg::OnAcceleratorEmChangeAllModelDataOnRowToPreviousModelRun)
ON_COMMAND(ID_ACCELERATOR_EM_CHANGE_ALL_MODEL_DATA_ON_ROW_TO_NEXT_MODEL_RUN, &CFmiExtraMapViewDlg::OnAcceleratorEmChangeAllModelDataOnRowToNextModelRun)
ON_COMMAND(ID_ACCELERATOR_EXTRA_MAP_VIEW_RANGE_METER_MODE_TOGGLE, &CFmiExtraMapViewDlg::OnAcceleratorExtraMapViewRangeMeterModeToggle)
ON_COMMAND(ID_ACCELERATOR_EXTRA_MAP_VIEW_RANGE_METER_COLOR_TOGGLE, &CFmiExtraMapViewDlg::OnAcceleratorExtraMapViewRangeMeterColorToggle)
ON_COMMAND(ID_ACCELERATOR_LOG_VIEWER_TOOLBOXDEB, &CFmiExtraMapViewDlg::OnAcceleratorLogViewerToolboxdeb)
ON_COMMAND(ID_BUTTON_OPEN_MAIN_MAP_VIEW_EXTRA_MAP, &CFmiExtraMapViewDlg::OnButtonOpenMainMapViewExtraMap)
ON_COMMAND(ID_BUTTON_OPEN_OTHER_SIDE_MAP_VIEW_EXTRA_MAP, &CFmiExtraMapViewDlg::OnButtonOpenOtherSideMapViewExtraMap)
ON_COMMAND(ID_BUTTON_OPEN_TIME_SERIAL_VIEW_EXTRA_MAP, &CFmiExtraMapViewDlg::OnButtonOpenTimeSerialViewExtraMap)
ON_COMMAND(ID_BUTTON_OPEN_SMARTTOOLS_DIALOG_EXTRA_MAP, &CFmiExtraMapViewDlg::OnButtonOpenSmarttoolsDialogExtraMap)
ON_COMMAND(ID_BUTTON_OPEN_SOUNDING_VIEW_EXTRA_MAP, &CFmiExtraMapViewDlg::OnButtonOpenSoundingViewExtraMap)
ON_COMMAND(ID_BUTTON_OPEN_CROSSSECTION_VIEW_EXTRA_MAP, &CFmiExtraMapViewDlg::OnButtonOpenCrosssectionViewExtraMap)
ON_COMMAND(ID_BUTTON_OPEN_VIEW_MACRO_DIALOG_EXTRA_MAP, &CFmiExtraMapViewDlg::OnButtonOpenViewMacroDialogExtraMap)
ON_COMMAND(ID_ACCELERATOR_EXTRA_MAP_VIEW_RANGE_METER_LOCK_MODE_TOGGLE, &CFmiExtraMapViewDlg::OnAcceleratorExtraMapViewRangeMeterLockModeToggle)
ON_COMMAND(ID_ACCELERATOR_EXTRA_MAP_MOVE_TIME_BOX_LOCATION, &CFmiExtraMapViewDlg::OnAcceleratorExtraMapMoveTimeBoxLocation)
ON_COMMAND(ID_ACCELERATOR_EXTRA_MAP_OBS_COMPARISON_MODE, &CFmiExtraMapViewDlg::OnAcceleratorExtraMapObsComparisonMode)
ON_COMMAND(ID_ACCELERATOR_EXTRA_MAP_TOGGLE_VIRTUAL_TIME_MODE, &CFmiExtraMapViewDlg::OnAcceleratorExtraMapToggleVirtualTimeMode)
END_MESSAGE_MAP()


// CFmiExtraMapViewDlg message handlers

void CFmiExtraMapViewDlg::Update(bool fUpdateHiddenView)
{
	static int counter = 0;
	counter++;

    if((IsWindowVisible() && !IsIconic()) || fUpdateHiddenView) // N‰yttˆ‰ p‰ivitet‰‰n vain jos se on n‰kyviss‰ ja se ei ole minimized tilassa, paitsi jos piirret‰‰n ns. offscreen moodissa (fUpdateHiddenView=true), jota k‰ytet‰‰n Beta-tuotteiden teossa
	{
		itsView->UpdateMap();
		Invalidate(FALSE);
	}
}

void CFmiExtraMapViewDlg::SetDefaultValues(void)
{
    MoveWindow(CFmiExtraMapViewDlg::ViewPosRegistryInfo().DefaultWindowRect());
    Persist2::WriteWindowRectToWinRegistry(itsSmartMetDocumentInterface->ApplicationWinRegistry(), MakeUsedWinRegistryKeyStr(itsMapViewDescTopIndex+1), this);
}

std::string CFmiExtraMapViewDlg::MakeUsedWinRegistryKeyStr(unsigned int theMapViewDescTopIndex)
{
    std::string winRegistryKeyStr = CFmiExtraMapViewDlg::ViewPosRegistryInfo().WinRegistryKeyStr();
    winRegistryKeyStr += NFmiStringTools::Convert(theMapViewDescTopIndex);
    return winRegistryKeyStr;
}

BOOL CFmiExtraMapViewDlg::OnInitDialog()
{
	CDialogEx2::OnInitDialog();

	// Load accelerators
	m_hAccel = ::LoadAccelerators(AfxGetResourceHandle(), MAKEINTRESOURCE(IDR_ACCELERATOR1));
	ASSERT(m_hAccel);

	CRect rect(CalcClientArea());
	itsView = new CFmiExtraMapView(itsSmartMetDocumentInterface, itsMapViewDescTopIndex);
	if(itsView)
	{
		itsView->Create(NULL,NULL, WS_VISIBLE | WS_CHILD, rect, this, NULL);
		itsView->OnInitialUpdate(); // pit‰‰ kutsua erikseen, koska formvieta ei ole sidottu dokumenttiin
	}

	CFmiWin32Helpers::SetUsedWindowIconDynamically(this);

	UINT indicators[] = {
							ID_SEPARATOR,
							ID_INDICATOR_CAPS,
							ID_INDICATOR_NUM,
							ID_INDICATOR_SCRL,
						};

	InitDialogEx(TRUE, TRUE, indicators, 4, IDR_TOOLBAR_EXTRA_MAP_VIEW, TBSTYLE_FLAT | WS_CHILD | WS_VISIBLE | CBRS_TOP | CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC, IDB_BITMAP_EXTRAMAPVIEW_TOOLBAR_24BIT_16X16);

	std::string errorBaseStr("Error in CFmiExtraMapViewDlg::OnInitDialog while reading dialog size and position values");
    CFmiWin32TemplateHelpers::DoWindowSizeSettingsFromWinRegistry(itsSmartMetDocumentInterface->ApplicationWinRegistry(), this, false, errorBaseStr, itsMapViewDescTopIndex+1);
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CFmiExtraMapViewDlg::OnSize(UINT nType, int cx, int cy)
{
	static int counter = 0;
	counter++;

	CDialogEx2::OnSize(nType, cx, cy);

	if(itsView)
		itsView->MoveWindow(CalcClientArea(), FALSE);
	Invalidate(FALSE);
    if(ApplicationInterface::GetApplicationInterfaceImplementation)
    {
        // Vain t‰m‰ n‰yttˆ itse ja zoomaus dialogi pit‰‰ p‰ivitt‰‰
        SmartMetViewId thisMapViewId = (itsMapViewDescTopIndex == 1) ? SmartMetViewId::MapView2 : SmartMetViewId::MapView3;
        ApplicationInterface::GetApplicationInterfaceImplementation()->RefreshApplicationViewsAndDialogs("Help map view (2/3) was rezised", thisMapViewId | SmartMetViewId::ZoomDlg);
    }
}

CRect CFmiExtraMapViewDlg::CalcClientArea(void)
{
	CRect rect;
	GetClientRect(rect);
	int aTop = rect.top;
	int aBottom = rect.bottom;
	CToolBar *toolBar = GetToolBar();
	if(toolBar && toolBar->GetSafeHwnd())
	{
		CRect aRect;
		toolBar->GetWindowRect(aRect);
		aTop += aRect.Height();
	}
	CStatusBar *statusBar = GetStatusBar();
	if(statusBar && statusBar->GetSafeHwnd())
	{
		CRect aRect;
		toolBar->GetWindowRect(aRect);
		aBottom -= aRect.Height() - 6;
	}
	rect.top = aTop;
	rect.bottom = aBottom;
	return rect;
}

BOOL CFmiExtraMapViewDlg::OnEraseBkgnd(CDC* pDC)
{
	return FALSE;	// n‰in ei tyhjennet‰ taustaa ennen ruudun piirtoa!!

//	return CDialogEx2::OnEraseBkgnd(pDC);
}

void CFmiExtraMapViewDlg::MapAreaButtonSelect(unsigned int newMapIndex)
{
    std::string updateMessage = "Map view ";
    updateMessage += std::to_string(itsMapViewDescTopIndex + 1) + ": Map area ";
    updateMessage += std::to_string(newMapIndex + 1) + " selected";
    CatLog::logMessage(updateMessage, CatLog::Severity::Info, CatLog::Category::Visualization);
    itsSmartMetDocumentInterface->SetSelectedMapHandler(itsMapViewDescTopIndex, newMapIndex);
    itsSmartMetDocumentInterface->RefreshApplicationViewsAndDialogs(updateMessage);
}


void CFmiExtraMapViewDlg::OnButtonSelectFinlandMap()
{
    MapAreaButtonSelect(0);
}

bool CFmiExtraMapViewDlg::IsMapSelected(int theMapIndex)
{
    return itsSmartMetDocumentInterface->ApplicationWinRegistry().ConfigurationRelatedWinRegistry().MapView(itsMapViewDescTopIndex)->SelectedMapIndex() == theMapIndex;
}


void CFmiExtraMapViewDlg::OnUpdateButtonSelectFinlandMap(CCmdUI *pCmdUI)
{
    pCmdUI->SetCheck(IsMapSelected(0));
}

void CFmiExtraMapViewDlg::OnButtonSelectEuropeMap()
{
    MapAreaButtonSelect(2);
}

void CFmiExtraMapViewDlg::OnUpdateButtonSelectEuropeMap(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(IsMapSelected(2));
}

void CFmiExtraMapViewDlg::OnButtonSelectScandinaviaMap()
{
    MapAreaButtonSelect(1);
}

void CFmiExtraMapViewDlg::OnUpdateButtonSelectScandinaviaMap(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(IsMapSelected(1));
}

void CFmiExtraMapViewDlg::OnButtonGlobe()
{
    MapAreaButtonSelect(3);
}

void CFmiExtraMapViewDlg::OnUpdateButtonGlobe(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(IsMapSelected(3));
}

void CFmiExtraMapViewDlg::OnButtonDataArea()
{
	itsSmartMetDocumentInterface->OnButtonDataArea(itsMapViewDescTopIndex);
}

void CFmiExtraMapViewDlg::DoWhenClosing(void)
{
    itsSmartMetDocumentInterface->MapViewDescTop(itsMapViewDescTopIndex)->DescTopOn(false);
	AfxGetMainWnd()->SetActiveWindow(); // aktivoidaan karttan‰yttˆ eli mainframe
}

void CFmiExtraMapViewDlg::OnOK()
{
	DoWhenClosing();

	CDialog::OnOK(); // HUOM!! T‰ss‰ ei saa kutsua suoran emon CDialogEx2 metodia, koska se tuhoaa ikkunan, vaan CDialog:in metodia
}

void CFmiExtraMapViewDlg::OnCancel()
{
	DoWhenClosing();

	CDialog::OnCancel(); // HUOM!! T‰ss‰ ei saa kutsua suoran emon CDialogEx2 metodia, koska se tuhoaa ikkunan, vaan CDialog:in metodia
}

void CFmiExtraMapViewDlg::OnClose()
{
	DoWhenClosing();

	CDialog::OnClose(); // HUOM!! T‰ss‰ ei saa kutsua suoran emon CDialogEx2 metodia, koska se tuhoaa ikkunan, vaan CDialog:in metodia
}

void CFmiExtraMapViewDlg::OnMenuitemViewGridSelectionDlg()
{
	CRect cr;
	this->GetWindowRect(cr);
	CRect clienrect(CalcClientArea());

	// HUOM! popup-ikkunan sijainti on nyt hard codattu, koska en tied‰ miten kysyt‰‰n toolbar-buttonin sijaintia
	// varsinkaan zeditmap2view:sta k‰sin.
	m_SizeWnd.Create(this, cr.left + 142, cr.top + clienrect.top + 25);
}

void CFmiExtraMapViewDlg::SetMapViewGridSize(const NFmiPoint &newSize)
{
	if(itsSmartMetDocumentInterface->SetMapViewGrid(itsMapViewDescTopIndex, newSize))
        itsSmartMetDocumentInterface->RefreshApplicationViewsAndDialogs("Map view 2/3: Map view grid changed");
}

LRESULT CFmiExtraMapViewDlg::DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	static int counter = 0;
	counter++;

	if(message == TW_SETPARAMS)
	{
		m_cx.cx = LOWORD(wParam);
		m_cx.cy = HIWORD(wParam);
		SetMapViewGridSize(NFmiPoint(m_cx.cx, m_cx.cy));
		return (LRESULT)1;
	}

	return CDialogEx2::DefWindowProc(message, wParam, lParam);
}

void CFmiExtraMapViewDlg::OnShowGridPoints()
{
    itsSmartMetDocumentInterface->OnShowGridPoints(itsMapViewDescTopIndex);
}

BOOL CFmiExtraMapViewDlg::PreTranslateMessage(MSG* pMsg)
{
	static int counter = 0;
	counter++;

	// T‰m‰ nuolin‰pp‰inten hanskaus koodi on 'kopioitu' p‰‰karttan‰ytˆst‰
	// CZeditmap2View::OnKeyUp
	if(pMsg->message==WM_KEYUP)
	{
        bool ctrlOrShiftKeyDown = CtrlView::IsKeyboardKeyDown(VK_CONTROL) || CtrlView::IsKeyboardKeyDown(VK_SHIFT); // ctrl/shift avaimet eiv‰t saa olla painettuina, jos on, t‰llˆin ei nuolin‰pp‰imi‰ oteta huomioon, koska ne hoidetaan muualla keyboard-acceleraattoreilla
		switch(pMsg->wParam)
		{
		case VK_LEFT:
			if(!ctrlOrShiftKeyDown && itsSmartMetDocumentInterface->SetDataToPreviousTime(itsMapViewDescTopIndex))
                itsSmartMetDocumentInterface->RefreshApplicationViewsAndDialogs("Map view 2/3: Left cursor key pressed");
			break;
		case VK_RIGHT:
			if(!ctrlOrShiftKeyDown && itsSmartMetDocumentInterface->SetDataToNextTime(itsMapViewDescTopIndex))
                itsSmartMetDocumentInterface->RefreshApplicationViewsAndDialogs("Map view 2/3: Right cursor key pressed");
			break;
		case VK_UP:
			if(!ctrlOrShiftKeyDown && itsSmartMetDocumentInterface->ScrollViewRow(itsMapViewDescTopIndex, -1))
                itsSmartMetDocumentInterface->RefreshApplicationViewsAndDialogs("Map view 2/3: Up cursor key pressed");
			break;
		case VK_DOWN:
			if(!ctrlOrShiftKeyDown && itsSmartMetDocumentInterface->ScrollViewRow(itsMapViewDescTopIndex, 1))
                itsSmartMetDocumentInterface->RefreshApplicationViewsAndDialogs("Map view 2/3: Down cursor key pressed");
			break;
		}
		SetFocus(); // dialogille pit‰‰ antaa nuolin‰pp‰inten j‰lkeen fokus, muuten hiiren rulla ei toimi (fokus siirtyi jostain syyst‰ toolbar buttoneihin)
	}

    if (WM_KEYFIRST <= pMsg->message &&
        pMsg->message <= WM_KEYLAST)
    {
		BOOL status = false;
        HACCEL hAccel = m_hAccel;
        if (hAccel && ::TranslateAccelerator(m_hWnd, hAccel, pMsg))
            status = TRUE;

		SetFocus(); // dialogille pit‰‰ antaa myˆs esim. HOME jms. n‰pp‰inten j‰lkeen fokus, muuten hiiren rulla ei toimi (fokus siirtyi jostain syyst‰ toolbar buttoneihin tai jonnekin?!?!?)

		if(status)
			return status;
    }

    if(pMsg->message==WM_MOUSEWHEEL && this != GetFocus())
    {
        bool help = false;
        if(help)
    		SetFocus(); // dialogille pit‰‰ antaa fokus, muuten hiiren rulla ei toimi (fokus on mahdollisesti siirtynyt jostain syyst‰ toolbar buttoneihin tai jonnekin?!?!?)
    }

//     if (IsDialogMessage(pMsg))
//         return TRUE;
//     else
	return CDialogEx2::PreTranslateMessage(pMsg);
}

void CFmiExtraMapViewDlg::OnToggleGridPointColor()
{
    itsSmartMetDocumentInterface->OnToggleGridPointColor(itsMapViewDescTopIndex);
}

void CFmiExtraMapViewDlg::OnToggleGridPointSize()
{
    itsSmartMetDocumentInterface->OnToggleGridPointSize(itsMapViewDescTopIndex);
}

void CFmiExtraMapViewDlg::OnEditSpaceOut()
{
    itsSmartMetDocumentInterface->OnEditSpaceOut(itsMapViewDescTopIndex);
}

void CFmiExtraMapViewDlg::OnChangeParamWindowPositionForward()
{
	itsSmartMetDocumentInterface->OnChangeParamWindowPosition(itsMapViewDescTopIndex, true);
}

void CFmiExtraMapViewDlg::OnChangeParamWindowPositionBackward()
{
	itsSmartMetDocumentInterface->OnChangeParamWindowPosition(itsMapViewDescTopIndex, false);
}

void CFmiExtraMapViewDlg::OnShowTimeString()
{
    itsSmartMetDocumentInterface->OnShowTimeString(itsMapViewDescTopIndex);
}

void CFmiExtraMapViewDlg::OnButtonRefresh()
{
    itsSmartMetDocumentInterface->OnButtonRefresh("Refreshing all views (F5)");
}

void CFmiExtraMapViewDlg::OnChangeMapType()
{
    itsSmartMetDocumentInterface->OnChangeMapType(itsMapViewDescTopIndex, true);
}

void CFmiExtraMapViewDlg::OnToggleShowNamesOnMap()
{
    itsSmartMetDocumentInterface->OnToggleShowNamesOnMap(itsMapViewDescTopIndex, true);
}


void CFmiExtraMapViewDlg::OnShowMasksOnMap()
{
    itsSmartMetDocumentInterface->OnShowMasksOnMap(itsMapViewDescTopIndex);
}

void CFmiExtraMapViewDlg::OnShowProjectionLines()
{
    itsSmartMetDocumentInterface->OnShowProjectionLines();
}

void CFmiExtraMapViewDlg::OnToggleLandBorderDrawColor()
{
    itsSmartMetDocumentInterface->OnToggleLandBorderDrawColor(itsMapViewDescTopIndex);
}

void CFmiExtraMapViewDlg::OnToggleLandBorderPenSize()
{
    itsSmartMetDocumentInterface->OnToggleLandBorderPenSize(itsMapViewDescTopIndex);
}

void CFmiExtraMapViewDlg::OnAcceleratorBorrowParams1()
{
    itsSmartMetDocumentInterface->OnAcceleratorBorrowParams(itsMapViewDescTopIndex, 1);
}

void CFmiExtraMapViewDlg::OnAcceleratorBorrowParams2()
{
    itsSmartMetDocumentInterface->OnAcceleratorBorrowParams(itsMapViewDescTopIndex, 2);
}

void CFmiExtraMapViewDlg::OnAcceleratorBorrowParams3()
{
    itsSmartMetDocumentInterface->OnAcceleratorBorrowParams(itsMapViewDescTopIndex, 3);
}

void CFmiExtraMapViewDlg::OnAcceleratorBorrowParams4()
{
    itsSmartMetDocumentInterface->OnAcceleratorBorrowParams(itsMapViewDescTopIndex, 4);
}

void CFmiExtraMapViewDlg::OnAcceleratorBorrowParams5()
{
    itsSmartMetDocumentInterface->OnAcceleratorBorrowParams(itsMapViewDescTopIndex, 5);
}

void CFmiExtraMapViewDlg::OnAcceleratorBorrowParams6()
{
	itsSmartMetDocumentInterface->OnAcceleratorBorrowParams(itsMapViewDescTopIndex, 6);
}

void CFmiExtraMapViewDlg::OnAcceleratorBorrowParams7()
{
	itsSmartMetDocumentInterface->OnAcceleratorBorrowParams(itsMapViewDescTopIndex, 7);
}

void CFmiExtraMapViewDlg::OnAcceleratorBorrowParams8()
{
	itsSmartMetDocumentInterface->OnAcceleratorBorrowParams(itsMapViewDescTopIndex, 8);
}

void CFmiExtraMapViewDlg::OnAcceleratorBorrowParams9()
{
	itsSmartMetDocumentInterface->OnAcceleratorBorrowParams(itsMapViewDescTopIndex, 9);
}

void CFmiExtraMapViewDlg::OnAcceleratorBorrowParams10()
{
	itsSmartMetDocumentInterface->OnAcceleratorBorrowParams(itsMapViewDescTopIndex, 10);
}

void CFmiExtraMapViewDlg::OnAcceleratorMapRow1()
{
    itsSmartMetDocumentInterface->OnAcceleratorMapRow(itsMapViewDescTopIndex, 1);
}

void CFmiExtraMapViewDlg::OnAcceleratorMapRow2()
{
    itsSmartMetDocumentInterface->OnAcceleratorMapRow(itsMapViewDescTopIndex, 2);
}

void CFmiExtraMapViewDlg::OnAcceleratorMapRow3()
{
    itsSmartMetDocumentInterface->OnAcceleratorMapRow(itsMapViewDescTopIndex, 3);
}

void CFmiExtraMapViewDlg::OnAcceleratorMapRow4()
{
    itsSmartMetDocumentInterface->OnAcceleratorMapRow(itsMapViewDescTopIndex, 4);
}

void CFmiExtraMapViewDlg::OnAcceleratorMapRow5()
{
    itsSmartMetDocumentInterface->OnAcceleratorMapRow(itsMapViewDescTopIndex, 5);
}

void CFmiExtraMapViewDlg::OnAcceleratorMapRow6()
{
    itsSmartMetDocumentInterface->OnAcceleratorMapRow(itsMapViewDescTopIndex, 6);
}

void CFmiExtraMapViewDlg::OnAcceleratorMapRow7()
{
    itsSmartMetDocumentInterface->OnAcceleratorMapRow(itsMapViewDescTopIndex, 7);
}

void CFmiExtraMapViewDlg::OnAcceleratorMapRow8()
{
    itsSmartMetDocumentInterface->OnAcceleratorMapRow(itsMapViewDescTopIndex, 8);
}

void CFmiExtraMapViewDlg::OnAcceleratorMapRow9()
{
    itsSmartMetDocumentInterface->OnAcceleratorMapRow(itsMapViewDescTopIndex, 9);
}

void CFmiExtraMapViewDlg::OnAcceleratorMapRow10()
{
    itsSmartMetDocumentInterface->OnAcceleratorMapRow(itsMapViewDescTopIndex, 10);
}

void CFmiExtraMapViewDlg::OnAcceleratorToggleHelpCursorOnMap()
{
    itsSmartMetDocumentInterface->ShowMouseHelpCursorsOnMap(!itsSmartMetDocumentInterface->ShowMouseHelpCursorsOnMap());
    itsSmartMetDocumentInterface->ForceDrawOverBitmapThings(itsMapViewDescTopIndex, true, true);
}

void CFmiExtraMapViewDlg::OnAcceleratorToggleOvermapForeBackGround()
{
    itsSmartMetDocumentInterface->OnToggleOverMapBackForeGround(itsMapViewDescTopIndex);
}

void CFmiExtraMapViewDlg::OnAcceleratorToggleTooltip()
{
    itsSmartMetDocumentInterface->MetEditorOptionsData().ShowToolTipsOnMapView(!itsSmartMetDocumentInterface->MetEditorOptionsData().ShowToolTipsOnMapView());
}

void CFmiExtraMapViewDlg::OnAcceleratorToggleKeepMapRatio()
{
    itsSmartMetDocumentInterface->OnAcceleratorToggleKeepMapRatio();
}

#ifndef _countof
#define _countof(array) (sizeof(array)/sizeof(array[0]))
#endif

// t‰m‰n overriden avulla saadaan tooltippiin kieli versio aikaiseksi
// eli t‰m‰ tooltip on toolbar-nappuloiden tooltippi
BOOL CFmiExtraMapViewDlg::OnToolTipText(UINT id, NMHDR* pNMHDR, LRESULT* pResult)
{
	BOOL status = CDialogEx2::OnToolTipText(id, pNMHDR, pResult);

	if(status)
	{
        TOOLTIPTEXT* pTTTA = (TOOLTIPTEXT*)pNMHDR;
        UINT nID = static_cast<UINT>(pNMHDR->idFrom);
        std::string str = CT2A(pTTTA->szText);
        CString realStrU_ = CA2T(itsSmartMetDocumentInterface->GetToolTipString(nID, str).c_str());
        _tcsncpy(pTTTA->szText, realStrU_, static_cast<int>(_tcslen(pTTTA->szText)));
	}
    return status;
}

#undef _countof

void CFmiExtraMapViewDlg::OnButtonMapViewSettings()
{
    NFmiMapViewWinRegistry *mapViewWinRegistry = itsSmartMetDocumentInterface->ApplicationWinRegistry().ConfigurationRelatedWinRegistry().MapView(itsMapViewDescTopIndex).get();
    auto mapViewDescTop = itsSmartMetDocumentInterface->MapViewDescTop(itsMapViewDescTopIndex);
    CFmiMapViewSettingsDlg dlg(*mapViewDescTop, *mapViewWinRegistry, this);
	if(dlg.DoModal() == IDOK)
	{
		// viewGridsize pit‰‰ laskea uudestaan, koska displayType on saattanut muuttua ja aloitus rivi pit‰‰ ehk‰ vaihtaa
        mapViewDescTop->ViewGridSize(mapViewDescTop->ViewGridSize(), mapViewWinRegistry);
        mapViewDescTop->MapViewDirty(true, true, true, false);
        mapViewDescTop->SetBorderDrawDirtyState(CountryBorderDrawDirtyState::Geometry);
        itsSmartMetDocumentInterface->UpdateRowInLockedDescTops(itsMapViewDescTopIndex);
        itsSmartMetDocumentInterface->RefreshApplicationViewsAndDialogs("Map view 2/3: Extra map view settings changed");
	}
}

CBitmap* CFmiExtraMapViewDlg::MemoryBitmap(void)
{
	if(itsView)
		return itsView->MemoryBitmap();
    else
    	return nullptr;
}

CBitmap* CFmiExtraMapViewDlg::FinalMapViewImageBitmap() 
{ 
    if(itsView)
        return itsView->FinalMapViewImageBitmap();
    else
        return nullptr;
}

void CFmiExtraMapViewDlg::OnButtonZoomDialog()
{
    itsSmartMetDocumentInterface->ActivateZoomDialog(itsMapViewDescTopIndex);
}

void CFmiExtraMapViewDlg::OnButtonSelectParamDialogExtraMap()
{
    ApplicationInterface::GetApplicationInterfaceImplementation()->ActivateParameterSelectionDlg(itsMapViewDescTopIndex);
}

void CFmiExtraMapViewDlg::OnToggleMapViewDisplayMode()
{
    itsSmartMetDocumentInterface->MapViewDescTop(itsMapViewDescTopIndex)->ToggleMapViewDisplayMode();
    itsSmartMetDocumentInterface->MapViewDescTop(itsMapViewDescTopIndex)->MapViewDirty(false, true, true, true);
    itsSmartMetDocumentInterface->RefreshApplicationViewsAndDialogs("Map view 2/3: Toggle map view display mode", ::GetWantedMapViewIdFlag(itsMapViewDescTopIndex));
}

void CFmiExtraMapViewDlg::OnButtonExtraMapPrint()
{
	itsView->DoPrint();
	SetActiveWindow(); // aktivoidaan viel‰ t‰m‰ ikkuna, koska jostain syyst‰ print-dialogi aktivoi p‰‰ikkunan
}

CString CFmiExtraMapViewDlg::GetFinalStatusString(UINT nID, const CString &strPromtpText)
{
	std::string tmpStr = CT2A(strPromtpText);
	return CString(CA2T(itsSmartMetDocumentInterface->GetToolTipString(nID, tmpStr).c_str()));
}

void CFmiExtraMapViewDlg::OnAcceleratorLockTimeToMainMap()
{
    itsSmartMetDocumentInterface->MapViewDescTop(itsMapViewDescTopIndex)->LockToMainMapViewTime(!itsSmartMetDocumentInterface->MapViewDescTop(itsMapViewDescTopIndex)->LockToMainMapViewTime());
    itsSmartMetDocumentInterface->RefreshApplicationViewsAndDialogs("Map view 2/3: Toggle lock time to main map view", SmartMetViewId::AllMapViews);
}

void CFmiExtraMapViewDlg::OnAcceleratorLockRowToMainMap()
{
    itsSmartMetDocumentInterface->MapViewDescTop(itsMapViewDescTopIndex)->LockToMainMapViewRow(!itsSmartMetDocumentInterface->MapViewDescTop(itsMapViewDescTopIndex)->LockToMainMapViewRow());
    itsSmartMetDocumentInterface->UpdateRowInLockedDescTops(itsMapViewDescTopIndex);
    itsSmartMetDocumentInterface->RefreshApplicationViewsAndDialogs("Map view 2/3: Toggle lock row to main map view", SmartMetViewId::AllMapViews);
}

void CFmiExtraMapViewDlg::OnAcceleratorMakeSwapBaseArea()
{
    itsSmartMetDocumentInterface->MakeSwapBaseArea(itsMapViewDescTopIndex);
}

void CFmiExtraMapViewDlg::OnAcceleratorSwapArea()
{
    itsSmartMetDocumentInterface->SwapArea(itsMapViewDescTopIndex);
    itsSmartMetDocumentInterface->RefreshApplicationViewsAndDialogs("Map view 2/3: Swap zoomed map area (SPACE)");
}

void CFmiExtraMapViewDlg::OnAcceleratorSwapAreaSecondaryKey()
{
	itsSmartMetDocumentInterface->SwapArea(itsMapViewDescTopIndex);
	itsSmartMetDocumentInterface->RefreshApplicationViewsAndDialogs("Map view 2/3: Swap zoomed map area (CTRL + SHIFT + SPACE -> secondary key)");
}

void CFmiExtraMapViewDlg::OnEditCopy()
{
    itsSmartMetDocumentInterface->CopyDrawParamsFromMapViewRow(itsMapViewDescTopIndex);
}

void CFmiExtraMapViewDlg::OnEditPaste()
{
    itsSmartMetDocumentInterface->PasteDrawParamsToMapViewRow(itsMapViewDescTopIndex);
}

void CFmiExtraMapViewDlg::OnAcceleratorCopyAllMapViewParams()
{
    itsSmartMetDocumentInterface->CopyMapViewDescTopParams(itsMapViewDescTopIndex);
}

void CFmiExtraMapViewDlg::OnAcceleratorPasteAllMapViewParams()
{
    itsSmartMetDocumentInterface->PasteMapViewDescTopParams(itsMapViewDescTopIndex);
}

void CFmiExtraMapViewDlg::OnAcceleratorToggleAnimationView()
{
    itsSmartMetDocumentInterface->ToggleTimeControlAnimationView(itsMapViewDescTopIndex);
}

void CFmiExtraMapViewDlg::OnAcceleratorSetHomeTime()
{
    itsSmartMetDocumentInterface->OnAcceleratorSetHomeTime(itsMapViewDescTopIndex);
}

void CFmiExtraMapViewDlg::OnToggleShowPreviousNamesOnMap()
{
    itsSmartMetDocumentInterface->OnToggleShowNamesOnMap(itsMapViewDescTopIndex, false);
}

void CFmiExtraMapViewDlg::OnChangePreviousMapType()
{
    itsSmartMetDocumentInterface->OnChangeMapType(itsMapViewDescTopIndex, false);
}

void CFmiExtraMapViewDlg::OnButtonAnimation()
{
    itsSmartMetDocumentInterface->ToggleTimeControlAnimationView(itsMapViewDescTopIndex);
}

void CFmiExtraMapViewDlg::OnAcceleratorExtraMapPanDown()
{
    CFmiWin32TemplateHelpers::ArrowKeyMapPan(itsView, itsSmartMetDocumentInterface, NFmiPoint(0, 0.05));
}

void CFmiExtraMapViewDlg::OnAcceleratorExtraMapPanLeft()
{
    CFmiWin32TemplateHelpers::ArrowKeyMapPan(itsView, itsSmartMetDocumentInterface, NFmiPoint(-0.05, 0));
}

void CFmiExtraMapViewDlg::OnAcceleratorExtraMapPanRight()
{
    CFmiWin32TemplateHelpers::ArrowKeyMapPan(itsView, itsSmartMetDocumentInterface, NFmiPoint(0.05, 0));
}

void CFmiExtraMapViewDlg::OnAcceleratorExtraMapPanUp()
{
    CFmiWin32TemplateHelpers::ArrowKeyMapPan(itsView, itsSmartMetDocumentInterface, NFmiPoint(0, -0.05));
}

void CFmiExtraMapViewDlg::OnAcceleratorExtraMapZoomIn()
{
    CFmiWin32TemplateHelpers::ArrowKeyMapZoom(itsView, itsSmartMetDocumentInterface, 0.95);
}

void CFmiExtraMapViewDlg::OnAcceleratorExtraMapZoomOut()
{
    CFmiWin32TemplateHelpers::ArrowKeyMapZoom(itsView, itsSmartMetDocumentInterface, 1.05);
}

void CFmiExtraMapViewDlg::OnGetMinMaxInfo(MINMAXINFO* lpMMI)
{
    // set the minimum tracking width and height of the window
    lpMMI->ptMinTrackSize.x = 250;
    lpMMI->ptMinTrackSize.y = 250;
}

const int gManyRowsCount = 10;

void CFmiExtraMapViewDlg::OnAcceleratorMoveManyMapRowsUp()
{
    if(itsSmartMetDocumentInterface->ScrollViewRow(itsMapViewDescTopIndex, -gManyRowsCount))
        itsSmartMetDocumentInterface->RefreshApplicationViewsAndDialogs("Map view 2/3: Jump up in starting absolute row number");
}

void CFmiExtraMapViewDlg::OnAcceleratorMoveManyMapRowsDown()
{
    if(itsSmartMetDocumentInterface->ScrollViewRow(itsMapViewDescTopIndex, gManyRowsCount))
        itsSmartMetDocumentInterface->RefreshApplicationViewsAndDialogs("Map view 2/3: Jump down in starting absolute row number");
}

void CFmiExtraMapViewDlg::OnAcceleratorExtraMapLocationFinderTool()
{
    ApplicationInterface::GetApplicationInterfaceImplementation()->OpenLocationFinderDialog(this);
}

void CFmiExtraMapViewDlg::OnAcceleratorChangeTimeByStep1Forward()
{
	itsSmartMetDocumentInterface->ChangeTime(1, kForward, itsMapViewDescTopIndex, 1);
}

void CFmiExtraMapViewDlg::OnAcceleratorChangeTimeByStep1Backward()
{
	itsSmartMetDocumentInterface->ChangeTime(1, kBackward, itsMapViewDescTopIndex, 1);
}

void CFmiExtraMapViewDlg::OnAcceleratorChangeTimeByStep2Forward()
{
	itsSmartMetDocumentInterface->ChangeTime(2, kForward, itsMapViewDescTopIndex, 1);
}

void CFmiExtraMapViewDlg::OnAcceleratorChangeTimeByStep2Backward()
{
	itsSmartMetDocumentInterface->ChangeTime(2, kBackward, itsMapViewDescTopIndex, 1);
}

void CFmiExtraMapViewDlg::OnAcceleratorChangeTimeByStep3Forward()
{
	itsSmartMetDocumentInterface->ChangeTime(3, kForward, itsMapViewDescTopIndex, 1);
}

void CFmiExtraMapViewDlg::OnAcceleratorChangeTimeByStep3Backward()
{
	itsSmartMetDocumentInterface->ChangeTime(3, kBackward, itsMapViewDescTopIndex, 1);
}

void CFmiExtraMapViewDlg::OnAcceleratorChangeTimeByStep4Forward()
{
	itsSmartMetDocumentInterface->ChangeTime(4, kForward, itsMapViewDescTopIndex, 1);
}

void CFmiExtraMapViewDlg::OnAcceleratorChangeTimeByStep4Backward()
{
	itsSmartMetDocumentInterface->ChangeTime(4, kBackward, itsMapViewDescTopIndex, 1);
}

void CFmiExtraMapViewDlg::OnEditVisualizationsettingsExtraMap()
{
	itsSmartMetDocumentInterface->OpenVisualizationsettingsDialog();
}

void CFmiExtraMapViewDlg::OnAcceleratorEmChangeAllModelDataOnRowToPreviousModelRun()
{
	itsSmartMetDocumentInterface->GetCombinedMapHandlerInterface().setModelRunOffsetForAllModelDataOnActiveRow(itsMapViewDescTopIndex, kBackward);
}

void CFmiExtraMapViewDlg::OnAcceleratorEmChangeAllModelDataOnRowToNextModelRun()
{
	itsSmartMetDocumentInterface->GetCombinedMapHandlerInterface().setModelRunOffsetForAllModelDataOnActiveRow(itsMapViewDescTopIndex, kForward);
}

void CFmiExtraMapViewDlg::OnAcceleratorExtraMapViewRangeMeterModeToggle()
{
	auto& rangeMeter = itsSmartMetDocumentInterface->ApplicationWinRegistry().ConfigurationRelatedWinRegistry().MapViewRangeMeter();
	rangeMeter.ModeOnToggle();
	// Kun rangeMeter laitetaan toimintaan, kannattaa karttojen tooltipin aukeamisaikaa laittaa pidemm‰ksi, 
	// muuten tooltip kuplaa pit‰‰ koko ajan klikata hiirell‰ ennen kuin p‰‰see tekem‰‰n mouse left-click-drag:ia
	ApplicationInterface::GetApplicationInterfaceImplementation()->SetAllMapViewTooltipDelays(!rangeMeter.ModeOn(), NFmiMapViewRangeMeterWinRegistry::TooltipDelayInMS);
	ApplicationInterface::GetApplicationInterfaceImplementation()->ForceDrawOverBitmapThings(itsMapViewDescTopIndex, true, true);
}

void CFmiExtraMapViewDlg::OnAcceleratorExtraMapViewRangeMeterColorToggle()
{
	auto& mapViewRangeMeter = itsSmartMetDocumentInterface->ApplicationWinRegistry().ConfigurationRelatedWinRegistry().MapViewRangeMeter();
	if(mapViewRangeMeter.ModeOn())
	{
		mapViewRangeMeter.ToggleColor();
		ApplicationInterface::GetApplicationInterfaceImplementation()->ForceDrawOverBitmapThings(itsMapViewDescTopIndex, true, true);
	}
}

void CFmiExtraMapViewDlg::OnAcceleratorExtraMapViewRangeMeterLockModeToggle()
{
	auto& mapViewRangeMeter = itsSmartMetDocumentInterface->ApplicationWinRegistry().ConfigurationRelatedWinRegistry().MapViewRangeMeter();
	if(mapViewRangeMeter.ModeOn())
	{
		mapViewRangeMeter.LockModeOnToggle();
		ApplicationInterface::GetApplicationInterfaceImplementation()->ForceDrawOverBitmapThings(itsMapViewDescTopIndex, true, true);
	}
}

void CFmiExtraMapViewDlg::OnAcceleratorLogViewerToolboxdeb()
{
	ApplicationInterface::GetApplicationInterfaceImplementation()->OpenLogViewer();
}


void CFmiExtraMapViewDlg::OnButtonOpenMainMapViewExtraMap()
{
	ApplicationInterface::GetApplicationInterfaceImplementation()->OpenMainMapView();
}


void CFmiExtraMapViewDlg::OnButtonOpenOtherSideMapViewExtraMap()
{
	unsigned int otherSideMapViewIndex = (itsMapViewDescTopIndex == 1) ? 2 : 1;
	ApplicationInterface::GetApplicationInterfaceImplementation()->OpenSideMapView(otherSideMapViewIndex);
}


void CFmiExtraMapViewDlg::OnButtonOpenTimeSerialViewExtraMap()
{
	ApplicationInterface::GetApplicationInterfaceImplementation()->OpenTimeSerialView();
}


void CFmiExtraMapViewDlg::OnButtonOpenSmarttoolsDialogExtraMap()
{
	ApplicationInterface::GetApplicationInterfaceImplementation()->OpenSmarttoolsDialog();
}


void CFmiExtraMapViewDlg::OnButtonOpenSoundingViewExtraMap()
{
	ApplicationInterface::GetApplicationInterfaceImplementation()->OpenSoundingView();
}


void CFmiExtraMapViewDlg::OnButtonOpenCrosssectionViewExtraMap()
{
	ApplicationInterface::GetApplicationInterfaceImplementation()->OpenCrossSectionView();
}


void CFmiExtraMapViewDlg::OnButtonOpenViewMacroDialogExtraMap()
{
	ApplicationInterface::GetApplicationInterfaceImplementation()->OpenViewMacroDialog();
}


void CFmiExtraMapViewDlg::OnAcceleratorExtraMapMoveTimeBoxLocation()
{
	itsSmartMetDocumentInterface->GetCombinedMapHandlerInterface().onMoveTimeBoxLocation(itsMapViewDescTopIndex);
}


void CFmiExtraMapViewDlg::OnAcceleratorExtraMapObsComparisonMode()
{
	itsSmartMetDocumentInterface->MapViewDescTop(itsMapViewDescTopIndex)->ShowObsComparisonOnMap(!itsSmartMetDocumentInterface->MapViewDescTop(itsMapViewDescTopIndex)->ShowObsComparisonOnMap());
	itsSmartMetDocumentInterface->GetCombinedMapHandlerInterface().mapViewDirty(itsMapViewDescTopIndex, false, true, true, false, false, false);
	itsSmartMetDocumentInterface->RefreshApplicationViewsAndDialogs("Map view 2/3: Obs comparison mode on/off", ::GetWantedMapViewIdFlag(itsMapViewDescTopIndex));
}



void CFmiExtraMapViewDlg::OnAcceleratorExtraMapToggleVirtualTimeMode()
{
	itsSmartMetDocumentInterface->ToggleVirtualTimeMode();
	if(ApplicationInterface::GetApplicationInterfaceImplementation)
	{
		std::string updateMessage = "Virtual-Time mode changed from Map view " + std::to_string(itsMapViewDescTopIndex + 1) + " by CRTL + K";
		ApplicationInterface::GetApplicationInterfaceImplementation()->UpdateAllViewsAndDialogs(updateMessage);
	}
}
