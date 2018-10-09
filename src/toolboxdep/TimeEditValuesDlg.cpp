// TimeEditValuesDlg.cpp : implementation file
//
#ifdef _MSC_VER
#pragma warning(disable : 4786) // poistaa n kpl VC++ kääntäjän varoitusta
#endif

#include "stdafx.h"
#include "TimeEditValuesDlg.h"
#include "TimeEditValuesView.h"
#include "SmartMetDocumentInterface.h"
#include "NFmiDrawParam.h"
#include "NFmiSmartInfo.h"
#include "NFmiMetEditorOptionsData.h"
#include "NFmiEditorControlPointManager.h"
#include "NFmiValueString.h"
#include "NFmiDictionaryFunction.h"
#include "FmiWin32TemplateHelpers.h"
#include "FmiNMeteditLibraryDefinitions.h"
#include "FmiWin32Helpers.h"
#include "FmiOperationProgressDlg.h"
#include "NFmiAnalyzeToolData.h"
#include "NFmiDrawParamList.h"
#include "CloneBitmap.h"
#include "NFmiCPManagerSet.h"
#include "NFmiApplicationWinRegistry.h"
#include "CtrlViewFunctions.h"
#include "persist2.h"

using namespace std::literals::string_literals;

#ifdef _MSC_VER
#pragma warning (disable : 4244 4267) // boost:in thread kirjastosta tulee ikävästi 4244 varoituksia
#endif
#include <boost/thread.hpp>
#ifdef _MSC_VER
#pragma warning (default : 4244 4267) // laitetaan 4244 takaisin päälle, koska se on tärkeä (esim. double -> int auto castaus varoitus)
#endif

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTimeEditValuesDlg dialog

const NFmiViewPosRegistryInfo CTimeEditValuesDlg::s_ViewPosRegistryInfo(CRect(500, 100, 1000, 600), "\\TimeSerialView");

CTimeEditValuesDlg::CTimeEditValuesDlg(SmartMetDocumentInterface *smartMetDocumentInterface, CWnd* pParent)
: CDialog(CTimeEditValuesDlg::IDD, pParent)
, itsTimeEditValuesView(nullptr)
,itsDrawParam(smartMetDocumentInterface->DefaultEditedDrawParam())
,itsSmartMetDocumentInterface(smartMetDocumentInterface)
,m_hAccel(0)
, fDoAutoAdjust(FALSE)
, fUseBothProducerData(FALSE)
, fUseMultiProcessCPCalc(FALSE)
, fAllowRightClickSelection(FALSE)
{
	//{{AFX_DATA_INIT(CTimeEditValuesDlg)
	itsModifyMode = itsSmartMetDocumentInterface->TimeEditMode();
	fUseMaskInTimeSerialViews = itsSmartMetDocumentInterface->IsMasksUsedInTimeSerialViews();
	fUseZoomedAreaCP = itsSmartMetDocumentInterface->UseCPGridCrop();
	itsSmootherValueStrU_ = _T("");
    itsCPManagerStrU_ = _T("");
	fUseAnalyzeTool = FALSE;
	fUseControlPointObservationsBlending = FALSE;
	//}}AFX_DATA_INIT
}

CTimeEditValuesDlg::~CTimeEditValuesDlg(void)
{
}

void CTimeEditValuesDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CTimeEditValuesDlg)
    DDX_Control(pDX, IDC_SLIDER_SMOOTHER_VALUE, itsSmootherSlider);
    DDX_Control(pDX, IDC_SLIDER_MODIFIERLENGTH, itsManualModifierLength);
    DDX_Check(pDX, IDC_CHECK_USE_MASKS_IN_TIME_SERIAL_VIEWS, fUseMaskInTimeSerialViews);
    DDX_Check(pDX, IDC_CHECK_USE_ZOOMED_AREA_CP, fUseZoomedAreaCP);

    DDX_Text(pDX, IDC_STATIC_SMOOTHER_VALUE_STR, itsSmootherValueStrU_);
    DDX_Text(pDX, IDC_STATIC_CP_MANAGER_NAME_STR, itsCPManagerStrU_);

    DDX_Check(pDX, IDC_CHECK_USE_ANALYZE_TOOL, fUseAnalyzeTool);
    DDX_Check(pDX, IDC_CHECK_USE_CP_OBS_BLENDING, fUseControlPointObservationsBlending);
    //}}AFX_DATA_MAP
    DDX_Check(pDX, IDC_CHECK_TIME_SERIAL_AUTO_ADJUST, fDoAutoAdjust);
    DDX_Control(pDX, IDC_COMBO_ANALYZE_PRODUCER1, itsProducer1Selector);
    DDX_Control(pDX, IDC_COMBO_ANALYZE_PRODUCER2, itsProducer2Selector);
    DDX_Check(pDX, IDC_CHECK_ANALYZE_WITH_BOTH_DATA, fUseBothProducerData);
    DDX_Check(pDX, IDC_CHECK_USE_MULTI_PROCESS_CP_CALC, fUseMultiProcessCPCalc);
    DDX_Check(pDX, IDC_CHECK_ALLOW_RIGHT_CLICK_SELECTION, fAllowRightClickSelection);
}


BEGIN_MESSAGE_MAP(CTimeEditValuesDlg, CDialog)
	//{{AFX_MSG_MAP(CTimeEditValuesDlg)
	ON_BN_CLICKED(IDC_BUTTON_TOIMINTO, OnButtonToiminto)
	ON_BN_CLICKED(IDC_BUTTON_CLOSE, OnButtonClose)
	ON_WM_SIZE()
	ON_BN_CLICKED(IDC_BUTTON_REDO, OnButtonRedo)
	ON_BN_CLICKED(IDC_BUTTON_UNDO, OnButtonUndo)
	ON_WM_CLOSE()
	ON_WM_HSCROLL()
	ON_BN_CLICKED(IDC_CHECK_USE_MASKS_IN_TIME_SERIAL_VIEWS, OnCheckUseMasksInTimeSerialViews)
	ON_BN_CLICKED(IDC_CHECK_USE_ZOOMED_AREA_CP, OnCheckUseZoomedAreaCP)
	ON_BN_CLICKED(IDC_BUTTON_CLEAR_ALL_CP_VALUES, OnButtonClearAllCpValues)
	ON_BN_CLICKED(IDC_BUTTON_PRINT, OnButtonPrint)
	ON_BN_CLICKED(IDC_CHECK_USE_ANALYZE_TOOL, OnCheckUseAnalyzeTool)
	ON_BN_CLICKED(IDC_CHECK_USE_CP_OBS_BLENDING, OnCheckUseCpObsBlending)
	//}}AFX_MSG_MAP
	ON_WM_MOUSEWHEEL()
	ON_WM_CTLCOLOR()
	ON_COMMAND(ID_ACCELERATOR_CHANGE_MAP_TYPE_EXTRA_MAP, OnAcceleratorTimeViewToggleHelpData2)
	ON_COMMAND(ID_ACCELERATOR_TIME_VIEW_TOGGLE_HELP_DATA1, OnAcceleratorTimeViewToggleHelpData1)
	ON_COMMAND(ID_ACCELERATOR_TOGGLE_TEMP_VIEW_TOOLTIP, OnAcceleratorToggleTooltip)
	ON_COMMAND(ID_ACCELERATOR_CLOSE_SIDE_VIEW, &CTimeEditValuesDlg::OnAcceleratorSwapArea)
	ON_BN_CLICKED(IDC_CHECK_TIME_SERIAL_AUTO_ADJUST, &CTimeEditValuesDlg::OnBnClickedCheckTimeSerialAutoAdjust)
	ON_CBN_SELCHANGE(IDC_COMBO_ANALYZE_PRODUCER1, &CTimeEditValuesDlg::OnCbnSelchangeComboAnalyzeProducer1)
	ON_CBN_SELCHANGE(IDC_COMBO_ANALYZE_PRODUCER2, &CTimeEditValuesDlg::OnCbnSelchangeComboAnalyzeProducer2)
	ON_BN_CLICKED(IDC_CHECK_ANALYZE_WITH_BOTH_DATA, &CTimeEditValuesDlg::OnBnClickedCheckAnalyzeWithBothData)
    ON_BN_CLICKED(IDC_CHECK_USE_MULTI_PROCESS_CP_CALC, &CTimeEditValuesDlg::OnBnClickedCheckUseMultiProcessCpCalc)
    ON_BN_CLICKED(IDC_CHECK_ALLOW_RIGHT_CLICK_SELECTION, &CTimeEditValuesDlg::OnBnClickedCheckAllowRightClickSelection)
    ON_COMMAND(ID_ACCELERATOR_TIME_VIEW_TOGGLE_HELP_DATA3, &CTimeEditValuesDlg::OnAcceleratorTimeViewToggleHelpData3)
    ON_COMMAND(ID_ACCELERATOR_TIME_VIEW_TOGGLE_HELP_DATA4, &CTimeEditValuesDlg::OnAcceleratorTimeViewToggleHelpData4)
    ON_COMMAND(ID_ACCELERATOR_CP_TIME_SERIAL_SELECT_NEXT, &CTimeEditValuesDlg::OnAcceleratorCpSelectNext)
    ON_COMMAND(ID_ACCELERATOR_CP_TIME_SERIAL_SELECT_PREVIOUS, &CTimeEditValuesDlg::OnAcceleratorCpSelectPrevious)
    ON_COMMAND(ID_ACCELERATOR_CP_TIME_SERIAL_SELECT_LEFT, &CTimeEditValuesDlg::OnAcceleratorCpSelectLeft)
    ON_COMMAND(ID_ACCELERATOR_CP_TIME_SERIAL_SELECT_RIGHT, &CTimeEditValuesDlg::OnAcceleratorCpSelectRight)
    ON_COMMAND(ID_ACCELERATOR_CP_TIME_SERIAL_SELECT_UP, &CTimeEditValuesDlg::OnAcceleratorCpSelectUp)
    ON_COMMAND(ID_ACCELERATOR_CP_TIME_SERIAL_SELECT_DOWN, &CTimeEditValuesDlg::OnAcceleratorCpSelectDown)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTimeEditValuesDlg message handlers

BOOL CTimeEditValuesDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

    // Load accelerators
    m_hAccel = ::LoadAccelerators(AfxGetResourceHandle(), MAKEINTRESOURCE(IDR_ACCELERATOR1));
    ASSERT(m_hAccel);

	std::string errorBaseStr("Error in CTimeEditValuesDlg::OnInitDialog while reading dialog size and position values");
    CFmiWin32TemplateHelpers::DoWindowSizeSettingsFromWinRegistry(itsSmartMetDocumentInterface->ApplicationWinRegistry(), this, false, errorBaseStr, 0);
	CRect rect(CalcClientArea());
    itsTimeEditValuesView = new CTimeEditValuesView(this, itsDrawParam, itsSmartMetDocumentInterface);
    itsTimeEditValuesView->Create(NULL,NULL, WS_VISIBLE | WS_CHILD, rect, this, NULL);
    itsTimeEditValuesView->OnInitialUpdate(); // pitää kutsua erikseen, koska formvieta ei ole sidottu dokumenttiin

	HICON hIcon = CCloneBitmap::BitmapToIcon(FMI_LOGO_BITMAP_2, ColorPOD(160, 160, 164));
	this->SetIcon(hIcon, FALSE);

	fUseMaskInTimeSerialViews = itsSmartMetDocumentInterface->IsMasksUsedInTimeSerialViews();
	fUseZoomedAreaCP = itsSmartMetDocumentInterface->UseCPGridCrop();

	UpdateSlider();
	itsManualModifierLength.SetPos(0); //alkuarvo
    itsTimeEditValuesView->ManualModifierLength(CalcRelativeSliderValue());

	itsSmootherSlider.SetRange(0, itsSmartMetDocumentInterface->TimeEditSmootherMaxValue());
	itsSmootherSlider.SetPos(itsSmartMetDocumentInterface->TimeEditSmootherValue());
	UpdateSmootherString();

	UpdateCPManagerString();

	InitDialogTexts();

	EnableButtons();
	SeViewModeButtonsSetup(); // tätä kutsutaan vain kerran ja laittaa monet kontrollit pois päältä, jos ollaan ns. view-moodissa

	CSize clienRectSizeInPixels(CalcClientArea().Size());
    itsSmartMetDocumentInterface->TimeSerialViewSizeInPixels(NFmiPoint(clienRectSizeInPixels.cx, clienRectSizeInPixels.cx));

    fDoAutoAdjust = itsSmartMetDocumentInterface->UseTimeSerialAxisAutoAdjust();
    fAllowRightClickSelection = itsSmartMetDocumentInterface->AllowRightClickDisplaySelection();
	fUseAnalyzeTool = FALSE; // alussa analyysi työkalu ei ole käytössä
	fUseBothProducerData = itsSmartMetDocumentInterface->AnalyzeToolData().UseBothProducers();
    fUseControlPointObservationsBlending = itsSmartMetDocumentInterface->AnalyzeToolData().ControlPointObservationBlendingData().UseBlendingTool();
    fUseMultiProcessCPCalc = itsSmartMetDocumentInterface->UseMultiProcessCpCalc();
	UpdateControlsAfterAnalyzeMode();
	UpdateControlsAfterMPCPMode();

	UpdateData(FALSE);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CTimeEditValuesDlg::UpdateSmootherString(void)
{
	UpdateData(TRUE);
	int value = itsSmootherSlider.GetPos();
	NFmiValueString valuStr(value, "%d");
	itsSmootherValueStrU_ = CA2T(valuStr);
    itsSmartMetDocumentInterface->TimeEditSmootherValue(value);
	UpdateData(FALSE);
}

void CTimeEditValuesDlg::UpdateCPManagerString(void)
{
    itsCPManagerStrU_ = _TEXT("");
	if(itsSmartMetDocumentInterface->MetEditorOptionsData().ControlPointMode() && itsSmartMetDocumentInterface->CPManagerSet().UseOldSchoolStyle() == false)
        itsCPManagerStrU_ = CA2T(itsSmartMetDocumentInterface->CPManager()->Name().c_str());
	UpdateData(FALSE);
}

NFmiPoint CTimeEditValuesDlg::CalcEditedDataGriddingResolutionInKM(void)
{
	NFmiPoint resolution;
	boost::shared_ptr<NFmiFastQueryInfo> editedInfo = itsSmartMetDocumentInterface->EditedSmartInfo();
	if(editedInfo && editedInfo->Grid())
	{
		unsigned long xSize = editedInfo->GridXNumber();
		unsigned long ySize = editedInfo->GridYNumber();
		editedInfo->LocationIndex((ySize/2 * xSize) + (xSize/2));
		NFmiLocation locCenter(editedInfo->LatLon());
		editedInfo->MoveRight();
		NFmiLocation locToRight(editedInfo->LatLon());
		editedInfo->MoveLeft();
		editedInfo->MoveUp();
		NFmiLocation locToUp(editedInfo->LatLon());
		resolution.X(locCenter.Distance(locToRight) / 1000.);
		resolution.Y(locCenter.Distance(locToUp) / 1000.);
	}
	return resolution;
}

// Tämä Aikasarjamuokkaus toiminnon progress/peruutus ei toimi, niin että laitetaan
// Muokkaus koodi threadiin ja Progress-Cancel-dialogi pääthreadiin osottamaan.
// Tästä seurasi ongelmia, kun SmartMet lukee dataa taustalla ja päivittelee ruutuja.
// Tällöin tulee väistämättä ongelmia, koska SmartMetin datanmuokkaus koodeja ei ole
// tehty thread-turvallisesti.
// Kokeile niin että datan muokkaus tapahtuu pääsäikeessä ja progress-cancel-dialogi on omassa threadissa.
// Kokeilin ja tuomio on se että mitään GUI-juttuja ei voi ajaa main-threadin ulkopuolella, joten ainoa
// oikea keino on tehdä SmartMetin datan muokkauksesta threadi turvallinen.
/*
static void DoTimeSerialModificationThread(NFmiThreadCallBacks &theThreadCallBacks, CTimeEditValuesView &theTimeModificationView)
{
	try
	{
		theTimeModificationView.ChangeTimeSeriesValues(theThreadCallBacks);
		theThreadCallBacks.DoPostMessage(ID_MESSAGE_WORKING_THREAD_COMPLETED);
		return ; // ei saa mennä catch:ien jälkeiseen viestin lähetykseen...
	}
	catch(NFmiStopThreadException & )
	{
	}
	catch(...)
	{
	}
	theThreadCallBacks.DoPostMessage(ID_MESSAGE_WORKING_THREAD_CANCELED);
}
*/
/*
void CTimeEditValuesDlg::OnButtonToiminto() 
{
	CWaitCursor waitCursor;
	UpdateData(TRUE);

	// Tehdään aikamuokkaukselle progress ja peruutus dialogi ja toiminnot.
	NFmiStopFunctor stopper;
	CFmiOperationProgressDlg dlg(stopper, this);
	NFmiThreadCallBacks threadCallBacks(&stopper, &dlg);
	CTimeEditValuesView &timeModifierView = *(CTimeEditValuesView*)itsClientView;
	// Luodaaan ja laitetaan työ-threadi käyntiin.
	boost::thread wrk_thread(::DoTimeSerialModificationThread, boost::ref(threadCallBacks), boost::ref(timeModifierView));
	// käynnistetään sitten porgress ja cancel dialogi, että käyttäjä voi tarvittaessa keskeyttää työt.
	int status = static_cast<int>(dlg.DoModal());
	if(status == IDCANCEL) // Jos käyttäjä keskeytti työn, pitää tehdä undo.
		OnButtonUndo();

	EnableButtons();
	RefreshApplicationViews();
}
*/

void CTimeEditValuesDlg::OnButtonToiminto() 
{
	CWaitCursor waitCursor;
	UpdateData(TRUE);

	if(itsSmartMetDocumentInterface->MetEditorOptionsData().ControlPointMode())
		EnableDlgItem(IDC_BUTTON_TOIMINTO, false); // koska mm. kontrollipiste muokkaus menee nykyään omaan threadiin ja pääkarttanäyttö lukitaan Proegress-cancelointi -dialogilla, 
													// pitää tässä estää että ei voi painaa uudestaan Muokkaa-nappulaa. Napin normaali tila palautetaan kun lähetetään viesti ID_MESSAGE_TIME_SERIAL_MODIFICATION_ENDED
	bool anyDataModified = itsTimeEditValuesView->ChangeTimeSeriesValues();

	// HUOM! jos mitään dataa ei muokattu kontrollipite moodissa, pitää nappuloiden tila palauttaa taas ennalleen ja ruutuja päivittää!
	if((itsSmartMetDocumentInterface->MetEditorOptionsData().ControlPointMode() == false) || (anyDataModified == false))
	{
		EnableButtons();
		RefreshApplicationViews("TimeSerialDlg: Modify edited data in CP mode had no effect");
	}
}

void CTimeEditValuesDlg::OnButtonClose() 
{
	OnOK();
}

void CTimeEditValuesDlg::OnOK()
{
	DoWhenClosing();
	ShowWindow(SW_HIDE); // suljettaessa vain piilotetaan, tuhoaminen tehdään ulkoakäsin
}

void CTimeEditValuesDlg::OnCancel()
{
	DoWhenClosing();
	ShowWindow(SW_HIDE); // suljettaessa vain piilotetaan, tuhoaminen tehdään ulkoakäsin
}

void CTimeEditValuesDlg::EditModeUpdate(void)
{
	UpdateData(TRUE);
    itsSmartMetDocumentInterface->TimeEditMode(itsModifyMode);
    itsSmartMetDocumentInterface->TimeSerialViewDirty(true);
	Invalidate(FALSE);
}

void CTimeEditValuesDlg::OnSize(UINT nType, int cx, int cy) 
{
	CDialog::OnSize(nType, cx, cy);

	if(itsTimeEditValuesView)
	{
		CRect clientRect(CalcClientArea());
        itsTimeEditValuesView->MoveWindow(clientRect, FALSE);
        itsSmartMetDocumentInterface->TimeSerialViewSizeInPixels(NFmiPoint(clientRect.Width(), clientRect.Height()));
	}
}

CRect CTimeEditValuesDlg::CalcClientArea(void)
{
	CRect rect;
	GetClientRect(rect);
	CWnd* window = GetDescendantWindow(IDC_BUTTON_CLOSE);
	WINDOWPLACEMENT wplace;
	BOOL bull = window->GetWindowPlacement(&wplace);
	CRect buttonRect = wplace.rcNormalPosition; // its size must shrink a little bit
	rect.top = buttonRect.bottom + 5;
	return rect;
}

void CTimeEditValuesDlg::EnableButtons()
{
	if(itsSmartMetDocumentInterface)
	{
		boost::shared_ptr<NFmiFastQueryInfo> info = itsSmartMetDocumentInterface->EditedSmartInfo();
		NFmiSmartInfo *smartInfo = dynamic_cast<NFmiSmartInfo*>(info.get());

		CheckIfAnalyzeToolModePossibleAndUpdateControls();
		bool isUndoable = false;
		if(smartInfo && smartInfo->Undo() && (itsSmartMetDocumentInterface->SmartMetEditingMode() == CtrlViewUtils::kFmiEditingModeNormal))
			isUndoable = true;
		bool isRedoable = false;
		if(smartInfo && smartInfo->Redo() && (itsSmartMetDocumentInterface->SmartMetEditingMode() == CtrlViewUtils::kFmiEditingModeNormal))
			isRedoable = true;

		CWnd* win;
		win = GetDlgItem(IDC_BUTTON_UNDO);
		if(win)
			win->EnableWindow(isUndoable);
		win = GetDlgItem(IDC_BUTTON_REDO);
		if(win)
			win->EnableWindow(isRedoable);
	}
}

void CTimeEditValuesDlg::SeViewModeButtonsSetup(void)
{
    bool enableEditingControls = itsSmartMetDocumentInterface->SmartMetEditingMode() == CtrlViewUtils::kFmiEditingModeNormal;
    // Riippuen ollaanko ns. view-moodissa vai ei, disabloidaan/enabloidaan editoinnissa käytettyjä nappuloita
    EnableDlgItem(IDC_CHECK_USE_CP_OBS_BLENDING, enableEditingControls);
    EnableDlgItem(IDC_STATIC_SMOOTHER_VALUE_STR, enableEditingControls);
    EnableDlgItem(IDC_STATIC_CP_MANAGER_NAME_STR, enableEditingControls);
    EnableDlgItem(IDC_SLIDER_SMOOTHER_VALUE, enableEditingControls);
    EnableDlgItem(IDC_STATIC_SMOOTH_STR, enableEditingControls);
    EnableDlgItem(IDC_SLIDER_MODIFIERLENGTH, enableEditingControls);
    EnableDlgItem(IDC_STATIC_VAIKUTUSALUE_STR, enableEditingControls);
    EnableDlgItem(IDC_BUTTON_TOIMINTO, enableEditingControls);
    EnableDlgItem(IDC_BUTTON_CLEAR_ALL_CP_VALUES, enableEditingControls);
    EnableDlgItem(IDC_CHECK_USE_MASKS_IN_TIME_SERIAL_VIEWS, enableEditingControls);
    EnableDlgItem(IDC_CHECK_USE_ZOOMED_AREA_CP, enableEditingControls);
    EnableDlgItem(IDC_CHECK_USE_ANALYZE_TOOL, enableEditingControls);
}

void CTimeEditValuesDlg::OnButtonRedo() 
{
    itsSmartMetDocumentInterface->OnButtonRedo();
}

void CTimeEditValuesDlg::OnButtonUndo() 
{
    itsSmartMetDocumentInterface->OnButtonUndo();
}

void CTimeEditValuesDlg::OnClose() 
{
	DoWhenClosing();
	CDialog::OnClose();
}

void CTimeEditValuesDlg::DoWhenClosing(void)
{
    itsSmartMetDocumentInterface->TimeSerialDataViewOn(false);
	AfxGetMainWnd()->SetActiveWindow(); // aktivoidaan karttanäyttö eli mainframe
}

void CTimeEditValuesDlg::Update(void)
{
    if(IsWindowVisible() && !IsIconic()) // Näyttöä päivitetään vain jos se on näkyvissä ja se ei ole minimized tilassa
    {
        UpdateSlider();
        UpdateCPManagerString();

        if(itsTimeEditValuesView)
            itsTimeEditValuesView->Update();

        if(itsSmartMetDocumentInterface)
        {
            EnableButtons();
            UpdateAnalyseActionControl();
            SeViewModeButtonsSetup();
            if(itsSmartMetDocumentInterface->AnalyzeToolData().ControlPointObservationBlendingData().OverrideSelection())
                UpdateControlsAfterAnalyzeMode();
        }
    }
}

double CTimeEditValuesDlg::CalcRelativeSliderValue()
{
	int left;
	int right;
	itsManualModifierLength.GetRange(left,right);
	return double(itsManualModifierLength.GetPos())/double(right-left);
}

void CTimeEditValuesDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	// TODO: Add your message handler code here and/or call default
	UpdateData(TRUE);
    itsTimeEditValuesView->ManualModifierLength(CalcRelativeSliderValue());
	UpdateSmootherString();
	CDialog::OnHScroll(nSBCode, nPos, pScrollBar);
}

// asettaa sen 'valittu muokkaus vaikutusalue'-säätimen koon (askelten määrän) dynaamisesti 
// riippuen datassa olevista aikojen lukumäärästä
void CTimeEditValuesDlg::UpdateSlider(void)
{
	if(itsSmartMetDocumentInterface && itsSmartMetDocumentInterface->EditedSmartInfo())
	{
		int timeCount = itsSmartMetDocumentInterface->EditedDataTimeBag().GetSize();
		itsManualModifierLength.SetRange(0, timeCount-1);
		UpdateData(FALSE);
	}
}

void CTimeEditValuesDlg::OnCheckUseMasksInTimeSerialViews() 
{
	if(itsSmartMetDocumentInterface)
	{
		UpdateData(TRUE);
        itsSmartMetDocumentInterface->UseMasksInTimeSerialViews(fUseMaskInTimeSerialViews == TRUE);
		Invalidate(FALSE);
	}
}

void CTimeEditValuesDlg::OnCheckUseZoomedAreaCP() 
{
	if(itsSmartMetDocumentInterface)
	{
		UpdateData(TRUE);
        itsSmartMetDocumentInterface->UseCPGridCrop(fUseZoomedAreaCP == TRUE);
        itsSmartMetDocumentInterface->MapDirty(0, true, true);
        itsSmartMetDocumentInterface->RefreshApplicationViewsAndDialogs("TimeSerialDlg: Toggle Zoomed CP setting");
		Invalidate(FALSE);
	}
}

void CTimeEditValuesDlg::RefreshApplicationViews(const std::string &reasonForUpdate)
{
    itsSmartMetDocumentInterface->AreaViewDirty(CtrlViewUtils::kDoAllMapViewDescTopIndex, true, false);
	EnableButtons();
    itsSmartMetDocumentInterface->RefreshApplicationViewsAndDialogs(reasonForUpdate);
}

void CTimeEditValuesDlg::SetDefaultValues(void)
{
    MoveWindow(CTimeEditValuesDlg::ViewPosRegistryInfo().DefaultWindowRect());
    Persist2::WriteWindowRectToWinRegistry(itsSmartMetDocumentInterface->ApplicationWinRegistry(), MakeUsedWinRegistryKeyStr(0), this);
}

void CTimeEditValuesDlg::OnButtonClearAllCpValues() 
{
	if(itsSmartMetDocumentInterface)
	{
		if(itsSmartMetDocumentInterface->MetEditorOptionsData().ControlPointMode())
            itsSmartMetDocumentInterface->CPManager()->ClearAllChangeValues(0, 0.); // 0=kaikkien CP:en kaikki arvot, 0. nollaus arvo
		else
            itsTimeEditValuesView->ResetAllModifyFactorValues();
		RefreshApplicationViews("TimeSerialDlg: Clear all CP values");
	}
}


void CTimeEditValuesDlg::OnButtonPrint() 
{
    itsTimeEditValuesView->DoPrint();
	SetActiveWindow(); // aktivoidaan vielä tämä ikkuna, koska jostain syystä print-dialogi aktivoi pääikkunan
}

void CTimeEditValuesDlg::OnCheckUseAnalyzeTool() 
{
	UpdateData(TRUE);
    SetAnalyzeRelatedTools(true, "TimeSerialDlg: TimeSerialDlg: Toggle use analyze tool"s);
}

void CTimeEditValuesDlg::OnCheckUseCpObsBlending()
{
	UpdateData(TRUE);
    SetAnalyzeRelatedTools(false, "TimeSerialDlg: Toggle use observation blending tool"s);
}

// Tämän funktion tehtävänä on varmistaa että vain yksi (tai ei yhtään) analyysi-työkaluun liittyvistä 
// työkaluista on kerrallaan päällä.
// Jos analyzeToolHasJustBeenSet on true, tällöin varmistetaan että Obs-blenderin asetukset laitetaan sallittuun tilaan
// ja jos se on false, varmistetaan että analyysi-työkalun asetukset menevät oikein.
void CTimeEditValuesDlg::SetAnalyzeRelatedTools(bool analyzeToolHasJustBeenSet, const std::string &logMessage)
{
    if(itsSmartMetDocumentInterface)
    {
        if(analyzeToolHasJustBeenSet)
        {
            itsSmartMetDocumentInterface->AnalyzeToolData().AnalyzeToolMode(fUseAnalyzeTool == TRUE);
            if(fUseAnalyzeTool)
            {
                // Analyysityökalu on juuri laitettu päälle, pitää varmistaa että Obs-blender ei ole päällä
                fUseControlPointObservationsBlending = FALSE;
                itsSmartMetDocumentInterface->AnalyzeToolData().ControlPointObservationBlendingData().UseBlendingTool(fUseControlPointObservationsBlending == TRUE);
            }
        }
        else
        {
            itsSmartMetDocumentInterface->AnalyzeToolData().ControlPointObservationBlendingData().UseBlendingTool(fUseControlPointObservationsBlending == TRUE);
            if(fUseControlPointObservationsBlending)
            {
                // Obs-blender on juuri laitettu päälle, pitää varmistaa että analyysi-työkalu ei ole päällä
                fUseAnalyzeTool = FALSE;
                itsSmartMetDocumentInterface->AnalyzeToolData().AnalyzeToolMode(fUseAnalyzeTool == TRUE);
            }
        }
        UpdateData(FALSE); // Päivitä checkbox kontrollien tilat
        UpdateControlsAfterAnalyzeMode();
        RefreshApplicationViews(logMessage);
    }
}

void CTimeEditValuesDlg::UpdateControlsAfterAnalyzeMode(void)
{
    if(fUseControlPointObservationsBlending)
    {
        // Jos CP observation blend -moodi on päällä, pitää moni kontrolli estää ja muutama mahdollistaa ja tuoda näkyviin
        UpdateCpObsBlendProducerList();
        bool status = EnableDlgItem(IDC_CHECK_ANALYZE_WITH_BOTH_DATA, false, false);
        status = EnableDlgItem(IDC_COMBO_ANALYZE_PRODUCER1, true, true);
        status = EnableDlgItem(IDC_COMBO_ANALYZE_PRODUCER2, false, false);

        status = EnableDlgItem(IDC_STATIC_SMOOTHER_VALUE_STR, false);
        status = EnableDlgItem(IDC_STATIC_CP_MANAGER_NAME_STR, false);
        status = EnableDlgItem(IDC_SLIDER_SMOOTHER_VALUE, false);
        status = EnableDlgItem(IDC_STATIC_SMOOTH_STR, false);
        status = EnableDlgItem(IDC_SLIDER_MODIFIERLENGTH, false);
        status = EnableDlgItem(IDC_STATIC_VAIKUTUSALUE_STR, false);
    }
    else if(fUseAnalyzeTool)
	{
        // jos analyysi-moodi on päällä, pitää moni kontrolli estää ja muutama mahdollistaa ja tuoda näkyviin
		UpdateProducerLists();
        bool status = EnableDlgItem(IDC_CHECK_ANALYZE_WITH_BOTH_DATA, true, true);
		status = EnableDlgItem(IDC_COMBO_ANALYZE_PRODUCER1, true, true);
		status = EnableDlgItem(IDC_COMBO_ANALYZE_PRODUCER2, true, true);

		status = EnableDlgItem(IDC_STATIC_SMOOTHER_VALUE_STR, false);
		status = EnableDlgItem(IDC_STATIC_CP_MANAGER_NAME_STR, false);
		status = EnableDlgItem(IDC_SLIDER_SMOOTHER_VALUE, false);
		status = EnableDlgItem(IDC_STATIC_SMOOTH_STR, false);
		status = EnableDlgItem(IDC_SLIDER_MODIFIERLENGTH, false);
		status = EnableDlgItem(IDC_STATIC_VAIKUTUSALUE_STR, false);
	}
	else
	{
        // Lopuksi kun kumpikaan erikois moodi ei ole päällä
        bool status = EnableDlgItem(IDC_CHECK_ANALYZE_WITH_BOTH_DATA, false, false);
		status = EnableDlgItem(IDC_COMBO_ANALYZE_PRODUCER1, false, false);
		status = EnableDlgItem(IDC_COMBO_ANALYZE_PRODUCER2, false, false);

		status = EnableDlgItem(IDC_STATIC_SMOOTHER_VALUE_STR, true);
		status = EnableDlgItem(IDC_STATIC_CP_MANAGER_NAME_STR, true);
		status = EnableDlgItem(IDC_SLIDER_SMOOTHER_VALUE, true);
		status = EnableDlgItem(IDC_STATIC_SMOOTH_STR, true);
		status = EnableDlgItem(IDC_SLIDER_MODIFIERLENGTH, true);
		status = EnableDlgItem(IDC_STATIC_VAIKUTUSALUE_STR, true);
	}
	UpdateAnalyseActionControl();
}

void CTimeEditValuesDlg::UpdateControlsAfterMPCPMode(void)
{
    if(fUseMultiProcessCPCalc)
    {
		EnableDlgItem(IDC_CHECK_USE_ZOOMED_AREA_CP, false, true);
    }
    else
    {
		EnableDlgItem(IDC_CHECK_USE_ZOOMED_AREA_CP, true, true);
    }
}

bool CTimeEditValuesDlg::EnableDlgItem(int theDlgId, bool fEnable, bool fShow)
{
	CWnd *window = GetDlgItem(theDlgId);
	if(window)
	{
		if(fEnable)
			window->EnableWindow(TRUE);
		else
			window->EnableWindow(FALSE);

		if(fShow)
			window->ShowWindow(TRUE);
		else
			window->ShowWindow(FALSE);
		return true;
	}
	return false;
}

void CTimeEditValuesDlg::CheckIfAnalyzeToolModePossibleAndUpdateControls(void)
{
	if(itsSmartMetDocumentInterface && itsSmartMetDocumentInterface->SmartMetEditingMode() == CtrlViewUtils::kFmiEditingModeNormal) // tehdään näitä asetuksia vain ns. edit-moodissa
	{
		// analyysi työkalu on enabloitava aina, koska tuottajaa voidaan vaihtaa...
		bool status = EnableDlgItem(IDC_CHECK_USE_ANALYZE_TOOL, true);
		UpdateControlsAfterAnalyzeMode();
/* 
		if(itsSmartMetDocumentInterface->EnableAnalyzeTool()) // jos shortrange ja analyysi dataa löytyy
		{
			bool status = EnableDlgItem(IDC_CHECK_USE_ANALYZE_TOOL, true);
			UpdateControlsAfterAnalyzeMode();
		}
		else
		{
			fUseAnalyzeTool = FALSE; // jos ei voi olla käytössä, otetaan se pois päältä!
			itsSmartMetDocumentInterface->AnalyzeToolData().AnalyzeToolMode(fUseAnalyzeTool == TRUE); // asetetaan vielä moodi varmuuden vuoksi dociin!
			bool status = EnableDlgItem(IDC_CHECK_USE_ANALYZE_TOOL, false);
			UpdateControlsAfterAnalyzeMode();
			UpdateData(FALSE);
		}
*/
	}
}

CBitmap* CTimeEditValuesDlg::ViewMemoryBitmap(void)
{
	return itsTimeEditValuesView->MemoryBitmap();
}

// Tämä funktio alustaa kaikki dialogin tekstit editoriin valitulla kielellä.
// Tämä on ikävä kyllä tehtävä erikseen dialogin muokkaus työkalusta, eli
// tekijän pitää lisätä erikseen tänne kaikki dialogin osat, joihin 
// kieli valinta voi vaikuttaa.
void CTimeEditValuesDlg::InitDialogTexts(void)
{
	SetWindowText(CA2T(::GetDictionaryString("TimeSerialViewDlgTitle").c_str()));

	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_VAIKUTUSALUE_STR, "IDC_STATIC_VAIKUTUSALUE_STR");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_SMOOTH_STR, "IDC_STATIC_SMOOTH_STR");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_BUTTON_TOIMINTO, "IDC_BUTTON_TOIMINTO");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_CHECK_USE_ANALYZE_TOOL, "IDC_CHECK_USE_ANALYZE_TOOL");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_CHECK_USE_CP_OBS_BLENDING, "CP obs blending");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_CHECK_USE_MASKS_IN_TIME_SERIAL_VIEWS, "IDC_CHECK_USE_MASKS_IN_TIME_SERIAL_VIEWS");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_CHECK_USE_ZOOMED_AREA_CP, "Zoomed CP");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_BUTTON_CLEAR_ALL_CP_VALUES, "IDC_BUTTON_CLEAR_ALL_CP_VALUES");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_BUTTON_CLOSE, "IDC_BUTTON_CLOSE");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_BUTTON_UNDO, "IDC_BUTTON_UNDO");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_BUTTON_REDO, "IDC_BUTTON_REDO");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_BUTTON_PRINT, "IDC_BUTTON_PRINT");
}

BOOL CTimeEditValuesDlg::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	// jos ollaan CPManager-tekstin päällä, rullalla voi vaihtaa edelliseen/seuraavaan listalla olevaan CPManageriin
	NFmiCPManagerSet &cpManager = itsSmartMetDocumentInterface->CPManagerSet();
	if(itsSmartMetDocumentInterface->MetEditorOptionsData().ControlPointMode() && cpManager.UseOldSchoolStyle() == false && cpManager.CPSetSize() > 1)
	{
		CPoint screenPt(pt);
		ScreenToClient(&screenPt);
		CWnd *win = ChildWindowFromPoint(screenPt);
		if(win && win->GetDlgCtrlID() == IDC_STATIC_CP_MANAGER_NAME_STR)
		{
			if(zDelta > 0)
				cpManager.NextCPManager();
			else
				cpManager.PreviousCPManager();

            itsSmartMetDocumentInterface->TimeSerialViewDirty(true);
            itsSmartMetDocumentInterface->AreaViewDirty(0, true, true);
			RefreshApplicationViews("TimeSerialDlg: Mouse wheel action");
			return TRUE;
		}
	}

	BOOL status = itsTimeEditValuesView->OnMouseWheel(nFlags, zDelta, pt);
	return CDialog::OnMouseWheel(nFlags, zDelta, pt) && status;
}

void CTimeEditValuesDlg::OnAcceleratorTimeViewToggleHelpData2()
{
	if(itsSmartMetDocumentInterface->ShowHelperData2InTimeSerialView())
        itsSmartMetDocumentInterface->ToggleShowHelperDatasInTimeView(kFmiDontShowHelperData2OnTimeSerialView);
	else
        itsSmartMetDocumentInterface->ToggleShowHelperDatasInTimeView(kFmiShowHelperData2OnTimeSerialView);

	Invalidate(FALSE);
}

void CTimeEditValuesDlg::OnAcceleratorTimeViewToggleHelpData1()
{
	if(itsSmartMetDocumentInterface->ShowHelperData1InTimeSerialView())
        itsSmartMetDocumentInterface->ToggleShowHelperDatasInTimeView(kFmiDontShowHelperDataOnTimeSerialView);
	else
        itsSmartMetDocumentInterface->ToggleShowHelperDatasInTimeView(kFmiShowHelperDataOnTimeSerialView);

	Invalidate(FALSE);
}


void CTimeEditValuesDlg::OnAcceleratorTimeViewToggleHelpData3()
{
    if(itsSmartMetDocumentInterface->ShowHelperData3InTimeSerialView())
        itsSmartMetDocumentInterface->ToggleShowHelperDatasInTimeView(kFmiDontShowHelperData3OnTimeSerialView);
    else
        itsSmartMetDocumentInterface->ToggleShowHelperDatasInTimeView(kFmiShowHelperData3OnTimeSerialView);

    Invalidate(FALSE);
}

void CTimeEditValuesDlg::OnAcceleratorTimeViewToggleHelpData4()
{
    if(itsSmartMetDocumentInterface->ShowHelperData4InTimeSerialView())
        itsSmartMetDocumentInterface->ToggleShowHelperDatasInTimeView(kFmiDontShowHelperData4OnTimeSerialView);
    else
        itsSmartMetDocumentInterface->ToggleShowHelperDatasInTimeView(kFmiShowHelperData4OnTimeSerialView);

    Invalidate(FALSE);
}

// Tämä PreTranslateMessage-käsittely mahdollistaa pikanäppäinten käsittelyn (kelluvassa) dialogissa.
BOOL CTimeEditValuesDlg::PreTranslateMessage(MSG* pMsg)
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

void CTimeEditValuesDlg::OnAcceleratorToggleTooltip()
{
	if(itsSmartMetDocumentInterface)
	{
		if(itsSmartMetDocumentInterface->ShowToolTipTimeView())
            itsTimeEditValuesView->HideToolTip(); // jostain syystä aikasarjaikkunassa tooltip ei itsestään häviä (niin kuin muissa ikkunoissa käy)
        itsSmartMetDocumentInterface->ShowToolTipTimeView(!itsSmartMetDocumentInterface->ShowToolTipTimeView());
	}
}

void CTimeEditValuesDlg::OnAcceleratorSwapArea()
{ // tämä on siis SPACE-näppäimen handleri, jolla tehdää aikasarjojen auto säätö
    itsTimeEditValuesView->AutoAdjustValueScales(false);
    itsSmartMetDocumentInterface->TimeSerialViewDirty(true);
	Invalidate(FALSE);
}

void CTimeEditValuesDlg::OnBnClickedCheckTimeSerialAutoAdjust()
{
	if(itsSmartMetDocumentInterface)
	{
		UpdateData(TRUE);
        itsSmartMetDocumentInterface->UseTimeSerialAxisAutoAdjust(fDoAutoAdjust == TRUE);
        itsSmartMetDocumentInterface->TimeSerialViewDirty(true);
		Invalidate(FALSE);
	}
}

void CTimeEditValuesDlg::OnCbnSelchangeComboAnalyzeProducer1()
{
	UpdateData(TRUE);
	auto &analyzeToolData = itsSmartMetDocumentInterface->AnalyzeToolData();
    if(analyzeToolData.ControlPointObservationBlendingData().UseBlendingTool())
    {
        auto &cpObsBlendingToolData = analyzeToolData.ControlPointObservationBlendingData();
        OnComboSelectionChanged(itsProducer1Selector, [&cpObsBlendingToolData](const auto &producerName) { return cpObsBlendingToolData.SelectProducer(producerName); });
    }
    else
    {
        OnComboSelectionChanged(itsProducer1Selector, [&analyzeToolData](const auto &producerName) { return analyzeToolData.SelectProducer1ByName(producerName); });
    }
}

void CTimeEditValuesDlg::OnCbnSelchangeComboAnalyzeProducer2()
{
	UpdateData(TRUE);
	NFmiAnalyzeToolData &analyzeToolData = itsSmartMetDocumentInterface->AnalyzeToolData();
	OnComboSelectionChanged(itsProducer2Selector, [&analyzeToolData](const auto &producerName) { return analyzeToolData.SelectProducer2ByName(producerName); });
}

void CTimeEditValuesDlg::OnComboSelectionChanged(CComboBox &theProducerSelector, SetByName setByName)
{
	int curIndex = theProducerSelector.GetCurSel();
	if(curIndex != CB_ERR)
	{
        CString prodNameU_;
        theProducerSelector.GetLBText(curIndex, prodNameU_);
        std::string tmp = CT2A(prodNameU_);
		setByName(tmp);
	}
	UpdateAnalyseActionControl();
    itsSmartMetDocumentInterface->TimeSerialViewDirty(true);
	Invalidate(FALSE);
}

void CTimeEditValuesDlg::OnBnClickedCheckAnalyzeWithBothData()
{
	UpdateData(TRUE);
    itsSmartMetDocumentInterface->AnalyzeToolData().UseBothProducers(fUseBothProducerData == TRUE);
	UpdateAnalyseActionControl();
}

// Tässä tutkitään analyysi työkalun valintoja, ja tutkitaan
// onko analyysityökalun toiminta mahdollista. Löytyykö dataa ja
// onko datojen ajoissa sellaisia aikoja että modifikaatio on mahdollista.
// Jos toiminta on mahdollinen, enabloidaan Muokkaus-nappula, muuten disabloidaan.
void CTimeEditValuesDlg::UpdateAnalyseActionControl(void)
{
	CWnd *window = GetDlgItem(IDC_BUTTON_TOIMINTO);
	if(window)
	{
		bool enableAnalyseTool = false;
        itsSmartMetDocumentInterface->TimeSerialViewDrawParamList()->Index(itsSmartMetDocumentInterface->TimeSerialViewDrawParamList()->FindEdited());
		boost::shared_ptr<NFmiDrawParam> drawParam = itsSmartMetDocumentInterface->TimeSerialViewDrawParamList()->Current();
		if(drawParam)
		{
			bool noEditMode = (itsSmartMetDocumentInterface->SmartMetEditingMode() != CtrlViewUtils::kFmiEditingModeNormal);
			enableAnalyseTool = noEditMode ? false : itsSmartMetDocumentInterface->AnalyzeToolData().EnableAnalyseTool(*(itsSmartMetDocumentInterface->InfoOrganizer()), *(drawParam->Param().GetParam()));
		}
		else
			enableAnalyseTool = itsSmartMetDocumentInterface->AnalyzeToolData().AnalyzeToolMode() == false;

		if(enableAnalyseTool)
			window->EnableWindow(TRUE);
		else
			window->EnableWindow(FALSE);
	}
}

void CTimeEditValuesDlg::UpdateProducerLists(void)
{
	UpdateData(TRUE);
	NFmiAnalyzeToolData &analyzeToolData = itsSmartMetDocumentInterface->AnalyzeToolData();
	analyzeToolData.SeekProducers(*itsSmartMetDocumentInterface->InfoOrganizer());
    const auto &producers = analyzeToolData.Producers();
    auto isSelectionMadeYet = analyzeToolData.IsSelectionsMadeYet();
	UpdateProducerList(itsProducer1Selector, analyzeToolData.SelectedProducer1(), producers, isSelectionMadeYet);
	UpdateProducerList(itsProducer2Selector, analyzeToolData.SelectedProducer2(), producers, isSelectionMadeYet);
	UpdateData(FALSE);
}

void CTimeEditValuesDlg::UpdateCpObsBlendProducerList(void)
{
    UpdateData(TRUE);
    auto &cpObsBlendingData = itsSmartMetDocumentInterface->AnalyzeToolData().ControlPointObservationBlendingData();
    cpObsBlendingData.SeekProducers(*itsSmartMetDocumentInterface->InfoOrganizer());
    const auto &producers = cpObsBlendingData.Producers();
    // Tietyissä tilanteissa pitää producer selectionia päivittää väkisin (kuten kun ollaan valittu toinen tuottaja CP-pisteiksi).
    // Jos selectionMadeYet on false, tällöin tuottaja otetaan uudestaan cpObsBlendingData:sta.
    bool selectionMadeYet = cpObsBlendingData.IsSelectionMadeYet();
    if(cpObsBlendingData.OverrideSelection())
    {
        selectionMadeYet = false;
        cpObsBlendingData.OverrideSelection(false);
    }
    UpdateProducerList(itsProducer1Selector, cpObsBlendingData.SelectedProducer(), producers, selectionMadeYet);
    UpdateData(FALSE);
}

void CTimeEditValuesDlg::UpdateProducerList(CComboBox &theProducerSelector, const NFmiProducer &theLastSessionProducer, const checkedVector<NFmiProducer> &producerList, bool isSelectionMadeYet)
{
	int lastSelectedProducer = theProducerSelector.GetCurSel();
    CString lastProdNameU_;
	if(lastSelectedProducer >= 0)
        theProducerSelector.GetLBText(lastSelectedProducer, lastProdNameU_);

	// Täytetään sitten lista tuottajista, jotka todellakin löytyivät
	theProducerSelector.ResetContent();
	for(size_t i=0; i < producerList.size(); i++)
	{
		theProducerSelector.AddString(CA2T(producerList[i].GetName()));
	}

	bool selectionMade = false;
	if(isSelectionMadeYet == false)
	{ // yritetään löytää edellisen smartmet session viimeksi valittua
		int index = theProducerSelector.FindString(-1, CA2T(theLastSessionProducer.GetName()));
		if(index != CB_ERR)
		{
			selectionMade = true;
			theProducerSelector.SetCurSel(index);
		}
	}

	if(selectionMade == false)
	{ // toiseksi yritetään viimeksi auto valittua
        int index = theProducerSelector.FindString(-1, lastProdNameU_);
		if(index != CB_ERR)
		{
			selectionMade = true;
			theProducerSelector.SetCurSel(index);
		}
	}

	if(selectionMade == false && theProducerSelector.GetCount())
	{ // kolmanneksi valitaan vain listalta ensimmäinen mikä löytyy
		theProducerSelector.SetCurSel(0);
	}
}


BOOL CTimeEditValuesDlg::OnWndMsg(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
	if(message == ID_MESSAGE_TIME_SERIAL_MODIFICATION_ENDED)
	{
		EnableDlgItem(IDC_BUTTON_TOIMINTO, true); // enabloidaan aikasarja muokkausten jälkeen taas muokkaa-nappi
		EnableButtons();
		RefreshApplicationViews("TimeSerialDlg: Modify edited data");
		return TRUE;
	}

	return CDialog::OnWndMsg(message, wParam, lParam, pResult);
}

HBRUSH CTimeEditValuesDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
    return CDialog::OnCtlColor(pDC, pWnd, nCtlColor);
	// TODO:  Return a different brush if the default is not desired
}


void CTimeEditValuesDlg::OnBnClickedCheckUseMultiProcessCpCalc()
{
	UpdateData(TRUE);
    itsSmartMetDocumentInterface->UseMultiProcessCpCalc(fUseMultiProcessCPCalc == TRUE);
	UpdateControlsAfterMPCPMode();
}


void CTimeEditValuesDlg::OnBnClickedCheckAllowRightClickSelection()
{
    UpdateData(TRUE);
    itsSmartMetDocumentInterface->AllowRightClickDisplaySelection(fAllowRightClickSelection == TRUE);
    itsSmartMetDocumentInterface->MapDirty(0, true, true);
    itsSmartMetDocumentInterface->TimeSerialViewDirty(true);
    itsSmartMetDocumentInterface->RefreshApplicationViewsAndDialogs("TimeSerialDlg: Toggle Allow right click selection setting");
    Invalidate(FALSE);
}

void CTimeEditValuesDlg::HandleCpAccelerator(ControlPointAcceleratorActions action, const std::string &updateMessage)
{
    if(itsSmartMetDocumentInterface->MakeControlPointAcceleratorAction(action, updateMessage))
    {
        Invalidate(FALSE);
    }
}

void CTimeEditValuesDlg::OnAcceleratorCpSelectNext()
{
    HandleCpAccelerator(ControlPointAcceleratorActions::Next, "TimeSerialDlg: Select next Control-point"s);
}

void CTimeEditValuesDlg::OnAcceleratorCpSelectPrevious()
{
    HandleCpAccelerator(ControlPointAcceleratorActions::Previous, "TimeSerialDlg: Select previous Control-point"s);
}

void CTimeEditValuesDlg::OnAcceleratorCpSelectLeft()
{
    HandleCpAccelerator(ControlPointAcceleratorActions::Left, "TimeSerialDlg: Select nearest left Control-point"s);
}

void CTimeEditValuesDlg::OnAcceleratorCpSelectRight()
{
    HandleCpAccelerator(ControlPointAcceleratorActions::Right, "TimeSerialDlg: Select nearest right Control-point"s);
}

void CTimeEditValuesDlg::OnAcceleratorCpSelectUp()
{
    HandleCpAccelerator(ControlPointAcceleratorActions::Up, "TimeSerialDlg: Select nearest upward Control-point"s);
}

void CTimeEditValuesDlg::OnAcceleratorCpSelectDown()
{
    HandleCpAccelerator(ControlPointAcceleratorActions::Down, "TimeSerialDlg: Select nearest downward Control-point"s);
}
