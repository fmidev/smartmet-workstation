#include "stdafx.h"
#include "FmiSynopPlotSettingsDlg.h"
#include "SmartMetDocumentInterface.h"
#include "NFmiSynopStationPrioritySystem.h"
#include "NFmiStringTools.h"
#include "NFmiInfoData.h"
#include "NFmiInfoOrganizer.h"
#include "NFmiFastQueryInfo.h"
#include "NFmiDictionaryFunction.h"
#include "NFmiProducerName.h"
#include "FmiWin32Helpers.h"
#include "CloneBitmap.h"
#include "NFmiSynopPlotSettings.h"
#include "CtrlViewFunctions.h"
#include "CtrlViewGdiPlusFunctions.h"
#include "CtrlViewWin32Functions.h"
#include "execute-command-in-separate-process.h"

NFmiPlotRelatedControls::NFmiPlotRelatedControls() = default;

NFmiPlotRelatedControls::~NFmiPlotRelatedControls()
{
	CtrlView::DestroyBitmap(&itsSingleColorBitmap, true);
}

// CFmiSynopPlotSettingsDlg dialog

IMPLEMENT_DYNAMIC(CFmiSynopPlotSettingsDlg, CDialog)
CFmiSynopPlotSettingsDlg::CFmiSynopPlotSettingsDlg(SmartMetDocumentInterface *smartMetDocumentInterface, CWnd* pParent /*=NULL*/)
	: CDialog(CFmiSynopPlotSettingsDlg::IDD, pParent)
	,itsSmartMetDocumentInterface(smartMetDocumentInterface)
	, itsSynopControls()
{
}

CFmiSynopPlotSettingsDlg::~CFmiSynopPlotSettingsDlg()
{
}

void CFmiSynopPlotSettingsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_SLIDER_SYNOP_FONT_SIZE, itsSynopControls.itsFontSizeSlider);
	DDX_Control(pDX, IDC_SLIDER_SYNOP_PLOT_SPACING, itsSynopControls.itsPlotSpacingSlider);
	DDX_Control(pDX, IDC_BUTTON_SYNOP_SINGLE_COLOR, itsSynopControls.itsSingleColorChangeButtom);
	DDX_Check(pDX, IDC_CHECK_SHOW_V, fShowV);
	DDX_Check(pDX, IDC_CHECK_SHOW_T, fShowT);
	DDX_Check(pDX, IDC_CHECK_SHOW_WW, fShowWw);
	DDX_Check(pDX, IDC_CHECK_SHOW_TD, fShowTd);
	DDX_Check(pDX, IDC_CHECK_SHOW_CH, fShowCh);
	DDX_Check(pDX, IDC_CHECK_SHOW_CM, fShowCm);
	DDX_Check(pDX, IDC_CHECK_SHOW_N, fShowN);
	DDX_Check(pDX, IDC_CHECK_SHOW_DDFF, fShowDdff);
	DDX_Check(pDX, IDC_CHECK_SHOW_CL, fShowCl);
	DDX_Check(pDX, IDC_CHECK_SHOW_H, fShowH);
	DDX_Check(pDX, IDC_CHECK_SHOW_RR, fShowRr);
	DDX_Check(pDX, IDC_CHECK_SHOW_PPPP, fShowPPPP);
	DDX_Check(pDX, IDC_CHECK_SHOW_PPP, fShowPpp);
	DDX_Check(pDX, IDC_CHECK_SHOW_NH, fShowNh);
	DDX_Check(pDX, IDC_CHECK_SHOW_TW, fShowTw);
	DDX_Check(pDX, IDC_CHECK_SHOW_A, fShowA);
	DDX_Check(pDX, IDC_CHECK_SHOW_W1, fShowW1);
	DDX_Check(pDX, IDC_CHECK_SHOW_W2, fShowW2);
	DDX_Check(pDX, IDC_CHECK_SHOW_SYNOP_SINGLE_COLOR, itsSynopControls.fSingleColor);
	DDX_Check(pDX, IDC_CHECK_TOGGLE_ALL_SYNOP_PARAMS, itsSynopControls.fSetAllParamState);
}


BEGIN_MESSAGE_MAP(CFmiSynopPlotSettingsDlg, CDialog)
	ON_BN_CLICKED(IDOK, OnBnClickedOk)
	ON_BN_CLICKED(IDC_BUTTON_REFRESH, OnBnClickedButtonRefresh)
	ON_BN_CLICKED(IDC_BUTTON_STATION_PRIORITIES, OnBnClickedButtonStationPriorities)
	ON_BN_CLICKED(IDC_BUTTON_SYNOP_SINGLE_COLOR, OnBnClickedButtonSynopSingleColor)
	ON_WM_HSCROLL()
	ON_BN_CLICKED(IDC_CHECK_TOGGLE_ALL_SYNOP_PARAMS, OnBnClickedCheckToggleAllSynopParams)
END_MESSAGE_MAP()


// CFmiSynopPlotSettingsDlg message handlers

BOOL CFmiSynopPlotSettingsDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	CFmiWin32Helpers::SetUsedWindowIconDynamically(this);
	itsSynopControls.itsFontSizeSlider.SetRange(10, 100); // 1-10 mm, jaetaan kymmenellä
	itsSynopControls.itsPlotSpacingSlider.SetRange(0, 20); // arvot 0 - 2, jaetaan kymmenellä

	InitDialogTexts();
	InitFromDoc();
	itsSynopControls.fSetAllParamState = true;
	UpdateData(FALSE);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CFmiSynopPlotSettingsDlg::InitFromDoc(void)
{
	NFmiSynopPlotSettings & synopSettings = *(itsSmartMetDocumentInterface->SynopPlotSettings());
	fShowT = synopSettings.ShowT();
	fShowTd = synopSettings.ShowTd();
	fShowV = synopSettings.ShowV();
	fShowWw = synopSettings.ShowWw();
	fShowCh = synopSettings.ShowCh();
	fShowCm = synopSettings.ShowCm();
	fShowCl = synopSettings.ShowCl();
	fShowN = synopSettings.ShowN();
	fShowNh = synopSettings.ShowNh();
	fShowDdff = synopSettings.ShowDdff();
	fShowPPPP = synopSettings.ShowPPPP();
	fShowPpp = synopSettings.ShowPpp();
	fShowA = synopSettings.ShowA();
	fShowW1 = synopSettings.ShowW1();
	fShowW2 = synopSettings.ShowW2();
	fShowH = synopSettings.ShowH();
	fShowRr = synopSettings.ShowRr();
	fShowTw = synopSettings.ShowTw();
	itsSynopControls.fSingleColor = synopSettings.UseSingleColor();

	itsSynopControls.itsFontSizeSlider.SetPos(static_cast<int>(synopSettings.FontSize() * 10));
	itsSynopControls.itsPlotSpacingSlider.SetPos(static_cast<int>(synopSettings.PlotSpacing() * 10));

	UpdateFontSizeString();
	UpdatePlotSpacingString();

	NFmiColor tColor = synopSettings.SingleColor();
	itsSynopControls.itsSingleColorRef = CtrlView::Color2ColorRef(tColor);
	CtrlView::InitialButtonColorUpdate(NFmiColorButtonDrawingData(this, itsSynopControls.itsSingleColorRef, &itsSynopControls.itsSingleColorBitmap, itsSynopControls.itsSingleColorRect, itsSynopControls.itsSingleColorChangeButtom));

	UpdateData(FALSE);
}

void CFmiSynopPlotSettingsDlg::StoreToDoc(void)
{
	UpdateData(TRUE);
	NFmiSynopPlotSettings & synopSettings = *(itsSmartMetDocumentInterface->SynopPlotSettings());
	synopSettings.ShowT(fShowT == TRUE);
	synopSettings.ShowTd(fShowTd == TRUE);
	synopSettings.ShowV(fShowV == TRUE);
	synopSettings.ShowWw(fShowWw == TRUE);
	synopSettings.ShowCh(fShowCh == TRUE);
	synopSettings.ShowCm(fShowCm == TRUE);
	synopSettings.ShowCl(fShowCl == TRUE);
	synopSettings.ShowN(fShowN == TRUE);
	synopSettings.ShowNh(fShowNh == TRUE);
	synopSettings.ShowDdff(fShowDdff == TRUE);
	synopSettings.ShowPPPP(fShowPPPP == TRUE);
	synopSettings.ShowPpp(fShowPpp == TRUE);
	synopSettings.ShowA(fShowA == TRUE);
	synopSettings.ShowW1(fShowW1 == TRUE);
	synopSettings.ShowW2(fShowW2 == TRUE);
	synopSettings.ShowH(fShowH == TRUE);
	synopSettings.ShowRr(fShowRr == TRUE);
	synopSettings.ShowTw(fShowTw == TRUE);
	synopSettings.UseSingleColor(itsSynopControls.fSingleColor == TRUE);

	synopSettings.FontSize(itsSynopControls.itsFontSizeSlider.GetPos() / 10.);
	synopSettings.PlotSpacing(itsSynopControls.itsPlotSpacingSlider.GetPos() / 10.);

	NFmiColor tColor;
	tColor.SetRGB((float(GetRValue(itsSynopControls.itsSingleColorRef))/float(255.0)),
				  (float(GetGValue(itsSynopControls.itsSingleColorRef))/float(255.0)),
				  (float(GetBValue(itsSynopControls.itsSingleColorRef))/float(255.0)));
	synopSettings.SingleColor(tColor);

	synopSettings.Store();
}

void CFmiSynopPlotSettingsDlg::Update(void)
{
    if(IsWindowVisible() && !IsIconic()) // Näyttöä päivitetään vain jos se on näkyvissä ja se ei ole minimized tilassa
    {
        InitFromDoc();

        NFmiInfoData::Type synopType = NFmiInfoData::kObservations;
        boost::shared_ptr<NFmiFastQueryInfo> editedInfo = itsSmartMetDocumentInterface->EditedSmartInfo();
        if(editedInfo && editedInfo->Producer()->GetIdent() == kFmiSYNOP)
        {
            synopType = NFmiInfoData::kEditable; // jos salama data editoitavana, pitää tyypiksi laittaa editable
        }
        auto *infoOrganizer = itsSmartMetDocumentInterface->InfoOrganizer();
        boost::shared_ptr<NFmiFastQueryInfo> info = infoOrganizer->FindInfo(synopType, NFmiProducer(kFmiSYNOP), true);
        if(info)
        {
            NFmiParamBag params = infoOrganizer->GetParams(kFmiSYNOP);
            // Jos synop-dataa löytyy, laita check boxien enable/disable riippuen, löytyykö kyseistä parametria synop-datasta
            EnableCheckBox(params, kFmiTemperature, IDC_CHECK_SHOW_T);
            EnableCheckBox(params, kFmiVisibility, IDC_CHECK_SHOW_V);
            EnableCheckBox(params, kFmiPresentWeather, IDC_CHECK_SHOW_WW);
            EnableCheckBox(params, kFmiDewPoint, IDC_CHECK_SHOW_TD);
            EnableCheckBox(params, kFmiHighCloudType, IDC_CHECK_SHOW_CH);
            EnableCheckBox(params, kFmiMiddleCloudType, IDC_CHECK_SHOW_CM);
            EnableCheckBox(params, kFmiTotalCloudCover, IDC_CHECK_SHOW_N);
            EnableCheckBox(params, kFmiWindSpeedMS, IDC_CHECK_SHOW_DDFF); // kokeillaan vain WS:ää testauksessa
            EnableCheckBox(params, kFmiLowCloudType, IDC_CHECK_SHOW_CL);
            EnableCheckBox(params, kFmiCloudHeight, IDC_CHECK_SHOW_H);
            EnableCheckBox(params, kFmiPrecipitationAmount, IDC_CHECK_SHOW_RR);
            EnableCheckBox(params, kFmiPressure, IDC_CHECK_SHOW_PPPP);
            EnableCheckBox(params, kFmiPressure, IDC_CHECK_SHOW_PPP); // pressurechange parametri lasketaankin paineen avulla
            EnableCheckBox(params, kFmiLowCloudCover, IDC_CHECK_SHOW_NH);
            EnableCheckBox(params, kFmiTemperatureSea, IDC_CHECK_SHOW_TW);
            EnableCheckBox(params, kFmiPressureTendency, IDC_CHECK_SHOW_A);
            EnableCheckBox(params, kFmiPastWeather1, IDC_CHECK_SHOW_W1);
            EnableCheckBox(params, kFmiPastWeather2, IDC_CHECK_SHOW_W2);
        }
        else
        {
            EnableCheckBox(FALSE, IDC_CHECK_SHOW_T);
            EnableCheckBox(FALSE, IDC_CHECK_SHOW_V);
            EnableCheckBox(FALSE, IDC_CHECK_SHOW_WW);
            EnableCheckBox(FALSE, IDC_CHECK_SHOW_TD);
            EnableCheckBox(FALSE, IDC_CHECK_SHOW_CH);
            EnableCheckBox(FALSE, IDC_CHECK_SHOW_CM);
            EnableCheckBox(FALSE, IDC_CHECK_SHOW_N);
            EnableCheckBox(FALSE, IDC_CHECK_SHOW_DDFF);
            EnableCheckBox(FALSE, IDC_CHECK_SHOW_CL);
            EnableCheckBox(FALSE, IDC_CHECK_SHOW_H);
            EnableCheckBox(FALSE, IDC_CHECK_SHOW_RR);
            EnableCheckBox(FALSE, IDC_CHECK_SHOW_PPPP);
            EnableCheckBox(FALSE, IDC_CHECK_SHOW_PPP);
            EnableCheckBox(FALSE, IDC_CHECK_SHOW_NH);
            EnableCheckBox(FALSE, IDC_CHECK_SHOW_TW);
            EnableCheckBox(FALSE, IDC_CHECK_SHOW_A);
            EnableCheckBox(FALSE, IDC_CHECK_SHOW_W1);
            EnableCheckBox(FALSE, IDC_CHECK_SHOW_W2);
        }
    }
}

void CFmiSynopPlotSettingsDlg::EnableCheckBox(NFmiParamBag &theParams, int theParId, int theCheckBoxIdNumber)
{
	CWnd *win = GetDlgItem(theCheckBoxIdNumber);
	if(win)
	{
		if(theParams.SetCurrent(static_cast<FmiParameterName>(theParId), false))
			win->EnableWindow(TRUE);
		else
			win->EnableWindow(FALSE);
	}
}

void CFmiSynopPlotSettingsDlg::EnableCheckBox(boost::shared_ptr<NFmiFastQueryInfo> &theInfo, int theParId, int theCheckBoxIdNumber)
{
	CWnd *win = GetDlgItem(theCheckBoxIdNumber);
	if(win)
	{
		if(theInfo->Param(static_cast<FmiParameterName>(theParId)))
			win->EnableWindow(TRUE);
		else
			win->EnableWindow(FALSE);
	}
}

void CFmiSynopPlotSettingsDlg::EnableCheckBox(BOOL theState, int theCheckBoxIdNumber)
{
	CWnd *win = GetDlgItem(theCheckBoxIdNumber);
	if(win)
		win->EnableWindow(theState);
}

void CFmiSynopPlotSettingsDlg::UpdateFontSizeString(void)
{
	CWnd *win = GetDlgItem(IDC_STATIC_FONT_SIZE_STR);
	if(win)
	{
		std::string str = NFmiStringTools::Convert(itsSynopControls.itsFontSizeSlider.GetPos() / 10.);
		win->SetWindowText(CA2T(str.c_str()));
	}
}

void CFmiSynopPlotSettingsDlg::UpdatePlotSpacingString(void)
{
	CWnd *win = GetDlgItem(IDC_STATIC_PLOT_SPACING_STR);
	if(win)
	{
		std::string str = NFmiStringTools::Convert(itsSynopControls.itsPlotSpacingSlider.GetPos() / 10.);
        win->SetWindowText(CA2T(str.c_str()));
	}
}

void CFmiSynopPlotSettingsDlg::OnBnClickedOk()
{
	StoreToDoc();
	RefreshApplicationViews(__FUNCTION__);
	OnOK();
}

void CFmiSynopPlotSettingsDlg::OnBnClickedButtonRefresh()
{
	StoreToDoc();
	RefreshApplicationViews(__FUNCTION__);
}

void CFmiSynopPlotSettingsDlg::RefreshApplicationViews(const std::string &reasonForUpdate)
{
	// luetaan varmuuden vuoksi priorisointi lista uusiksi, jos ollaan muokattu asema listaa
    auto *synopStationPrioritySystem = itsSmartMetDocumentInterface->SynopStationPrioritySystem();
    synopStationPrioritySystem->Init(synopStationPrioritySystem->InitFileName());
    itsSmartMetDocumentInterface->MapViewDirty(CtrlViewUtils::kDoAllMapViewDescTopIndex, false, true, true, false, false, false);
    itsSmartMetDocumentInterface->RefreshApplicationViewsAndDialogs(reasonForUpdate, SmartMetViewId::AllMapViews);
}

void CFmiSynopPlotSettingsDlg::OnBnClickedButtonStationPriorities()
{

	std::string commandStr("notepad ");
	commandStr += itsSmartMetDocumentInterface->SynopStationPrioritySystem()->InitFileName();
    CFmiProcessHelpers::ExecuteCommandInSeparateProcess(commandStr);
}

void CFmiSynopPlotSettingsDlg::OnBnClickedButtonSynopSingleColor()
{
    CtrlView::ColorButtonPressed(NFmiColorButtonDrawingData(this, itsSynopControls.itsSingleColorRef, &itsSynopControls.itsSingleColorBitmap, itsSynopControls.itsSingleColorRect, itsSynopControls.itsSingleColorChangeButtom));
}

void CFmiSynopPlotSettingsDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	UpdateFontSizeString();
	UpdatePlotSpacingString();
	UpdateData(TRUE);

	CDialog::OnHScroll(nSBCode, nPos, pScrollBar);
}

// Tämä funktio alustaa kaikki dialogin tekstit editoriin valitulla kielellä.
// Tämä on ikävä kyllä tehtävä erikseen dialogin muokkaus työkalusta, eli
// tekijän pitää lisätä erikseen tänne kaikki dialogin osat, joihin
// kieli valinta voi vaikuttaa.
void CFmiSynopPlotSettingsDlg::InitDialogTexts(void)
{
    SetWindowText(CA2T(::GetDictionaryString("SynopPlotSettingsDlgTitle").c_str()));

	CFmiWin32Helpers::SetDialogItemText(this, IDOK, "IDOK");
	CFmiWin32Helpers::SetDialogItemText(this, IDCANCEL, "IDCANCEL");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_BUTTON_REFRESH, "IDC_BUTTON_REFRESH");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_BUTTON_STATION_PRIORITIES, "IDC_BUTTON_STATION_PRIORITIES");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_CHECK_SHOW_SYNOP_SINGLE_COLOR, "Show with single color");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_SYNOP_PLOT_FONT_SIZE_STR, "Font size [mm]");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_SYNOP_PLOT_SPACING, "Plot spacing (0 - 2)");
}

void CFmiSynopPlotSettingsDlg::OnBnClickedCheckToggleAllSynopParams()
{
	UpdateData(TRUE);

	SetAllSynopParamStates(itsSynopControls.fSetAllParamState);
}

void CFmiSynopPlotSettingsDlg::SetAllSynopParamStates(BOOL newState)
{
	UpdateData(TRUE);

	fShowT = newState;
	fShowTd = newState;
	fShowV = newState;
	fShowWw = newState;
	fShowCh = newState;
	fShowCm = newState;
	fShowCl = newState;
	fShowN = newState;
	fShowNh = newState;
	fShowDdff = newState;
	fShowPPPP = newState;
	fShowPpp = newState;
	fShowA = newState;
	fShowW1 = newState;
	fShowW2 = newState;
	fShowH = newState;
	fShowRr = newState;
	fShowTw = newState;

	UpdateData(FALSE);
}
