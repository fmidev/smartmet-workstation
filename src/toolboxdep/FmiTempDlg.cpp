#include "stdafx.h"
#include "FmiTempDlg.h"
#include "FmiTempView.h"
#include "SmartMetDocumentInterface.h"
#include "NFmiMTATempSystem.h"
#include "FmiTempSettingsDlg.h"
#include "NFmiDictionaryFunction.h"
#include "FmiTempCodeInsertDlg.h"
#include "FmiWin32TemplateHelpers.h"
#include "NFmiProducerSystem.h"
#include "NFmiInfoOrganizer.h"
#include "FmiRichEditDlg.h"
#include "FmiWin32Helpers.h"
#include "CloneBitmap.h"
#include "NFmiApplicationWinRegistry.h"
#include "CtrlViewFunctions.h"
#include "persist2.h"
#include "ApplicationInterface.h"
#include "SoundingViewSettingsFromWindowsRegisty.h"
#include "NFmiValueString.h"

/*
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
*/
/////////////////////////////////////////////////////////////////////////////
// CFmiTempDlg dialog

const NFmiViewPosRegistryInfo CFmiTempDlg::s_ViewPosRegistryInfo(CRect(300, 200, 800, 900), "\\TempView");

IMPLEMENT_DYNAMIC(CFmiTempDlg, CDialog)
CFmiTempDlg::CFmiTempDlg(SmartMetDocumentInterface *smartMetDocumentInterface, CWnd* pParent /*=NULL*/)
	: CDialog(CFmiTempDlg::IDD, pParent)
	,itsSmartMetDocumentInterface(smartMetDocumentInterface)
	,itsTooltipCtrl()
	,itsView(0)
	,fSkewTModeOn(FALSE)
	,fShowStabilityIndexSideView(FALSE)
	,fShowHodograf(FALSE)
	,itsProducerListWithData()
	,fProducerSelectorUsedYet(false)
	,fShowMapMarkers(FALSE)
	,m_hAccel(0)
	, fShowTextualSoundingDataSideView(FALSE)
	, itsModelRunCount(0)
    , fSoundingTimeLockWithMapView(FALSE)
    , fSoundingTextUpward(FALSE)
    , fShowSecondaryDataView(FALSE)
	, itsAvgRangeInKmStr(_T("0"))
	, itsAvgTimeRange1Str(_T("0"))
	, itsAvgTimeRange2Str(_T("0"))
{
	//{{AFX_DATA_INIT(CFmiTempDlg)
	//}}AFX_DATA_INIT
}

CFmiTempDlg::~CFmiTempDlg(void)
{
}

void CFmiTempDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CFmiTempDlg)
	DDX_Control(pDX, IDC_BUTTON_PRINT, itsPrintButtom);
	DDX_Check(pDX, IDC_CHECK_TEMP_SKEWT_MODE, fSkewTModeOn);
	DDX_Check(pDX, IDC_CHECK_SHOW_STABILITY_INDEXIES_SIDE_VIEW, fShowStabilityIndexSideView);
	DDX_Check(pDX, IDC_CHECK_SHOW_HODOGRAF, fShowHodograf);
	//}}AFX_DATA_MAP
	DDX_Check(pDX, IDC_CHECK_TEMP_SHOW_MAP_MARKERS, fShowMapMarkers);
	DDX_Control(pDX, IDC_COMBO_TEMP_PRODUCER_MULTI_SELECTOR, itsMultiProducerSelector);
	DDX_Check(pDX, IDC_CHECK_SHOW_TEXTUAL_SOUNDING_DATA_SIDE_VIEW, fShowTextualSoundingDataSideView);
	DDX_Text(pDX, IDC_EDIT_MODEL_RUN_COUNT, itsModelRunCount);
	//	DDV_MinMaxInt(pDX, itsModelRunCount, 0, 10);
	DDX_Control(pDX, IDC_SPIN_MODEL_RUN_COUNT, itsModelRunSpinner);
	DDX_Check(pDX, IDC_CHECK_USE_MAP_TIME_WITH_SOUNDINGS, fSoundingTimeLockWithMapView);
	DDX_Check(pDX, IDC_CHECK_PUT_SOUNDING_TEXTS_UPWARD, fSoundingTextUpward);
	DDX_Check(pDX, IDC_CHECK_SHOW_SECONDARY_DATA_VIEW, fShowSecondaryDataView);
	DDX_Text(pDX, IDC_EDIT_AVG_RANGE_IN_KM, itsAvgRangeInKmStr);
	DDX_Text(pDX, IDC_EDIT_AVG_TIME_RANGE_1, itsAvgTimeRange1Str);
	DDX_Text(pDX, IDC_EDIT_AVG_TIME_RANGE_2, itsAvgTimeRange2Str);
}

BEGIN_MESSAGE_MAP(CFmiTempDlg, CDialog)
	//{{AFX_MSG_MAP(CFmiTempDlg)
	ON_WM_SIZE()
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_BUTTON_PRINT, OnButtonPrint)
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	//}}AFX_MSG_MAP
	ON_WM_CLOSE()

	ON_BN_CLICKED(IDC_CHECK_TEMP_SHOW_MAP_MARKERS, OnBnClickedShowMapMarkers)
	ON_BN_CLICKED(IDC_CHECK_TEMP_SKEWT_MODE, OnBnClickedCheckTempSkewtMode)
	ON_BN_CLICKED(IDC_BUTTON_SETTINGS, OnBnClickedButtonSettings)
	ON_BN_CLICKED(IDC_CHECK_SHOW_STABILITY_INDEXIES_SIDE_VIEW, OnBnClickedCheckShowStabilityIndexiesSideView)
	ON_BN_CLICKED(IDC_CHECK_SHOW_HODOGRAF, OnBnClickedCheckShowHodograf)
	ON_BN_CLICKED(IDC_BUTTON_SHOW_TXT_SOUNDING_DATA, OnBnClickedShowTxtSoundingData)
	ON_WM_MOUSEWHEEL()
	ON_BN_CLICKED(IDC_BUTTON_RESET_SCALES, OnBnClickedButtonResetScales)
	ON_BN_CLICKED(INSERT_TEMP_CODE, &CFmiTempDlg::OnBnClickedTempCode)
	ON_BN_CLICKED(IDC_BUTTON_RESET_SOUNDING_DATA, OnBnClickedButtonResetSoundingData)
	ON_CBN_SELCHANGE(IDC_COMBO_TEMP_PRODUCER_MULTI_SELECTOR, OnCbnSelchangeComboProducerSelection)
	ON_CBN_CLOSEUP(IDC_COMBO_TEMP_PRODUCER_MULTI_SELECTOR, OnCbnCloseUp)

	ON_BN_CLICKED(IDC_CHECK_SHOW_TEXTUAL_SOUNDING_DATA_SIDE_VIEW, &CFmiTempDlg::OnBnClickedCheckShowTextualSoundingDataSideView)
	ON_EN_CHANGE(IDC_EDIT_MODEL_RUN_COUNT, &CFmiTempDlg::OnEnChangeEditModelRunCount)
    ON_BN_CLICKED(IDC_CHECK_USE_MAP_TIME_WITH_SOUNDINGS, &CFmiTempDlg::OnBnClickedCheckUseMapTimeWithSoundings)
    ON_BN_CLICKED(IDC_CHECK_PUT_SOUNDING_TEXTS_UPWARD, &CFmiTempDlg::OnBnClickedCheckPutSoundingTextsUpward)
    ON_BN_CLICKED(IDC_CHECK_SHOW_SECONDARY_DATA_VIEW, &CFmiTempDlg::OnBnClickedCheckShowSecondaryDataView)
	ON_WM_GETMINMAXINFO()
	ON_COMMAND(ID_ACCELERATOR_SOUNDING_PRODUCER_1, &CFmiTempDlg::OnAcceleratorSoundingProducer1)
	ON_COMMAND(ID_ACCELERATOR_SOUNDING_PRODUCER_2, &CFmiTempDlg::OnAcceleratorSoundingProducer2)
	ON_COMMAND(ID_ACCELERATOR_SOUNDING_PRODUCER_3, &CFmiTempDlg::OnAcceleratorSoundingProducer3)
	ON_COMMAND(ID_ACCELERATOR_SOUNDING_PRODUCER_4, &CFmiTempDlg::OnAcceleratorSoundingProducer4)
	ON_COMMAND(ID_ACCELERATOR_SOUNDING_PRODUCER_5, &CFmiTempDlg::OnAcceleratorSoundingProducer5)
	ON_COMMAND(ID_ACCELERATOR_SOUNDING_PRODUCER_6, &CFmiTempDlg::OnAcceleratorSoundingProducer6)
	ON_COMMAND(ID_ACCELERATOR_SOUNDING_PRODUCER_7, &CFmiTempDlg::OnAcceleratorSoundingProducer7)
	ON_COMMAND(ID_ACCELERATOR_SOUNDING_PRODUCER_8, &CFmiTempDlg::OnAcceleratorSoundingProducer8)
	ON_COMMAND(ID_ACCELERATOR_SOUNDING_PRODUCER_9, &CFmiTempDlg::OnAcceleratorSoundingProducer9)
	ON_COMMAND(ID_ACCELERATOR_SOUNDING_PRODUCER_10, &CFmiTempDlg::OnAcceleratorSoundingProducer10)
	ON_COMMAND(ID_ACCELERATOR_SOUNDING_OPEN_LOG_VIEWER, &CFmiTempDlg::OnAcceleratorSoundingOpenLogViewer)
	ON_COMMAND(ID_ACCELERATOR_SOUNDING_SPACE_OUT_WINDS, &CFmiTempDlg::OnAcceleratorSoundingSpaceOutWinds)
	ON_COMMAND(ID_ACCELERATOR_SOUNDING_TOGGLE_SIDE_VIEW_2, &CFmiTempDlg::OnAcceleratorSoundingToggleSideView2)
	ON_COMMAND(ID_ACCELERATOR_SOUNDING_TOGGLE_STABILITY_INDEX_VIEW, &CFmiTempDlg::OnAcceleratorSoundingToggleStabilityIndexView)
	ON_COMMAND(ID_ACCELERATOR_SOUNDING_TOGGLE_TEXT_VIEW, &CFmiTempDlg::OnAcceleratorSoundingToggleTextView)
	ON_COMMAND(ID_ACCELERATOR_SOUNDING_TOGGLE_TOOLTIP, &CFmiTempDlg::OnAcceleratorSoundingToggleTooltip)
	ON_EN_CHANGE(IDC_EDIT_AVG_RANGE_IN_KM, &CFmiTempDlg::OnEnChangeEditAvgRangeInKm)
	ON_EN_CHANGE(IDC_EDIT_AVG_TIME_RANGE_1, &CFmiTempDlg::OnEnChangeEditAvgTimeRange1)
	ON_EN_CHANGE(IDC_EDIT_AVG_TIME_RANGE_2, &CFmiTempDlg::OnEnChangeEditAvgTimeRange2)
	ON_BN_CLICKED(IDC_BUTTON_CLEAR_AVG_CONTROLS, &CFmiTempDlg::OnBnClickedButtonClearAvgControls)
	ON_COMMAND(ID_ACCELERATOR_SOUNDING_AUTO_ADJUST_SCALES, &CFmiTempDlg::OnAcceleratorSoundingAutoAdjustScales)
	ON_COMMAND(ID_ACCELERATOR_SOUNDING_TOGGLE_VIRTUAL_TIME_MODE, &CFmiTempDlg::OnAcceleratorSoundingToggleVirtualTimeMode)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFmiTempDlg message handlers

BOOL CFmiTempDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

    // Load accelerators, piti luoda oma IDR_ACCELERATOR_SOUNDING_VIEW pikanäppäinlista, koska
	// muuten yleisestä listasta tulee pikanäppäimet 1-9,0, jotka estävät edit controlleihin kyseisten numeroiden syötön.
    m_hAccel = ::LoadAccelerators(AfxGetResourceHandle(), MAKEINTRESOURCE(IDR_ACCELERATOR_SOUNDING_VIEW));
    ASSERT(m_hAccel);

	if(!InitTooltipControl())
		return TRUE;

	std::string errorBaseStr("Error in CFmiTempDlg::OnInitDialog while reading dialog size and position values");
    CFmiWin32TemplateHelpers::DoWindowSizeSettingsFromWinRegistry(itsSmartMetDocumentInterface->ApplicationWinRegistry(), this, false, errorBaseStr, 0);
	CRect rect(CalcClientArea());
	itsView = new CFmiTempView(this, itsSmartMetDocumentInterface);
	itsView->Create(NULL,NULL, WS_VISIBLE | WS_CHILD, rect, this, NULL);
	itsView->OnInitialUpdate(); // pitää kutsua erikseen, koska formvieta ei ole sidottu dokumenttiin
	CFmiWin32Helpers::SetUsedWindowIconDynamically(this);
	SetWindowPos(&wndBottom, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);	 // not possible, thus child stays still upon its parent

	itsModelRunSpinner.SetRange(0, 10);
	InitDialogTexts();
	UpdateProducerList(); // tämä asettaa soundingit oletukseksi päälle tai ainakin yrittää
    UpdateControlsFromDocument();

	UpdateData(FALSE);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CFmiTempDlg::Update(void)
{
    if(IsWindowVisible() && !IsIconic()) // Näyttöä päivitetään vain jos se on näkyvissä ja se ei ole minimized tilassa
    {
        itsView->Update(true);
        UpdateProducerList();
        UpdateControlsFromDocument();

        UpdateData(FALSE);
        Invalidate(FALSE);
    }
}

static CString ConvertIntegrationLimitToCString(double value)
{
	return CString(CA2T(NFmiValueString::GetStringWithMaxDecimalsSmartWay(value, 1)));
}

// Tämän avulla luetaan dokumentista tiettyjen luotausnäyttö kontrollien asetukset ja laitetaan ne
// käyttöön dialogin kontrolleihin. Tätä kutustaan OnInitDialog- ja Update -metodeista.
void CFmiTempDlg::UpdateControlsFromDocument()
{
    auto &mtaTempSystem = itsSmartMetDocumentInterface->GetMTATempSystem();
    fSkewTModeOn = mtaTempSystem.SkewTDegree() == 45 ? TRUE : FALSE;
	auto& soundingSettingsFromWinReg = mtaTempSystem.GetSoundingViewSettingsFromWindowsRegisty();
    fShowStabilityIndexSideView = soundingSettingsFromWinReg.ShowStabilityIndexSideView() ? TRUE : FALSE;
    fShowHodograf = mtaTempSystem.ShowHodograf();
    fShowMapMarkers = mtaTempSystem.ShowMapMarkers();
    itsModelRunSpinner.SetPos(mtaTempSystem.ModelRunCount());
    fShowTextualSoundingDataSideView = soundingSettingsFromWinReg.ShowTextualSoundingDataSideView();
    fSoundingTimeLockWithMapView = soundingSettingsFromWinReg.SoundingTimeLockWithMapView();
    fSoundingTextUpward = soundingSettingsFromWinReg.SoundingTextUpward();
    fShowSecondaryDataView = mtaTempSystem.DrawSecondaryData();
	itsAvgRangeInKmStr = ::ConvertIntegrationLimitToCString(mtaTempSystem.IntegrationRangeInKm());
	itsAvgTimeRange1Str = ::ConvertIntegrationLimitToCString(mtaTempSystem.IntegrationTimeOffset1InHours());
	itsAvgTimeRange2Str = ::ConvertIntegrationLimitToCString(mtaTempSystem.IntegrationTimeOffset2InHours());
	
    UpdateData(FALSE);
}

void CFmiTempDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);

	if(itsView)
		itsView->MoveWindow(CalcClientArea(), FALSE);
	Invalidate(FALSE);
}

// Tämä metodi on sitä varten että voidaan laskea kuinka paljon tilaa dialogin client-arean yläosasta 
// on ns. piirto-osaan. 
int CFmiTempDlg::CalcControlAreaHeight(void)
{
	int areaHeight = 20;
	CWnd* win = GetDescendantWindow(IDC_EDIT_AVG_TIME_RANGE_1);
	if(win)
	{
		WINDOWPLACEMENT wplace;
		BOOL bull = win->GetWindowPlacement(&wplace);
		CRect descRect = wplace.rcNormalPosition;
		areaHeight = descRect.bottom - 1;
	}
	return areaHeight;
}

CRect CFmiTempDlg::CalcClientArea(void)
{
	CRect rect;
	GetClientRect(rect);
	rect.top = rect.top + CalcControlAreaHeight();
	return rect;
}

// tämä alue pitää maalata myös, koska muuten välkkymätön näyttö ei onnistu
CRect CFmiTempDlg::CalcOtherArea(void)
{
	CRect rect;
	GetClientRect(rect);
	rect.bottom = rect.top + CalcControlAreaHeight();
	return rect;
}

void CFmiTempDlg::OnButtonPrint()
{
	itsView->DoPrint();
	SetActiveWindow(); // aktivoidaan vielä tämä ikkuna, koska jostain syystä print-dialogi aktivoi pääikkunan
}

BOOL CFmiTempDlg::OnEraseBkgnd(CDC* pDC)
{
	return FALSE;

//	return CView::OnEraseBkgnd(pDC);
}

void CFmiTempDlg::OnPaint()
{
	CPaintDC dc(this); // device context for painting

 // tämä on pika viritys, kun muuten Print-nappulan kohdalta jää kaista maalaamatta kun laitoin ikkunaan välkkymättömän päivityksen
	CBrush brush(RGB(240, 240, 240));
	CRect area(CalcOtherArea());
	dc.FillRect(&area, &brush);

	// Do not call CDialog::OnPaint() for painting messages
}

CBitmap* CFmiTempDlg::ViewMemoryBitmap(void)
{
	return itsView->MemoryBitmap();
}

void CFmiTempDlg::OnClose()
{
	DoWhenClosing();
	CDialog::OnClose();
}

void CFmiTempDlg::DoWhenClosing(void)
{
    auto &mtaTempSystem = itsSmartMetDocumentInterface->GetMTATempSystem();
    mtaTempSystem.TempViewOn(false);
	fShowMapMarkers = FALSE;
	mtaTempSystem.ShowMapMarkers(false);

	AfxGetMainWnd()->SetActiveWindow(); // aktivoidaan karttanäyttö eli mainframe
	// päivitetään kartta ja muutkin näytöt, koska luotaus asemien kolmioiden kartta piirto riippuu tästä
    itsSmartMetDocumentInterface->MapViewDirty(CtrlViewUtils::kDoAllMapViewDescTopIndex, false, false, true, false, false, false); // laitetaan viela kaikki ajat likaisiksi cachesta
    ApplicationInterface::GetApplicationInterfaceImplementation()->ApplyUpdatedViewsFlag(SmartMetViewId::AllMapViews);
    itsSmartMetDocumentInterface->RefreshApplicationViewsAndDialogs("Sounding view: Closing view", TRUE);
}

void CFmiTempDlg::SetDefaultValues(void)
{
    MoveWindow(CFmiTempDlg::ViewPosRegistryInfo().DefaultWindowRect());
    Persist2::WriteWindowRectToWinRegistry(itsSmartMetDocumentInterface->ApplicationWinRegistry(), MakeUsedWinRegistryKeyStr(0), this);
}

void CFmiTempDlg::OnCancel()
{
	DoWhenClosing();

	CDialog::OnCancel();
}

void CFmiTempDlg::OnOK()
{
	DoWhenClosing();

	CDialog::OnOK();
}

void CFmiTempDlg::OnBnClickedShowMapMarkers()
{
	UpdateData(TRUE);
    itsSmartMetDocumentInterface->GetMTATempSystem().ShowMapMarkers(fShowMapMarkers == TRUE);
	Update(); // tämä laittaa vielä mm. luotausnäyttöluokan cachen likaiseksi
	// Karttanäyttö on piirrettävä uudestaan, koska siellä saattaa näkyä
	// uusia luotaus merkkejä.
    itsSmartMetDocumentInterface->MapViewDirty(CtrlViewUtils::kDoAllMapViewDescTopIndex, false, false, true, false, false, false); // laitetaan viela kaikki ajat likaisiksi cachesta
    ApplicationInterface::GetApplicationInterfaceImplementation()->ApplyUpdatedViewsFlag(SmartMetViewId::AllMapViews);
    itsSmartMetDocumentInterface->RefreshApplicationViewsAndDialogs("Sounding view: Toggle show map markers", TRUE);
}

void CFmiTempDlg::OnBnClickedCheckTempSkewtMode()
{
	UpdateData(TRUE);
    itsSmartMetDocumentInterface->GetMTATempSystem().SkewTDegree(fSkewTModeOn ? 45: 0);
	Update(); // tämä laittaa vielä mm. luotausnäyttöluokan cachen likaiseksi
	Invalidate(FALSE);
}

void CFmiTempDlg::OnBnClickedButtonSettings()
{
	NFmiMTATempSystem tmp(itsSmartMetDocumentInterface->GetMTATempSystem());
	CFmiTempSettingsDlg dlg(&tmp, this);
	if(dlg.DoModal() == IDOK)
	{
        itsSmartMetDocumentInterface->GetMTATempSystem() = tmp; // otetaan muutokset käyttöön
		try
		{
            itsSmartMetDocumentInterface->GetMTATempSystem().StoreSettings();
		}
		catch(std::exception & e )
		{
            ::MessageBox(this->GetSafeHwnd(), CA2T(e.what()), _TEXT("Ongelma luotaus asetuksien talletuksen kanssa!"), MB_OK);
		}
		if(tmp.SkewTDegree() == 45)
			fSkewTModeOn = TRUE;
		else
			fSkewTModeOn = FALSE;
		UpdateData(FALSE);
		Update(); // tämä laittaa vielä mm. luotausnäyttöluokan cachen likaiseksi
		Invalidate(FALSE);
	}
}

void CFmiTempDlg::OnBnClickedCheckShowStabilityIndexiesSideView()
{
	UpdateData(TRUE);
	// Asetus pitää laittaa talteen sekä MTATempSystem:iin että Windows rekisteriin
	itsSmartMetDocumentInterface->GetMTATempSystem().GetSoundingViewSettingsFromWindowsRegisty().ShowStabilityIndexSideView(fShowStabilityIndexSideView == TRUE);
	itsSmartMetDocumentInterface->ApplicationWinRegistry().ShowStabilityIndexSideView(fShowStabilityIndexSideView == TRUE);
	Update(); // tämä laittaa vielä mm. luotausnäyttöluokan cachen likaiseksi
	Invalidate(FALSE);
}

void CFmiTempDlg::OnBnClickedCheckShowHodograf()
{
	UpdateData(TRUE);
    itsSmartMetDocumentInterface->GetMTATempSystem().ShowHodograf(fShowHodograf == TRUE);
	Update(); // tämä laittaa vielä mm. luotausnäyttöluokan cachen likaiseksi
	Invalidate(FALSE);
}

void CFmiTempDlg::OnBnClickedShowTxtSoundingData()
{
	CFmiRichEditDlg dlg("Sounding data in txt", itsView->SoundingIndexStr(), this);
	dlg.DoModal();
}

BOOL CFmiTempDlg::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	BOOL status = itsView->OnMouseWheel(nFlags, zDelta, pt);
	if(status)
		return status;
	else
		return CDialog::OnMouseWheel(nFlags, zDelta, pt);
}

void CFmiTempDlg::OnBnClickedButtonResetScales()
{
	itsView->ResetScales();
	Update(); // tämä laittaa vielä mm. luotausnäyttöluokan cachen likaiseksi
	Invalidate(FALSE);
}

// Tämä funktio alustaa kaikki dialogin tekstit editoriin valitulla kielellä.
// Tämä on ikävä kyllä tehtävä erikseen dialogin muokkaus työkalusta, eli
// tekijän pitää lisätä erikseen tänne kaikki dialogin osat, joihin
// kieli valinta voi vaikuttaa.
void CFmiTempDlg::InitDialogTexts(void)
{
	SetWindowText(CA2T(::GetDictionaryString("TempViewDlgTitle").c_str()));
	CFmiWin32Helpers::SetDialogItemText(this, IDC_BUTTON_PRINT, "Print");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_BUTTON_SETTINGS, "Sett.");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_BUTTON_RESET_SCALES, "<>");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_CHECK_TEMP_SKEWT_MODE, u8"45\xB0");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_CHECK_SHOW_STABILITY_INDEXIES_SIDE_VIEW, "Stab.-ind");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_CHECK_SHOW_HODOGRAF, "Hodograph");

    CFmiWin32Helpers::SetDialogItemText(this, INSERT_TEMP_CODE, "TEMP");
    CFmiWin32Helpers::SetDialogItemText(this, IDC_BUTTON_RESET_SOUNDING_DATA, "R");
    CFmiWin32Helpers::SetDialogItemText(this, IDC_BUTTON_SHOW_TXT_SOUNDING_DATA, "txt");
    CFmiWin32Helpers::SetDialogItemText(this, IDC_CHECK_TEMP_SHOW_MAP_MARKERS, "Map mkr's");
    CFmiWin32Helpers::SetDialogItemText(this, IDC_CHECK_SHOW_TEXTUAL_SOUNDING_DATA_SIDE_VIEW, "Text-data");
    CFmiWin32Helpers::SetDialogItemText(this, IDC_CHECK_SHOW_SECONDARY_DATA_VIEW, "SV2");
    CFmiWin32Helpers::SetDialogItemText(this, IDC_CHECK_USE_MAP_TIME_WITH_SOUNDINGS, "MT");
    CFmiWin32Helpers::SetDialogItemText(this, IDC_CHECK_PUT_SOUNDING_TEXTS_UPWARD, "Up");
    CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_MODEL_RUN_COUNT_STR, "Model run cnt");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_AVG_RANGE_IN_KM_STR, "Range [km]");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_AVG_TIME_RANGE_STR, "Time offsets [h]");
}

void CFmiTempDlg::OnBnClickedTempCode()
{
	CFmiTempCodeInsertDlg dlg(itsSmartMetDocumentInterface, this);
	if(dlg.DoModal() == IDOK)
	{
		// Karttanäyttö on piirrettävä uudestaan, koska siellä saattaa näkyä
		// uusia luotaus merkkejä.
        itsSmartMetDocumentInterface->MapViewDirty(CtrlViewUtils::kDoAllMapViewDescTopIndex, false, false, true, false, false, false); // laitetaan viela kaikki ajat likaisiksi cachesta
        ApplicationInterface::GetApplicationInterfaceImplementation()->ApplyUpdatedViewsFlag(SmartMetViewId::AllMapViews | SmartMetViewId::SoundingView);
        itsSmartMetDocumentInterface->RefreshApplicationViewsAndDialogs("Sounding view: Applying new Temp Code data", TRUE);
	}
}

BOOL CFmiTempDlg::PreTranslateMessage(MSG* pMsg)
{
	// Hoidetaan näppäintenpainalluksien yhteydessä mahdolliset pikanäppäimet.
	// Huom! Luotausnäytölle piti luoda oma accelerator lista, joka ladataan m_hAccel:iin.
	// Koska SmartMetToolboxDep kirjaston yleisessä accelerator listassa on mukana 
	// pikanäppäimet 1-9,0, ja niiden käsittely estäisi editbox:ien käytön luotausnäytössä,
	// koska kyseiset näppäimet 'syötäisiin' tässä.
	if(WM_KEYFIRST <= pMsg->message && pMsg->message <= WM_KEYLAST) // acceleratorien hanskaus
	{
		HACCEL hAccel = m_hAccel;
		if(hAccel && ::TranslateAccelerator(m_hWnd, hAccel, pMsg))
			return TRUE;
	}

	itsTooltipCtrl.RelayEvent(pMsg); // tooltipit pitää forwardoida näin

	return CDialog::PreTranslateMessage(pMsg);
}

BOOL CFmiTempDlg::InitTooltipControl(void)
{
	if(!itsTooltipCtrl.Create(this))
	{
		TRACE("Unable To create ToolTipCtrl in CFmiTempDlg-class\n");
		return FALSE;
	}
	itsTooltipCtrl.SetDelayTime(TTDT_INITIAL, 150); // lyhyt alku aika (150 ms) ennen kuin tootip tulee näkyviin
	itsTooltipCtrl.SetDelayTime(TTDT_AUTOPOP, 30 * 1000); // 30 sekuntia on pisin saika minkä sain tooltipin pysymään näkyvissä (jos suurempi, menee defaultti arvoon 5 sekuntia)
	itsTooltipCtrl.Activate(TRUE);

	InitControlsTooltip(IDC_BUTTON_RESET_SCALES, "Restore default limits to pressure and temperature scales");
	InitControlsTooltip(IDC_BUTTON_RESET_SOUNDING_DATA, "Clear all TEMP data generated from raw temp text messages");
	InitControlsTooltip(IDC_CHECK_SHOW_SECONDARY_DATA_VIEW, "Show/hide secondary data view");
	InitControlsTooltip(IDC_CHECK_USE_MAP_TIME_WITH_SOUNDINGS, "Make selected soundings follow the main map view times");
	InitControlsTooltip(IDC_CHECK_TEMP_SKEWT_MODE, "Set Skew-T mode on/off");
	InitControlsTooltip(IDC_CHECK_PUT_SOUNDING_TEXTS_UPWARD, "Set textual sounding values go upward from the ground (bottom)");
	InitControlsTooltip(IDC_BUTTON_SHOW_TXT_SOUNDING_DATA, "Show textual sounding values in separate view for copy-paste action");
	InitControlsTooltip(IDC_EDIT_AVG_RANGE_IN_KM, "Avg integration range [km] for model soundings");
	InitControlsTooltip(IDC_STATIC_AVG_RANGE_IN_KM_STR, "Avg integration range [km] for model soundings");
	InitControlsTooltip(IDC_EDIT_AVG_TIME_RANGE_1, "Avg integration start time offset [h] for model soundings");
	InitControlsTooltip(IDC_EDIT_AVG_TIME_RANGE_2, "Avg integration end time offset [h] for model soundings");
	InitControlsTooltip(IDC_STATIC_AVG_TIME_RANGE_STR, "Avg integration start+end time offsets [h] for model soundings");
	InitControlsTooltip(IDC_BUTTON_CLEAR_AVG_CONTROLS, "Clear all the Avg functionality related controls");
	InitControlsTooltip(IDC_CHECK_TEMP_SHOW_MAP_MARKERS, "Show/don't show sounding related markers on map views");
	InitControlsTooltip(IDC_EDIT_MODEL_RUN_COUNT, "How many latest model runs shown simultaneously");
	InitControlsTooltip(IDC_SPIN_MODEL_RUN_COUNT, "How many latest model runs shown simultaneously");
	
	return TRUE;
}

void CFmiTempDlg::InitControlsTooltip(int theControlId, const char *theDictionaryStr)
{
	CWnd *win = GetDlgItem(theControlId);
	if(win)
		itsTooltipCtrl.AddTool(win, CA2T(::GetDictionaryString(theDictionaryStr).c_str()));
}

void CFmiTempDlg::OnBnClickedButtonResetSoundingData()
{
	itsView->ResetSoundingData();
	Update(); // tämä laittaa vielä mm. luotausnäyttöluokan cachen likaiseksi
	Invalidate(FALSE);
}

void CFmiTempDlg::SetSelectedProducer(void)
{
	std::vector<bool> selVec = itsMultiProducerSelector.GetSelectionVector();
	std::vector<NFmiProducer> selProducers;
	for(size_t i = 0; i<selVec.size(); i++)
	{
		if(selVec[i])
			selProducers.push_back(itsProducerListWithData[i]);
	}
    itsSmartMetDocumentInterface->GetMTATempSystem().SoundingComparisonProducers(selProducers);
}

// tässä pitää asettaa halutut tuottajat itsMultiProducerSelector-otuksen valinnoiksi
void CFmiTempDlg::SetSelectedProducersFromViewMacro(void)
{
	itsMultiProducerSelector.SelectAll(false);
	const auto &selectedProducers = itsSmartMetDocumentInterface->GetMTATempSystem().SoundingComparisonProducers();
	size_t selectedProducerCount = 0;
	for(size_t i = 0; i < selectedProducers.size(); i++)
	{
		for(size_t j = 0; j < itsProducerListWithData.size(); j++)
		{
			if(selectedProducers[i] == itsProducerListWithData[j])
			{
				itsMultiProducerSelector.SetCheck(static_cast<int>(j), true);
				selectedProducerCount++;
			}
		}
	}

	fProducerSelectorUsedYet = true;
	if(selectedProducers.empty() || selectedProducers.size() == selectedProducerCount)
	{
		// Jos ei ollut yhtään tuottajaa näyttömakrossa valittuna tai kaikki näyttömakron tuottajat löytyivät ja asetettiin päälle
		fAllProducersSetFromViewMacro = true;
	}
	UpdateData(FALSE);
}

void CFmiTempDlg::OnCbnSelchangeComboProducerSelection()
{
	fProducerSelectorUsedYet = true;
	fAllProducersSetFromViewMacro = true; // Jos käyttäjä tekee tuottaja valintoja, niitä ei enää tarvitse päivitellä, jos valinnat jääneet vajaiksi
	SetSelectedProducer();
	Update();
	Invalidate(FALSE);
}

void CFmiTempDlg::OnCbnCloseUp()
{
	// pitää tehdä dropdown listasta ei aktiivinen aina käytön jälkeen, että ei varasta hiiren rullan huomiota
	CWnd *dummy = GetDlgItem(IDC_BUTTON_SETTINGS);
	if(dummy) // yritetään asettaa fokus ominaisuus-napille, niin rulla toimii
		dummy->SetFocus();
}

static bool FindProducer(const std::vector<std::pair<int, CString> > &theSelectionVector, const NFmiString &theProdName)
{
    CString prodNameU_ = CA2T(theProdName.CharPtr());
	for(std::vector<std::pair<int, CString> >::const_iterator it = theSelectionVector.begin(); it != theSelectionVector.end(); ++it)
	{
        if((*it).second == prodNameU_)
			return true;
	}
	return false;
}

// Normaali NFmiProducer yhtäsuuruus testi vertaa vain tuottaja id:t, nyt server datan kanssa tarvitaan myös nimi vertailu
static bool ProducerVectorsAreEqual(const std::vector<NFmiProducer> &producers1, const std::vector<NFmiProducer> &producers2)
{
    if(producers1.size() != producers2.size())
        return false;
    for(size_t producerIndex = 0; producerIndex < producers1.size(); producerIndex++)
    {
        if(!NFmiMTATempSystem::ServerProducer::ProducersAreEqual(producers1[producerIndex], producers2[producerIndex]))
            return false;
    }

    return true;
}

void CFmiTempDlg::UpdateProducerList(void)
{
	std::vector<std::pair<int, CString> > selectionVec = itsMultiProducerSelector.GetSelectedWithStr();

	std::vector<NFmiProducer> tmpProducerListWithData;
	// 1. kysy täysi tuottajalista MTASystemiltä
	const auto &possibleProdList = itsSmartMetDocumentInterface->GetMTATempSystem().PossibleProducerList();
	// 2. käy lista läpi ja katso mille tuottajalle löytyy dataa
	for(const auto possibleProducer : possibleProdList)
	{
		boost::shared_ptr<NFmiFastQueryInfo> info = itsSmartMetDocumentInterface->InfoOrganizer()->FindSoundingInfo(possibleProducer, 0, NFmiInfoOrganizer::ParamCheckFlags(true));
	// 3. laita ne tuottajat, joille löytyi dataa itsProducerListWithData-listaan, tai jos ne ovat serveriltä haettavaa dataa
		if(info || possibleProducer.useServer() || ::FindProducer(selectionVec, possibleProducer.GetName())) // laitetaan sittenkin kaikki tuottajat aina listaan, että datojen reload ei tuhoa listaa (testi)
			tmpProducerListWithData.push_back(possibleProducer);
	}

	if(!::ProducerVectorsAreEqual(tmpProducerListWithData, itsProducerListWithData))
	{ // Päivitetään listoja vain jos lista on muuttunut
		itsProducerListWithData = tmpProducerListWithData;
		// 4. laita tämän listan nimet itsProducerSelector-comboboxiin

		itsMultiProducerSelector.ResetContent();
		for(size_t producerIndex = 0; producerIndex < itsProducerListWithData.size(); producerIndex++)
			itsMultiProducerSelector.AddString(CA2T(itsProducerListWithData[producerIndex].GetName()));
		int ssize = itsMultiProducerSelector.GetCount();

		if(fProducerSelectorUsedYet == false)
		{ // 5. jos käyttäjä ei ole vielä käynyt säätämässä tuottaja listaa, voidaan asettaa oletus tuottajaksi luotaus havainnot (kFmiTEMP)
			if(selectionVec.size() == 0 && ssize > 0)
				itsMultiProducerSelector.SetCheck(0, TRUE); // varmistetaan että jos on ollut yhtään tuottajaa listassa, ainakin se valitaan defaultiksi
			for(int i=0; i<static_cast<int>(itsProducerListWithData.size()); i++)
			{
                if(NFmiInfoOrganizer::IsTempData(itsProducerListWithData[i].GetIdent()))
				{
					itsMultiProducerSelector.SelectAll(FALSE);
					itsMultiProducerSelector.SetCheck(i, TRUE);
					break;
				}
			}
		}
		else
		{ // Laita viimeksi valitut taas päälle listasta
			for(size_t j = 0; j < selectionVec.size(); j++)
			{
				for(int i=0; i<static_cast<int>(itsProducerListWithData.size()); i++)
				{
					if(itsProducerListWithData[i].GetName() == NFmiString(CT2A(selectionVec[j].second)))
					{
						itsMultiProducerSelector.SetCheck(i, TRUE);
					}
				}
			}
		}

        // If not loading viewMacro, put selected producers back to MTATempSystem
        if(!itsSmartMetDocumentInterface->GetMTATempSystem().UpdateFromViewMacro() && fAllProducersSetFromViewMacro)
            SetSelectedProducer();
	}

	// jos ollaan lataamassa näyttömakroa, pitää itsMultiProducerSelector-otus päivittää
	if(itsSmartMetDocumentInterface->GetMTATempSystem().UpdateFromViewMacro())
	{
		fAllProducersSetFromViewMacro = false; // Resetoidaan tämä flagi ennen kuin aletaan lataamaan tuottajia näyttömakrosta
		SetSelectedProducersFromViewMacro();
        itsSmartMetDocumentInterface->GetMTATempSystem().UpdateFromViewMacro(false);
	}
	else
	{
		DoPossibleViewMacroProducersUpdates();
	}
}

void CFmiTempDlg::DoPossibleViewMacroProducersUpdates()
{
	if(!fAllProducersSetFromViewMacro)
	{
		SetSelectedProducersFromViewMacro();
	}
}

void CFmiTempDlg::SelectProducer(int producerIndex)
{
	if(producerIndex >= 0 && producerIndex < itsMultiProducerSelector.GetCount())
	{
		itsMultiProducerSelector.SetCheck(producerIndex, !itsMultiProducerSelector.GetCheck(producerIndex));
		SetSelectedProducer();
		Update();
		Invalidate(FALSE);
	}
}

void CFmiTempDlg::OnBnClickedCheckShowTextualSoundingDataSideView()
{
	UpdateData(TRUE);
	// Asetus pitää laittaa talteen sekä MTATempSystem:iin että Windows rekisteriin
	itsSmartMetDocumentInterface->GetMTATempSystem().GetSoundingViewSettingsFromWindowsRegisty().ShowTextualSoundingDataSideView(fShowTextualSoundingDataSideView == TRUE);
	itsSmartMetDocumentInterface->ApplicationWinRegistry().ShowTextualSoundingDataSideView(fShowTextualSoundingDataSideView == TRUE);
	Update();
	Invalidate(FALSE);
}

void CFmiTempDlg::OnEnChangeEditModelRunCount()
{
	static bool firstTime = true;
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// TODO:  Add your control notification handler code here
	if(firstTime == false)
	{
		UpdateData(TRUE);
        itsSmartMetDocumentInterface->GetMTATempSystem().ModelRunCount(itsModelRunCount);
		itsView->Update(true);
		Invalidate(FALSE);
	}
	else
		firstTime = false;
}

void CFmiTempDlg::OnBnClickedCheckUseMapTimeWithSoundings()
{
	UpdateData(TRUE);
	// Asetus pitää laittaa talteen sekä MTATempSystem:iin että Windows rekisteriin
	itsSmartMetDocumentInterface->ApplicationWinRegistry().SoundingTimeLockWithMapView(fSoundingTimeLockWithMapView == TRUE);
    itsSmartMetDocumentInterface->GetMTATempSystem().GetSoundingViewSettingsFromWindowsRegisty().SoundingTimeLockWithMapView(itsSmartMetDocumentInterface->ApplicationWinRegistry().SoundingTimeLockWithMapView()); 
	Update();
    itsSmartMetDocumentInterface->MapViewDirty(CtrlViewUtils::kDoAllMapViewDescTopIndex, false, false, true, false, false, false); // laitetaan viela kaikki ajat likaisiksi cachesta
    ApplicationInterface::GetApplicationInterfaceImplementation()->ApplyUpdatedViewsFlag(SmartMetViewId::AllMapViews | SmartMetViewId::SoundingView);
    itsSmartMetDocumentInterface->RefreshApplicationViewsAndDialogs("Sounding view: Toggle Use map time");
}

void CFmiTempDlg::OnBnClickedCheckPutSoundingTextsUpward()
{
	UpdateData(TRUE);
	// Asetus pitää laittaa talteen sekä MTATempSystem:iin että Windows rekisteriin
	itsSmartMetDocumentInterface->ApplicationWinRegistry().SoundingTextUpward(fSoundingTextUpward == TRUE);
    itsSmartMetDocumentInterface->GetMTATempSystem().GetSoundingViewSettingsFromWindowsRegisty().SoundingTextUpward(itsSmartMetDocumentInterface->ApplicationWinRegistry().SoundingTextUpward()); // Asetus pitää laittaa talteen myös MTATempSystem -luokalle!
    Update();
	Invalidate(FALSE);
}

void CFmiTempDlg::OnBnClickedCheckShowSecondaryDataView()
{
    UpdateData(TRUE);
    itsSmartMetDocumentInterface->GetMTATempSystem().DrawSecondaryData(fShowSecondaryDataView == TRUE);
    Update();
    Invalidate(FALSE);
}

void CFmiTempDlg::OnGetMinMaxInfo(MINMAXINFO* lpMMI)
{
	// set the minimum tracking width and height of the window
	lpMMI->ptMinTrackSize.x = 500;
	lpMMI->ptMinTrackSize.y = 500;
}

// CTRL + 1 toglaa 1. tuottajan listalta päälle/pois
void CFmiTempDlg::OnAcceleratorSoundingProducer1()
{
	SelectProducer(0);
}

// CTRL + 2 toglaa 2. tuottajan listalta päälle/pois
void CFmiTempDlg::OnAcceleratorSoundingProducer2()
{
	SelectProducer(1);
}

// CTRL + 3 toglaa 3. tuottajan listalta päälle/pois
void CFmiTempDlg::OnAcceleratorSoundingProducer3()
{
	SelectProducer(2);
}

// CTRL + 4 toglaa 4. tuottajan listalta päälle/pois
void CFmiTempDlg::OnAcceleratorSoundingProducer4()
{
	SelectProducer(3);
}

// CTRL + 5 toglaa 5. tuottajan listalta päälle/pois
void CFmiTempDlg::OnAcceleratorSoundingProducer5()
{
	SelectProducer(4);
}

// CTRL + 6 toglaa 6. tuottajan listalta päälle/pois
void CFmiTempDlg::OnAcceleratorSoundingProducer6()
{
	SelectProducer(5);
}

// CTRL + 7 toglaa 7. tuottajan listalta päälle/pois
void CFmiTempDlg::OnAcceleratorSoundingProducer7()
{
	SelectProducer(6);
}

// CTRL + 8 toglaa 8. tuottajan listalta päälle/pois
void CFmiTempDlg::OnAcceleratorSoundingProducer8()
{
	SelectProducer(7);
}

// CTRL + 9 toglaa 9. tuottajan listalta päälle/pois
void CFmiTempDlg::OnAcceleratorSoundingProducer9()
{
	SelectProducer(8);
}

// CTRL + 0 toglaa 10. tuottajan listalta päälle/pois
void CFmiTempDlg::OnAcceleratorSoundingProducer10()
{
	SelectProducer(9);
}

// CTRL + L avaa siis log-viewerin
void CFmiTempDlg::OnAcceleratorSoundingOpenLogViewer()
{
	ApplicationInterface::GetApplicationInterfaceImplementation()->OpenLogViewer();
}

// CTRL + D muuttaa tuuliviirien harvennusta
void CFmiTempDlg::OnAcceleratorSoundingSpaceOutWinds()
{
	itsSmartMetDocumentInterface->SoundingViewWindBarbSpaceOutFactor(itsSmartMetDocumentInterface->SoundingViewWindBarbSpaceOutFactor() + 1);
	Update();
	Invalidate(FALSE);
}


// Tämä CTRL + F accelerator siis laittaa luotausnäytön apudata (SV2) osion päälle/pois
void CFmiTempDlg::OnAcceleratorSoundingToggleSideView2()
{
	itsSmartMetDocumentInterface->GetMTATempSystem().DrawSecondaryData(!itsSmartMetDocumentInterface->GetMTATempSystem().DrawSecondaryData());
	fShowSecondaryDataView = itsSmartMetDocumentInterface->GetMTATempSystem().DrawSecondaryData();
	Update();
	Invalidate(FALSE);
}


void CFmiTempDlg::OnAcceleratorSoundingToggleStabilityIndexView()
{
	fShowStabilityIndexSideView = !fShowStabilityIndexSideView;
	UpdateData(FALSE);
	OnBnClickedCheckShowStabilityIndexiesSideView();
}

// CTRL + T toglaa tekstisivunäytön päälle/pois
void CFmiTempDlg::OnAcceleratorSoundingToggleTextView()
{
	fShowTextualSoundingDataSideView = !fShowTextualSoundingDataSideView;
	UpdateData(FALSE);
	OnBnClickedCheckShowTextualSoundingDataSideView();
}

// F11 toglaa tooltipin päälle/pois
void CFmiTempDlg::OnAcceleratorSoundingToggleTooltip()
{
	itsSmartMetDocumentInterface->ShowToolTipTempView(!itsSmartMetDocumentInterface->ShowToolTipTempView());
}

static std::pair<double, bool> SecureCString2Double(CString& valueStr)
{
	std::string valueStdStr;
	try
	{
		valueStdStr = (CT2A(valueStr));

		std::size_t pos;
		auto value = std::stod(valueStdStr, &pos);
		if(pos >= valueStdStr.size())
		{
			return std::make_pair(value, true);
		}
	}
	catch(...)
	{
	}

	// Jos käyttäjä antanut tyhjää, '-' (miinusmerkin), '.' (desimaalipiste) tai space:n alkuun, 
	// tehdään luvusta 0, mutta tehdään inputista sallittu eli true paluuarvon second:iin
	if(valueStdStr.empty() || valueStdStr.front() == '-' || valueStdStr.front() == '.' || valueStdStr.front() == ' ')
		return std::make_pair(0, true);

	// Jos käyttäjä on sotkenut integraatio rajojen arvot roskalla, laitetaan arvot aina takaisin 0:ksi.
	return std::make_pair(0, false);
}

void CFmiTempDlg::OnEnChangeEditAvgRangeInKm()
{
	UpdateData(TRUE);
	auto secureValue = ::SecureCString2Double(itsAvgRangeInKmStr);
	itsSmartMetDocumentInterface->GetMTATempSystem().IntegrationRangeInKm(secureValue.first);
	auto updatedValue = itsSmartMetDocumentInterface->GetMTATempSystem().IntegrationRangeInKm();
	if(secureValue.second == false || secureValue.first != updatedValue)
	{
		// Jos syötteessä oli jotain roskaa (secureValue.second == false), päivitetään kontrolli korjatulla arvolla.
		// Tai jos syöte meni ali/yli arvorajojen, päivitetään kontrolli korjatulla arvolla.
		itsAvgRangeInKmStr = ::ConvertIntegrationLimitToCString(updatedValue);
		UpdateData(FALSE);
	}
	itsView->Update(true);
	Invalidate(FALSE);
}


void CFmiTempDlg::OnEnChangeEditAvgTimeRange1()
{
	UpdateData(TRUE);
	auto secureValue = ::SecureCString2Double(itsAvgTimeRange1Str);
	itsSmartMetDocumentInterface->GetMTATempSystem().IntegrationTimeOffset1InHours(secureValue.first);
	auto updatedValue = itsSmartMetDocumentInterface->GetMTATempSystem().IntegrationTimeOffset1InHours();
	if(secureValue.second == false || secureValue.first != updatedValue)
	{
		// Jos syötteessä oli jotain roskaa (secureValue.second == false), päivitetään kontrolli korjatulla arvolla.
		// Tai jos syöte meni ali/yli arvorajojen, päivitetään kontrolli korjatulla arvolla.
		itsAvgTimeRange1Str = ::ConvertIntegrationLimitToCString(updatedValue);
		UpdateData(FALSE);
	}
	itsView->Update(true);
	Invalidate(FALSE);
}


void CFmiTempDlg::OnEnChangeEditAvgTimeRange2()
{
	UpdateData(TRUE);
	auto secureValue = ::SecureCString2Double(itsAvgTimeRange2Str);
	itsSmartMetDocumentInterface->GetMTATempSystem().IntegrationTimeOffset2InHours(secureValue.first);
	auto updatedValue = itsSmartMetDocumentInterface->GetMTATempSystem().IntegrationTimeOffset2InHours();
	if(secureValue.second == false || secureValue.first != updatedValue)
	{
		// Jos syötteessä oli jotain roskaa (secureValue.second == false), päivitetään kontrolli korjatulla arvolla.
		// Tai jos syöte meni ali/yli arvorajojen, päivitetään kontrolli korjatulla arvolla.
		itsAvgTimeRange2Str = ::ConvertIntegrationLimitToCString(updatedValue);
		UpdateData(FALSE);
	}
	itsView->Update(true);
	Invalidate(FALSE);
}

void CFmiTempDlg::OnBnClickedButtonClearAvgControls()
{
	auto& mtaTempSystem = itsSmartMetDocumentInterface->GetMTATempSystem();
	mtaTempSystem.IntegrationRangeInKm(0);
	mtaTempSystem.IntegrationTimeOffset1InHours(0);
	mtaTempSystem.IntegrationTimeOffset2InHours(0);
	itsAvgRangeInKmStr = "0";
	itsAvgTimeRange1Str = "0";
	itsAvgTimeRange2Str = "0";
	UpdateData(FALSE);
	itsView->Update(true);
	Invalidate(FALSE);
}

void CFmiTempDlg::OnAcceleratorSoundingAutoAdjustScales()
{
	itsView->AutoAdjustValueScales(false, false);
	itsView->Update(true);
	Invalidate(FALSE);
}

void CFmiTempDlg::OnAcceleratorSoundingToggleVirtualTimeMode()
{
	std::string viewName = "Sounding-view";
	CFmiWin32TemplateHelpers::OnAcceleratorToggleVirtualTimeMode(itsSmartMetDocumentInterface, viewName);
}
