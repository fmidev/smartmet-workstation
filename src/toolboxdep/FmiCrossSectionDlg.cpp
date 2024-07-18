#include "stdafx.h"
#include "FmiCrossSectionDlg.h"
#include "FmiCrossSectionView.h"
#include "FmiModifyDrawParamDlg.h"
#include "SmartMetDocumentInterface.h"
#include "NFmiCrossSectionSystem.h"
#include "NFmiDictionaryFunction.h"
#include "NFmiTrajectorySystem.h"
#include "FmiWin32TemplateHelpers.h"
#include "FmiWin32Helpers.h"
#include "CloneBitmap.h"
#include "NFmiApplicationWinRegistry.h"
#include "CtrlViewFunctions.h"
#include "persist2.h"
#include "NFmiMacroParamDataCache.h"
#include "SpecialDesctopIndex.h"
#include "ApplicationInterface.h"


// CFmiCrossSectionDlg dialog

const NFmiViewPosRegistryInfo CFmiCrossSectionDlg::s_ViewPosRegistryInfo(CRect(400, 200, 1000, 700), "\\CrossSectionView");

IMPLEMENT_DYNAMIC(CFmiCrossSectionDlg, CDialog)

CFmiCrossSectionDlg::CFmiCrossSectionDlg(SmartMetDocumentInterface *smartMetDocumentInterface, CWnd* pParent /*=NULL*/)
:CDialog(CFmiCrossSectionDlg::IDD, pParent)
,itsView(0)
,itsSmartMetDocumentInterface(smartMetDocumentInterface)
,m_hAccel(0)
, fUseTimeCrossSection(FALSE)
, fUseCrossSectionMapMode(FALSE)
, fUse3PointMode(FALSE)
, fUseRouteCrossSection(FALSE)
, fShowHybridLevels(FALSE)
, fShowTrajectories(FALSE)
, fUseObsAndForCrossSection(FALSE)
{
}

CFmiCrossSectionDlg::~CFmiCrossSectionDlg()
{
	if(itsParameterSelectionButtonBitmap)
		DeleteObject(itsParameterSelectionButtonBitmap);
}

void CFmiCrossSectionDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Check(pDX, IDC_CHECK_USE_TIME_CROSS_SECTION, fUseTimeCrossSection);
	DDX_Check(pDX, IDC_CHECK_USE_ROUTE_CROSS_SECTION, fUseRouteCrossSection);

	DDX_Check(pDX, IDC_CHECK_USE_CROSSSECTION_MAP_MODE, fUseCrossSectionMapMode);
	DDX_Check(pDX, IDC_CHECK_USE_CROSSSECTION_3_POINT_MODE, fUse3PointMode);
	DDX_Control(pDX, IDC_SLIDER_CROSS_SECTION_VIEW_COUNT, itsViewCountSlider);
	DDX_Check(pDX, IDC_CHECK_CROSSSECTION_SHOW_HYBRID_LEVELS, fShowHybridLevels);
	DDX_Check(pDX, IDC_CHECK_CROSSSECTION_SHOW_TRAJECTORIES, fShowTrajectories);
	DDX_Check(pDX, IDC_CHECK_USE_OBSANDFOR_CROSS_SECTION, fUseObsAndForCrossSection);
}


BEGIN_MESSAGE_MAP(CFmiCrossSectionDlg, CDialog)
	ON_BN_CLICKED(IDC_BUTTON_PRINT, OnBnClickedButtonPrint)
	ON_WM_SIZE()
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
ON_WM_MOUSEWHEEL()
ON_BN_CLICKED(IDC_CHECK_USE_TIME_CROSS_SECTION, OnBnClickedCheckUseTimeCrossSection)
ON_BN_CLICKED(IDC_CHECK_USE_CROSSSECTION_MAP_MODE, OnBnClickedCheckUseCrosssectionMapMode)
ON_WM_HSCROLL()
ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_SLIDER_CROSS_SECTION_VIEW_COUNT, OnNMReleasedcaptureSliderCrossSectionViewCount)
ON_BN_CLICKED(IDC_CHECK_USE_CROSSSECTION_3_POINT_MODE, OnBnClickedCheckUseCrosssection3PointMode2)
ON_WM_CLOSE()
ON_BN_CLICKED(IDC_CHECK_USE_ROUTE_CROSS_SECTION, OnBnClickedCheckUseRouteCrossSection)
ON_BN_CLICKED(IDC_CHECK_CROSSSECTION_SHOW_HYBRID_LEVELS, OnBnClickedCheckCrosssectionShowHybridLevels)
ON_BN_CLICKED(IDC_CHECK_CROSSSECTION_SHOW_TRAJECTORIES, OnBnClickedCheckCrosssectionShowTrajectories)
ON_COMMAND(ID_ACCELERATOR_TOGGLE_TEMP_VIEW_TOOLTIP, OnAcceleratorToggleTooltip)
ON_BN_CLICKED(IDC_CHECK_USE_OBSANDFOR_CROSS_SECTION, OnBnClickedCheckUseObsandforCrossSection)
ON_COMMAND(ID_CHANGE_PARAM_WINDOW_POSITION_FORWARD_EXTRA_MAP, CFmiCrossSectionDlg::OnChangeParamWindowPositionForward)
ON_COMMAND(ID_CHANGE_PARAM_WINDOW_POSITION_BACKWARD_EXTRA_MAP, CFmiCrossSectionDlg::OnChangeParamWindowPositionBackward)
ON_COMMAND(ID_ACCELERATOR_SET_CROSS_SECTION_SPECIAL_AXIS, &CFmiCrossSectionDlg::OnAcceleratorSetCrossSectionSpecialAxis)
ON_COMMAND(ID_ACCELERATOR_SPACE_OUT_TEMP_WINDS, &CFmiCrossSectionDlg::OnAcceleratorSetCrossSectionDefaultAxis)
ON_COMMAND(ID_ACCELERATOR_SET_CROSS_SECTION_DEFAULT_AXIS_ALL, &CFmiCrossSectionDlg::OnAcceleratorSetCrossSectionDefaultAxisAll)
ON_COMMAND(ID_ACCELERATOR_SET_CROSS_SECTION_SPECIAL_AXIS_ALL, &CFmiCrossSectionDlg::OnAcceleratorSetCrossSectionSpecialAxisAll)
ON_COMMAND(ID_ACCELERATOR_SAVE_CROSS_SECTION_DEFAULT_AXIS, &CFmiCrossSectionDlg::OnAcceleratorSaveCrossSectionDefaultAxis)
ON_COMMAND(ID_ACCELERATOR_SAVE_CROSS_SECTION_SPECIAL_AXIS, &CFmiCrossSectionDlg::OnAcceleratorSaveCrossSectionSpecialAxis)
ON_BN_CLICKED(IDC_BUTTON_CROSS_SECTION_PARAMETER_SELECTION, &CFmiCrossSectionDlg::OnButtonOpenParameterSelection)
ON_COMMAND(ID_ACCELERATOR_LOG_VIEWER_TOOLBOXDEB, &CFmiCrossSectionDlg::OnAcceleratorLogViewerToolboxdeb)
ON_COMMAND(ID_ACCELERATOR_EXTRA_MAP_TOGGLE_VIRTUAL_TIME_MODE, &CFmiCrossSectionDlg::OnAcceleratorExtraMapToggleVirtualTimeMode)
END_MESSAGE_MAP()

// CFmiCrossSectionDlg message handlers

void CFmiCrossSectionDlg::Update()
{
    if(IsWindowVisible() && !IsIconic()) // Näyttöä päivitetään vain jos se on näkyvissä ja se ei ole minimized tilassa
    {
        UpdateMapMode();
        fShowTrajectories = itsSmartMetDocumentInterface->TrajectorySystem()->ShowTrajectoriesInCrossSectionView();
		UpdateData(FALSE);
		EnableControls();
		if(itsView)
			itsView->Update(true);
		Invalidate(FALSE);
	}
}

void CFmiCrossSectionDlg::OnBnClickedButtonPrint()
{
	((CFmiCrossSectionView*)itsView)->DoPrint();
	SetActiveWindow(); // aktivoidaan vielä tämä ikkuna, koska jostain syystä print-dialogi aktivoi pääikkunan
}

int CFmiCrossSectionDlg::GetDialogButtonHeight(void)
{
	int buttonAreaHeight = 20;
	CWnd *win = GetDlgItem(IDC_BUTTON_PRINT);
	if(win)
	{
		CRect rect;
		win->GetWindowRect(rect);
		buttonAreaHeight = rect.Height();
	}
	return buttonAreaHeight;
}

CRect CFmiCrossSectionDlg::CalcClientArea(void)
{
	CRect rect;
	GetClientRect(rect);
	rect.top = rect.top + GetDialogButtonHeight() + 5;
	return rect;
}

// tämä alue pitää maalata myös, koska muuten välkkymätön näyttö ei onnistu
CRect CFmiCrossSectionDlg::CalcOtherArea(void)
{
	CRect rect;
	GetClientRect(rect);
	CRect realClientRect(CalcClientArea());
	rect.bottom = rect.top + rect.Height() - realClientRect.Height();
	return rect;
}

BOOL CFmiCrossSectionDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

    // Load accelerators
    m_hAccel = ::LoadAccelerators(AfxGetResourceHandle(), MAKEINTRESOURCE(IDR_ACCELERATOR1));
    ASSERT(m_hAccel);

	CFmiWin32Helpers::SetUsedWindowIconDynamically(this);
	std::string errorBaseStr("Error in CFmiCrossSectionDlg::OnInitDialog while reading dialog size and position values");
    CFmiWin32TemplateHelpers::DoWindowSizeSettingsFromWinRegistry(itsSmartMetDocumentInterface->ApplicationWinRegistry(), this, false, errorBaseStr, 0);

    CRect rect(CalcClientArea());
	itsView = new CFmiCrossSectionView(this, itsSmartMetDocumentInterface);
	itsView->Create(NULL,NULL, WS_VISIBLE | WS_CHILD, rect, this, NULL);
	itsView->OnInitialUpdate(); // pitää kutsua erikseen, koska formvieta ei ole sidottu dokumenttiin
	SetWindowPos(&wndBottom, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);	 // not possible, thus child stays still upon its parent

	InitFromCrossSectionSystem();
	SetParameterSelectionIcon();
	InitDialogTexts();
	UpdateData(FALSE);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CFmiCrossSectionDlg::InitFromCrossSectionSystem(void)
{
    auto crossSectionSystem = itsSmartMetDocumentInterface->CrossSectionSystem();
	itsViewCountSlider.SetRange(1, crossSectionSystem->MaxViewRowSize());
	itsViewCountSlider.SetPos(crossSectionSystem->RowCount());
	fUseRouteCrossSection = crossSectionSystem->UseRouteCrossSection();
	fUseTimeCrossSection = crossSectionSystem->UseTimeCrossSection();
	fUseCrossSectionMapMode = crossSectionSystem->CrossSectionSystemActive();
	fUse3PointMode = crossSectionSystem->CrossSectionMode() == NFmiCrossSectionSystem::k3Point;
	fShowHybridLevels = crossSectionSystem->ShowHybridLevels();
	EnableControls();

	UpdateData(FALSE);
}

void CFmiCrossSectionDlg::SetParameterSelectionIcon()
{
	CButton* pButton = (CButton*)GetDlgItem(IDC_BUTTON_CROSS_SECTION_PARAMETER_SELECTION);

	pButton->ModifyStyle(0, BS_BITMAP);

	itsParameterSelectionButtonBitmap = (HBITMAP)LoadImage(
		AfxGetApp()->m_hInstance,
		MAKEINTRESOURCE(IDB_BITMAP_PLUS),
		IMAGE_BITMAP,
		16, 16,
		LR_DEFAULTCOLOR
	);

	pButton->SetBitmap(itsParameterSelectionButtonBitmap);
}

void CFmiCrossSectionDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);

	if(itsView)
		itsView->MoveWindow(CalcClientArea(), FALSE);
	Invalidate(FALSE);
}

BOOL CFmiCrossSectionDlg::OnEraseBkgnd(CDC* pDC)
{
	return FALSE; // näin ruutu ei välky

//	return CView::OnEraseBkgnd(pDC);
}

void CFmiCrossSectionDlg::OnPaint()
{
	CPaintDC dc(this); // device context for painting

 // tämä on pika viritys, kun muuten Print-nappulan kohdalta jää kaista maalaamatta kun laitoin ikkunaan välkkymättömän päivityksen
	CBrush brush(RGB(239, 235, 222));
	CRect area(CalcOtherArea());
	dc.FillRect(&area, &brush);

// Do not call CDialog::OnPaint() for painting messages
}

BOOL CFmiCrossSectionDlg::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	BOOL status = itsView->OnMouseWheel(nFlags, zDelta, pt);
	return CDialog::OnMouseWheel(nFlags, zDelta, pt) && status;
}

void CFmiCrossSectionDlg::MakeCrossSectionModeUpdates(const std::string &reasonForUpdate)
{
    EnableControls();
    itsView->ViewDirty(true);
    Invalidate(FALSE);
    if(itsSmartMetDocumentInterface->CrossSectionSystem()->CrossSectionSystemActive())
    { 
        // jos karttanäytössä crosssection moodi päällä, päivitetään kartta ja muutkin näytöt
        itsSmartMetDocumentInterface->MacroParamDataCache().clearView(CtrlViewUtils::kFmiCrossSectionView);
        itsSmartMetDocumentInterface->MapViewDirty(CtrlViewUtils::kDoAllMapViewDescTopIndex, false, false, true, false, false, false); // laitetaan viela kaikki ajat likaisiksi cachesta
        itsSmartMetDocumentInterface->RefreshApplicationViewsAndDialogs(reasonForUpdate, SmartMetViewId::MainMapView | SmartMetViewId::CrossSectionView);
    }
}

void CFmiCrossSectionDlg::OnBnClickedCheckUseTimeCrossSection()
{
	UpdateData(TRUE);
    itsSmartMetDocumentInterface->CrossSectionSystem()->UseTimeCrossSection(fUseTimeCrossSection == TRUE);
    MakeCrossSectionModeUpdates("CrossSectionDlg: Toggle time cross section mode");
}

void CFmiCrossSectionDlg::OnBnClickedCheckUseRouteCrossSection()
{
	UpdateData(TRUE);
	itsSmartMetDocumentInterface->CrossSectionSystem()->UseRouteCrossSection(fUseRouteCrossSection == TRUE);
    MakeCrossSectionModeUpdates("CrossSectionDlg: Toggle route cross section mode");
}

void CFmiCrossSectionDlg::OnBnClickedCheckUseObsandforCrossSection()
{
	UpdateData(TRUE);
	itsSmartMetDocumentInterface->CrossSectionSystem()->UseObsAndForCrossSection(fUseObsAndForCrossSection == TRUE);
    MakeCrossSectionModeUpdates("CrossSectionDlg: Toggle obs-model cross section mode");
}

void CFmiCrossSectionDlg::OnBnClickedCheckUseCrosssectionMapMode()
{
	UpdateData(TRUE);
    itsSmartMetDocumentInterface->CrossSectionSystem()->CrossSectionSystemActive(fUseCrossSectionMapMode == TRUE);
    itsSmartMetDocumentInterface->MapViewDirty(CtrlViewUtils::kDoAllMapViewDescTopIndex, false, false, true, false, false, false);
    itsSmartMetDocumentInterface->RefreshApplicationViewsAndDialogs("CrossSectionDlg: Toggle cross section map mode", SmartMetViewId::MainMapView | SmartMetViewId::CrossSectionView);
}

// map modea on muutettu muualla, nyt pitää päivittää ruksin tilaa
void CFmiCrossSectionDlg::UpdateMapMode(void)
{
	fUseCrossSectionMapMode = itsSmartMetDocumentInterface->CrossSectionSystem()->CrossSectionSystemActive();
	UpdateData(FALSE);
}

void CFmiCrossSectionDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	UpdateData(TRUE);
    itsSmartMetDocumentInterface->CrossSectionSystem()->RowCount(itsViewCountSlider.GetPos());
	itsView->Update(true);
	Invalidate(FALSE);

	CDialog::OnHScroll(nSBCode, nPos, pScrollBar);
}

void CFmiCrossSectionDlg::OnNMReleasedcaptureSliderCrossSectionViewCount(NMHDR *pNMHDR, LRESULT *pResult)
{
	// Kikka vitonen: pitää laittaa fokus jollekin toiselle kontrollille, koska muuten hiiren rulla jää slideriin
	CWnd *win = this->GetDlgItem(IDC_BUTTON_CROSS_SECTION_PARAMETER_SELECTION);
	if(win)
		win->SetFocus();
	*pResult = 0;
}

void CFmiCrossSectionDlg::OnBnClickedCheckUseCrosssection3PointMode2()
{
	UpdateData(TRUE);
    itsSmartMetDocumentInterface->CrossSectionSystem()->CrossSectionMode(fUse3PointMode ? NFmiCrossSectionSystem::k3Point : NFmiCrossSectionSystem::k2Point);
    itsSmartMetDocumentInterface->MapViewDirty(CtrlViewUtils::kDoAllMapViewDescTopIndex, false, false, true, false, false, false);
    itsSmartMetDocumentInterface->RefreshApplicationViewsAndDialogs("CrossSectionDlg: Toggle 2/3-point cross section mode", SmartMetViewId::MainMapView | SmartMetViewId::CrossSectionView);
}

void CFmiCrossSectionDlg::OnClose()
{
	DoWhenClosing();
	CDialog::OnClose();
}

void CFmiCrossSectionDlg::DoWhenClosing(void)
{
    auto crossSectionSystem = itsSmartMetDocumentInterface->CrossSectionSystem();
    crossSectionSystem->CrossSectionViewOn(false);
	if(crossSectionSystem->CrossSectionSystemActive())
	{ // jos karttanäytöllä oli poikkileikkaus moodi päällä, laita se pois ja vielä pitää päivittää myös karttanäyttö, että pallukat saadaaan pois
        crossSectionSystem->CrossSectionSystemActive(false);
        itsSmartMetDocumentInterface->MapViewDirty(CtrlViewUtils::kDoAllMapViewDescTopIndex, false, false, true, false, false, false);
        itsSmartMetDocumentInterface->RefreshApplicationViewsAndDialogs("CrossSectionDlg: Close view", SmartMetViewId::AllMapViews);
	}
	AfxGetMainWnd()->SetActiveWindow(); // aktivoidaan karttanäyttö eli mainframe
}

CBitmap* CFmiCrossSectionDlg::MemoryBitmap(void)
{
	if(itsView)
		return itsView->MemoryBitmap();
	return 0;
}

void CFmiCrossSectionDlg::SetDefaultValues(void)
{
    MoveWindow(CFmiCrossSectionDlg::ViewPosRegistryInfo().DefaultWindowRect());
    Persist2::WriteWindowRectToWinRegistry(itsSmartMetDocumentInterface->ApplicationWinRegistry(), MakeUsedWinRegistryKeyStr(0), this);
}

void CFmiCrossSectionDlg::OnCancel()
{
	DoWhenClosing();

	CDialog::OnCancel();
}

void CFmiCrossSectionDlg::OnOK()
{
	DoWhenClosing();

	CDialog::OnOK();
}

// Tämä funktio alustaa kaikki dialogin tekstit editoriin valitulla kielellä.
// Tämä on ikävä kyllä tehtävä erikseen dialogin muokkaus työkalusta, eli
// tekijän pitää lisätä erikseen tänne kaikki dialogin osat, joihin
// kieli valinta voi vaikuttaa.
void CFmiCrossSectionDlg::InitDialogTexts(void)
{
    SetWindowText(CA2T(::GetDictionaryString("CrossSectionDlgTitle").c_str()));
	CFmiWin32Helpers::SetDialogItemText(this, IDC_BUTTON_CROSS_SECTION_PARAMETER_SELECTION, "+");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_BUTTON_PRINT, "IDC_BUTTON_PRINT");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_CHECK_USE_TIME_CROSS_SECTION, "IDC_CHECK_USE_TIME_CROSS_SECTION");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_CHECK_USE_CROSSSECTION_MAP_MODE, "IDC_CHECK_USE_CROSSSECTION_MAP_MODE");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_CHECK_USE_CROSSSECTION_3_POINT_MODE, "IDC_CHECK_USE_CROSSSECTION_3_POINT_MODE");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_CROSS_SECTION_VIEW_COUNT_STR, "IDC_STATIC_CROSS_SECTION_VIEW_COUNT_STR");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_CHECK_USE_ROUTE_CROSS_SECTION, "IDC_CHECK_USE_ROUTE_CROSS_SECTION");
}

void CFmiCrossSectionDlg::EnableControls(void)
{
	CWnd *winTime = GetDlgItem(IDC_CHECK_USE_TIME_CROSS_SECTION);
	CWnd *winRoute = GetDlgItem(IDC_CHECK_USE_ROUTE_CROSS_SECTION);
	CWnd *winObsAndFor = GetDlgItem(IDC_CHECK_USE_OBSANDFOR_CROSS_SECTION);


	if(fUseObsAndForCrossSection)
	{
		winTime->EnableWindow(false);
		winRoute->EnableWindow(false);
		winObsAndFor->EnableWindow(true);
	}
	else if(fUseRouteCrossSection)
	{
		winTime->EnableWindow(false);
		winRoute->EnableWindow(true);
		winObsAndFor->EnableWindow(false);
	}
	else if(fUseTimeCrossSection)
	{
		winTime->EnableWindow(true);
		winRoute->EnableWindow(false);
		winObsAndFor->EnableWindow(false);
	}
	else
	{
		winTime->EnableWindow(true);
		winRoute->EnableWindow(true);
		winObsAndFor->EnableWindow(true);
	}

	// trajektori moodissa disabloidaan muutama kontrolli, koska  trajektori moodi overridaan ne toiminnot
	bool showTrajectories = itsSmartMetDocumentInterface->TrajectorySystem()->ShowTrajectoriesInCrossSectionView();
	if(showTrajectories)
	{
		winTime->EnableWindow(false);
		winRoute->EnableWindow(false);
		winObsAndFor->EnableWindow(false);
	}
	CWnd *win = GetDlgItem(IDC_CHECK_USE_CROSSSECTION_3_POINT_MODE);
	if(win)
		win->EnableWindow(!showTrajectories);
}

void CFmiCrossSectionDlg::OnBnClickedCheckCrosssectionShowHybridLevels()
{
	UpdateData(TRUE);
    itsSmartMetDocumentInterface->CrossSectionSystem()->ShowHybridLevels(fShowHybridLevels == TRUE);
	itsView->Update(true);
	Invalidate(FALSE);
}

void CFmiCrossSectionDlg::OnBnClickedCheckCrosssectionShowTrajectories()
{
	UpdateData(TRUE);
    itsSmartMetDocumentInterface->TrajectorySystem()->ShowTrajectoriesInCrossSectionView(fShowTrajectories == TRUE);
    itsSmartMetDocumentInterface->MapViewDirty(CtrlViewUtils::kDoAllMapViewDescTopIndex, false, false, true, false, false, false);
	EnableControls();
	itsView->Update(true);
	Invalidate(FALSE);
    itsSmartMetDocumentInterface->RefreshApplicationViewsAndDialogs("CrossSectionDlg: Toggle show trajectories cross section mode", SmartMetViewId::MainMapView | SmartMetViewId::CrossSectionView);
}

void CFmiCrossSectionDlg::OnAcceleratorToggleTooltip()
{
    itsSmartMetDocumentInterface->CrossSectionSystem()->ShowTooltipOnCrossSectionView(!itsSmartMetDocumentInterface->CrossSectionSystem()->ShowTooltipOnCrossSectionView());
}

BOOL CFmiCrossSectionDlg::PreTranslateMessage(MSG* pMsg)
{
    if (WM_KEYFIRST <= pMsg->message &&
        pMsg->message <= WM_KEYLAST)
    {
        HACCEL hAccel = m_hAccel;
        if (hAccel &&
            ::TranslateAccelerator(m_hWnd, hAccel, pMsg))
            return TRUE;
    }
    return CDialog::PreTranslateMessage(pMsg);
}

void CFmiCrossSectionDlg::OnChangeParamWindowPositionForward()
{
	itsSmartMetDocumentInterface->OnChangeParamWindowPosition(itsView->MapViewDescTopIndex(), true);
	itsView->Update(true);
	Invalidate(FALSE);
}

void CFmiCrossSectionDlg::OnChangeParamWindowPositionBackward()
{
	itsSmartMetDocumentInterface->OnChangeParamWindowPosition(itsView->MapViewDescTopIndex(), false);
	itsView->Update(true);
	Invalidate(FALSE);
}

void CFmiCrossSectionDlg::OnAcceleratorSetCrossSectionSpecialAxis()
{
    itsSmartMetDocumentInterface->CrossSectionSystem()->SetCrossSectionSpecialAxisToFirstVisibleView();
	itsView->Update(true);
	Invalidate(FALSE);
}

void CFmiCrossSectionDlg::OnAcceleratorSetCrossSectionDefaultAxis()
{
    itsSmartMetDocumentInterface->CrossSectionSystem()->SetCrossSectionDefaultAxisToFirstVisibleView();
	itsView->Update(true);
	Invalidate(FALSE);
}

void CFmiCrossSectionDlg::OnAcceleratorSetCrossSectionDefaultAxisAll()
{
    itsSmartMetDocumentInterface->CrossSectionSystem()->SetCrossSectionDefaultAxisAll();
	itsView->Update(true);
	Invalidate(FALSE);
}

void CFmiCrossSectionDlg::OnAcceleratorSetCrossSectionSpecialAxisAll()
{
    itsSmartMetDocumentInterface->CrossSectionSystem()->SetCrossSectionSpecialAxisAll();
	itsView->Update(true);
	Invalidate(FALSE);
}

void CFmiCrossSectionDlg::OnAcceleratorSaveCrossSectionDefaultAxis()
{
    itsSmartMetDocumentInterface->CrossSectionSystem()->SaveCrossSectionDefaultAxisValues();
}

void CFmiCrossSectionDlg::OnAcceleratorSaveCrossSectionSpecialAxis()
{
    itsSmartMetDocumentInterface->CrossSectionSystem()->SaveCrossSectionSpecialAxisValues();
}

void CFmiCrossSectionDlg::OnButtonOpenParameterSelection()
{
	itsSmartMetDocumentInterface->ActivateParameterSelectionDlg(itsView->MapViewDescTopIndex());
}

void CFmiCrossSectionDlg::OnAcceleratorLogViewerToolboxdeb()
{
	ApplicationInterface::GetApplicationInterfaceImplementation()->OpenLogViewer();
}


void CFmiCrossSectionDlg::OnAcceleratorExtraMapToggleVirtualTimeMode()
{
	std::string viewName = "Cross-section-view";
	CFmiWin32TemplateHelpers::OnAcceleratorToggleVirtualTimeMode(itsSmartMetDocumentInterface, viewName);
}
