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
	,fShowIndexies(FALSE)
	,fShowHodograf(FALSE)
	,itsProducerListWithData()
	,fProducerSelectorUsedYet(false)
	,fShowMapMarkers(FALSE)
	,m_hAccel(0)
	, fShowSideView(FALSE)
	, itsModelRunCount(0)
    , fSoundingTimeLockWithMapView(FALSE)
    , fSoundingTextUpward(FALSE)
    , fShowSecondaryDataView(FALSE)
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
    DDX_Check(pDX, IDC_CHECK_SHOW_INDEXIES, fShowIndexies);
    DDX_Check(pDX, IDC_CHECK_SHOW_HODOGRAF, fShowHodograf);
    //}}AFX_DATA_MAP
    DDX_Check(pDX, IDC_CHECK_TEMP_SHOW_MAP_MARKERS, fShowMapMarkers);
    DDX_Control(pDX, IDC_COMBO_TEMP_PRODUCER_MULTI_SELECTOR, itsMultiProducerSelector);
    DDX_Check(pDX, IDC_CHECK_SHOW_SIDE_VIEW, fShowSideView);
    DDX_Text(pDX, IDC_EDIT_MODEL_RUN_COUNT, itsModelRunCount);
    //	DDV_MinMaxInt(pDX, itsModelRunCount, 0, 10);
    DDX_Control(pDX, IDC_SPIN_MODEL_RUN_COUNT, itsModelRunSpinner);
    DDX_Check(pDX, IDC_CHECK_USE_MAP_TIME_WITH_SOUNDINGS, fSoundingTimeLockWithMapView);
    DDX_Check(pDX, IDC_CHECK_PUT_SOUNDING_TEXTS_UPWARD, fSoundingTextUpward);
    DDX_Check(pDX, IDC_CHECK_SHOW_SECONDARY_DATA_VIEW, fShowSecondaryDataView);
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
	ON_BN_CLICKED(IDC_CHECK_SHOW_INDEXIES, OnBnClickedCheckShowIndexies)
	ON_BN_CLICKED(IDC_CHECK_SHOW_HODOGRAF, OnBnClickedCheckShowHodograf)
	ON_BN_CLICKED(IDC_BUTTON_SHOW_TXT_SOUNDING_DATA, OnBnClickedShowTxtSoundingData)
	ON_WM_MOUSEWHEEL()
	ON_BN_CLICKED(IDC_BUTTON_RESET_SCALES, OnBnClickedButtonResetScales)
	ON_BN_CLICKED(INSERT_TEMP_CODE, &CFmiTempDlg::OnBnClickedTempCode)
	ON_BN_CLICKED(IDC_BUTTON_RESET_SOUNDING_DATA, OnBnClickedButtonResetSoundingData)
	ON_CBN_SELCHANGE(IDC_COMBO_TEMP_PRODUCER_MULTI_SELECTOR, OnCbnSelchangeComboProducerSelection)
	ON_CBN_CLOSEUP(IDC_COMBO_TEMP_PRODUCER_MULTI_SELECTOR, OnCbnCloseUp)

	ON_COMMAND(ID_ACCELERATOR_TEMP_PRODUCER1, OnAcceleratorTempProducer1)
	ON_COMMAND(ID_ACCELERATOR_TEMP_PRODUCER2, OnAcceleratorTempProducer2)
	ON_COMMAND(ID_ACCELERATOR_TEMP_PRODUCER3, OnAcceleratorTempProducer3)
	ON_COMMAND(ID_ACCELERATOR_TEMP_PRODUCER4, OnAcceleratorTempProducer4)
	ON_COMMAND(ID_ACCELERATOR_TEMP_PRODUCER5, OnAcceleratorTempProducer5)
	ON_COMMAND(ID_ACCELERATOR_TEMP_PRODUCER6, OnAcceleratorTempProducer6)
	ON_COMMAND(ID_ACCELERATOR_TEMP_PRODUCER7, OnAcceleratorTempProducer7)
	ON_COMMAND(ID_ACCELERATOR_TEMP_PRODUCER8, OnAcceleratorTempProducer8)
	ON_COMMAND(ID_ACCELERATOR_TEMP_PRODUCER9, OnAcceleratorTempProducer9)

	ON_COMMAND(ID_ACCELERATOR_SPACE_OUT_TEMP_WINDS, OnEditSpaceOut)
	ON_BN_CLICKED(IDC_CHECK_SHOW_SIDE_VIEW, &CFmiTempDlg::OnBnClickedCheckShowSideView)
	ON_COMMAND(ID_ACCELERATOR_CLOSE_SIDE_VIEW, &CFmiTempDlg::OnAcceleratorSwapArea)
	ON_COMMAND(ID_ACCELERATOR_TOGGLE_TEMP_VIEW_TOOLTIP, &CFmiTempDlg::OnAcceleratorToggleTooltip)
	ON_EN_CHANGE(IDC_EDIT_MODEL_RUN_COUNT, &CFmiTempDlg::OnEnChangeEditModelRunCount)
    ON_BN_CLICKED(IDC_CHECK_USE_MAP_TIME_WITH_SOUNDINGS, &CFmiTempDlg::OnBnClickedCheckUseMapTimeWithSoundings)
    ON_BN_CLICKED(IDC_CHECK_PUT_SOUNDING_TEXTS_UPWARD, &CFmiTempDlg::OnBnClickedCheckPutSoundingTextsUpward)
    ON_COMMAND(ID_ACCELERATOR_CHANGE_MAP_TYPE_EXTRA_MAP, &CFmiTempDlg::OnAcceleratorChangeMapTypeExtraMap)
    ON_BN_CLICKED(IDC_CHECK_SHOW_SECONDARY_DATA_VIEW, &CFmiTempDlg::OnBnClickedCheckShowSecondaryDataView)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFmiTempDlg message handlers

BOOL CFmiTempDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

    // Load accelerators
    m_hAccel = ::LoadAccelerators(AfxGetResourceHandle(), MAKEINTRESOURCE(IDR_ACCELERATOR1));
    ASSERT(m_hAccel);

	if(!InitTooltipControl())
		return TRUE;

	std::string errorBaseStr("Error in CFmiTempDlg::OnInitDialog while reading dialog size and position values");
    CFmiWin32TemplateHelpers::DoWindowSizeSettingsFromWinRegistry(itsSmartMetDocumentInterface->ApplicationWinRegistry(), this, false, errorBaseStr, 0);
	CRect rect(CalcClientArea());
	itsView = new CFmiTempView(this, itsSmartMetDocumentInterface);
	itsView->Create(NULL,NULL, WS_VISIBLE | WS_CHILD, rect, this, NULL);
	itsView->OnInitialUpdate(); // pitää kutsua erikseen, koska formvieta ei ole sidottu dokumenttiin

	HICON hIcon = CCloneBitmap::BitmapToIcon(FMI_LOGO_BITMAP_2, ColorPOD(160, 160, 164));
	this->SetIcon(hIcon, FALSE);

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

// Tämän avulla luetaan dokumentista tiettyjen luotausnäyttö kontrollien asetukset ja laitetaan ne
// käyttöön dialogin kontrolleihin. Tätä kutustaan OnInitDialog- ja Update -metodeista.
void CFmiTempDlg::UpdateControlsFromDocument()
{
    auto &mtaTempSystem = itsSmartMetDocumentInterface->GetMTATempSystem();
    fSkewTModeOn = mtaTempSystem.SkewTDegree() == 45 ? TRUE : FALSE;
    fShowIndexies = mtaTempSystem.ShowIndexies() ? TRUE : FALSE;
    fShowHodograf = mtaTempSystem.ShowHodograf();
    fShowMapMarkers = mtaTempSystem.ShowMapMarkers();
    itsModelRunSpinner.SetPos(mtaTempSystem.ModelRunCount());
    fShowSideView = mtaTempSystem.ShowSideView();
    fSoundingTimeLockWithMapView = mtaTempSystem.SoundingTimeLockWithMapView();
    fSoundingTextUpward = mtaTempSystem.SoundingTextUpward();
    fShowSecondaryDataView = mtaTempSystem.DrawSecondaryData();

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
	CWnd* win = GetDescendantWindow(IDC_CHECK_SHOW_SIDE_VIEW);
	if(win)
	{
		WINDOWPLACEMENT wplace;
		BOOL bull = win->GetWindowPlacement(&wplace);
		CRect descRect = wplace.rcNormalPosition;
		areaHeight = descRect.bottom;
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
	CBrush brush(RGB(239, 235, 222));
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
    itsSmartMetDocumentInterface->MapViewDirty(CtrlViewUtils::kDoAllMapViewDescTopIndex, true, true, false, false, false); // laitetaan viela kaikki ajat likaisiksi cachesta
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
    itsSmartMetDocumentInterface->MapViewDirty(CtrlViewUtils::kDoAllMapViewDescTopIndex, true, true, false, false, false); // laitetaan viela kaikki ajat likaisiksi cachesta
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

void CFmiTempDlg::OnBnClickedCheckShowIndexies()
{
	UpdateData(TRUE);
    itsSmartMetDocumentInterface->GetMTATempSystem().ShowIndexies(fShowIndexies == TRUE);
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
	CFmiWin32Helpers::SetDialogItemText(this, IDC_BUTTON_PRINT, "IDC_BUTTON_PRINT");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_BUTTON_SETTINGS, "IDC_BUTTON_SETTINGS");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_BUTTON_RESET_SCALES, "IDC_BUTTON_RESET_SCALES");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_CHECK_TEMP_SKEWT_MODE, "IDC_CHECK_TEMP_SKEWT_MODE");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_CHECK_SHOW_INDEXIES, "IDC_CHECK_SHOW_INDEXIES");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_CHECK_SHOW_HODOGRAF, "IDC_CHECK_SHOW_HODOGRAF");

    CFmiWin32Helpers::SetDialogItemText(this, INSERT_TEMP_CODE, "TEMP");
    CFmiWin32Helpers::SetDialogItemText(this, IDC_BUTTON_RESET_SOUNDING_DATA, "R");
    CFmiWin32Helpers::SetDialogItemText(this, IDC_BUTTON_SHOW_TXT_SOUNDING_DATA, "txt");
    CFmiWin32Helpers::SetDialogItemText(this, IDC_CHECK_TEMP_SHOW_MAP_MARKERS, "Map markers");
    CFmiWin32Helpers::SetDialogItemText(this, IDC_CHECK_SHOW_SIDE_VIEW, "SideView");
    CFmiWin32Helpers::SetDialogItemText(this, IDC_CHECK_SHOW_SECONDARY_DATA_VIEW, "SV2");
    CFmiWin32Helpers::SetDialogItemText(this, IDC_CHECK_USE_MAP_TIME_WITH_SOUNDINGS, "MT");
    CFmiWin32Helpers::SetDialogItemText(this, IDC_CHECK_PUT_SOUNDING_TEXTS_UPWARD, "Up");
    CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_MODEL_RUN_COUNT_STR, "Model Run Count");
}

void CFmiTempDlg::OnBnClickedTempCode()
{
	CFmiTempCodeInsertDlg dlg(itsSmartMetDocumentInterface, this);
	if(dlg.DoModal() == IDOK)
	{
		// Karttanäyttö on piirrettävä uudestaan, koska siellä saattaa näkyä
		// uusia luotaus merkkejä.
        itsSmartMetDocumentInterface->MapViewDirty(CtrlViewUtils::kDoAllMapViewDescTopIndex, true, true, false, false, false); // laitetaan viela kaikki ajat likaisiksi cachesta
        itsSmartMetDocumentInterface->RefreshApplicationViewsAndDialogs("Sounding view: Applying new Temp Code data", TRUE);
	}
}


BOOL CFmiTempDlg::PreTranslateMessage(MSG* pMsg)
{
	// ei käsitellä 1-5 acceleraattoreita täällä luotaus näytössä (karttanäytössä rivin vaihto suoraan kyseiseen numeroon)
	// HUOM!!!! Tämä 1-5 pikanäppäin ohitus ei toimikaan, TODO: tutki miten saadaan näppäimet 1-5 käyttöön...
	bool ignoreThisAccelerator = (pMsg->message >= ID_ACCELERATOR_EXTRA_MAP_ROW_1) && (pMsg->message <= ID_ACCELERATOR_EXTRA_MAP_ROW_5);
    if (ignoreThisAccelerator == false && WM_KEYFIRST <= pMsg->message && pMsg->message <= WM_KEYLAST) // acceleratorien hanskaus
    {
        HACCEL hAccel = m_hAccel;
        if (hAccel && ::TranslateAccelerator(m_hWnd, hAccel, pMsg))
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

	InitControlsTooltip(IDC_BUTTON_RESET_SCALES, "IDC_BUTTON_RESET_SCALES_TOOLTIP");
	InitControlsTooltip(IDC_BUTTON_RESET_SOUNDING_DATA, "IDC_BUTTON_RESET_SOUNDING_DATA_TOOLTIP");

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
	checkedVector<NFmiProducer> selProducers;
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
	const checkedVector<NFmiProducer> &selectedProducers = itsSmartMetDocumentInterface->GetMTATempSystem().SoundingComparisonProducers();
	checkedVector<NFmiProducer>::size_type ssize = selectedProducers.size();
	checkedVector<NFmiProducer>::size_type xsize = itsProducerListWithData.size();
	for(checkedVector<NFmiProducer>::size_type i=0; i<ssize; i++)
	{
		for(checkedVector<NFmiProducer>::size_type j=0; j<xsize; j++)
		{
			if(selectedProducers[i].GetIdent() == itsProducerListWithData[j].GetIdent())
				itsMultiProducerSelector.SetCheck(static_cast<int>(j), true);
		}
	}
    fProducerSelectorUsedYet = true;
    UpdateData(FALSE);
}

void CFmiTempDlg::OnCbnSelchangeComboProducerSelection()
{
	fProducerSelectorUsedYet = true;
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

void CFmiTempDlg::UpdateProducerList(void)
{
	std::vector<std::pair<int, CString> > selectionVec = itsMultiProducerSelector.GetSelectedWithStr();

	checkedVector<NFmiProducer> tmpProducerListWithData;
	size_t i = 0;
	// 1. kysy täysi tuottajalista MTASystemiltä
	checkedVector<NFmiProducer>& possibleProdList = itsSmartMetDocumentInterface->GetMTATempSystem().PossibleProducerList();
	// 2. käy lista läpi ja katso mille tuottajalle löytyy dataa
	for(i=0; i<possibleProdList.size(); i++)
	{
		boost::shared_ptr<NFmiFastQueryInfo> info = itsSmartMetDocumentInterface->InfoOrganizer()->FindSoundingInfo(possibleProdList[i], 0, NFmiInfoOrganizer::ParamCheckFlags(true));
	// 3. laita ne tuottajat, joille löytyi dataa itsProducerListWithData-listaan
		if(info || ::FindProducer(selectionVec, possibleProdList[i].GetName())) // laitetaan sittenkin kaikki tuottajat aina listaan, että datojen reload ei tuhoa listaa (testi)
			tmpProducerListWithData.push_back(possibleProdList[i]);
	}

	if(tmpProducerListWithData != itsProducerListWithData)
	{ // Päivitetään listoja vain jos lista on muuttunut
		itsProducerListWithData = tmpProducerListWithData;
		// 4. laita tämän listan nimet itsProducerSelector-comboboxiin

		itsMultiProducerSelector.ResetContent();
		for(i=0; i<itsProducerListWithData.size(); i++)
			itsMultiProducerSelector.AddString(CA2T(itsProducerListWithData[i].GetName()));
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
        if(!itsSmartMetDocumentInterface->GetMTATempSystem().UpdateFromViewMacro())
            SetSelectedProducer();
	}

	// jos ollaan lataamassa näyttömakroa, pitää itsMultiProducerSelector-otus päivittää
	if(itsSmartMetDocumentInterface->GetMTATempSystem().UpdateFromViewMacro())
	{
		SetSelectedProducersFromViewMacro();
        itsSmartMetDocumentInterface->GetMTATempSystem().UpdateFromViewMacro(false);
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

void CFmiTempDlg::OnAcceleratorTempProducer1()
{
	SelectProducer(0);
}

void CFmiTempDlg::OnAcceleratorTempProducer2()
{
	SelectProducer(1);
}

void CFmiTempDlg::OnAcceleratorTempProducer3()
{
	SelectProducer(2);
}

void CFmiTempDlg::OnAcceleratorTempProducer4()
{
	SelectProducer(3);
}

void CFmiTempDlg::OnAcceleratorTempProducer5()
{
	SelectProducer(4);
}

void CFmiTempDlg::OnAcceleratorTempProducer6()
{
	SelectProducer(5);
}

void CFmiTempDlg::OnAcceleratorTempProducer7()
{
	SelectProducer(6);
}

void CFmiTempDlg::OnAcceleratorTempProducer8()
{
	SelectProducer(7);
}

void CFmiTempDlg::OnAcceleratorTempProducer9()
{
	SelectProducer(8);
}

void CFmiTempDlg::OnEditSpaceOut()
{
    itsSmartMetDocumentInterface->SoundingViewWindBarbSpaceOutFactor(itsSmartMetDocumentInterface->SoundingViewWindBarbSpaceOutFactor()+1);
	Update();
	Invalidate(FALSE);
}

void CFmiTempDlg::OnBnClickedCheckShowSideView()
{
	UpdateData(TRUE);
    itsSmartMetDocumentInterface->GetMTATempSystem().ShowSideView(fShowSideView == TRUE);
	Update();
	Invalidate(FALSE);
}

void CFmiTempDlg::OnAcceleratorSwapArea()
{
	fShowSideView = !fShowSideView;
	UpdateData(FALSE);
	OnBnClickedCheckShowSideView();
}

void CFmiTempDlg::OnAcceleratorToggleTooltip()
{
    itsSmartMetDocumentInterface->ShowToolTipTempView(!itsSmartMetDocumentInterface->ShowToolTipTempView());
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
    itsSmartMetDocumentInterface->ApplicationWinRegistry().SoundingTimeLockWithMapView(fSoundingTimeLockWithMapView == TRUE);
    itsSmartMetDocumentInterface->GetMTATempSystem().SoundingTimeLockWithMapView(itsSmartMetDocumentInterface->ApplicationWinRegistry().SoundingTimeLockWithMapView()); // Asetus pitää laittaa talteen myös MTATempSystem -luokalle!
	Update();
    itsSmartMetDocumentInterface->MapViewDirty(CtrlViewUtils::kDoAllMapViewDescTopIndex, true, true, false, false, false); // laitetaan viela kaikki ajat likaisiksi cachesta
    itsSmartMetDocumentInterface->RefreshApplicationViewsAndDialogs("Sounding view: Toggle Use map time");
}


void CFmiTempDlg::OnBnClickedCheckPutSoundingTextsUpward()
{
	UpdateData(TRUE);
    itsSmartMetDocumentInterface->ApplicationWinRegistry().SoundingTextUpward(fSoundingTextUpward == TRUE);
    itsSmartMetDocumentInterface->GetMTATempSystem().SoundingTextUpward(itsSmartMetDocumentInterface->ApplicationWinRegistry().SoundingTextUpward()); // Asetus pitää laittaa talteen myös MTATempSystem -luokalle!
    Update();
	Invalidate(FALSE);
}

// Tämä CTRL + F accelerator siis laittaa luotausnäytön apudata osion päälle/pois
void CFmiTempDlg::OnAcceleratorChangeMapTypeExtraMap()
{
    itsSmartMetDocumentInterface->GetMTATempSystem().DrawSecondaryData(!itsSmartMetDocumentInterface->GetMTATempSystem().DrawSecondaryData());
    fShowSecondaryDataView = itsSmartMetDocumentInterface->GetMTATempSystem().DrawSecondaryData();
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
