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
	DDX_Check(pDX, IDC_CHECK_SHOW_METAR_SKYINFO, fMetarShow_SkyInfo);
	DDX_Check(pDX, IDC_CHECK_SHOW_METAR_TT, fMetarShow_TT);
	DDX_Check(pDX, IDC_CHECK_SHOW_METAR_TdTd, fMetarShow_TdTd);
	DDX_Check(pDX, IDC_CHECK_SHOW_METAR_PhPhPhPh, fMetarShow_PhPhPhPh);
	DDX_Check(pDX, IDC_CHECK_SHOW_METAR_dddff, fMetarShow_dddff);
	DDX_Check(pDX, IDC_CHECK_SHOW_METAR_Gff, fMetarShow_Gff);
	DDX_Check(pDX, IDC_CHECK_SHOW_METAR_VVVV, fMetarShow_VVVV);
	DDX_Check(pDX, IDC_CHECK_SHOW_METAR_ww, fMetarShow_ww);
	DDX_Check(pDX, IDC_CHECK_SHOW_METAR_STATUS, fMetarShow_Status);
	DDX_Control(pDX, IDC_SLIDER_METAR_FONT_SIZE, itsMetarControls.itsFontSizeSlider);
	DDX_Control(pDX, IDC_SLIDER_METAR_PLOT_SPACING, itsMetarControls.itsPlotSpacingSlider);
	DDX_Control(pDX, IDC_BUTTON_METAR_SINGLE_COLOR, itsMetarControls.itsSingleColorChangeButtom);
	DDX_Check(pDX, IDC_CHECK_SHOW_METAR_SINGLE_COLOR, itsMetarControls.fSingleColor);
	DDX_Check(pDX, IDC_CHECK_TOGGLE_ALL_METAR_PARAMS, itsMetarControls.fSetAllParamState);
}


BEGIN_MESSAGE_MAP(CFmiSynopPlotSettingsDlg, CDialog)
	ON_BN_CLICKED(IDOK, OnBnClickedOk)
	ON_BN_CLICKED(IDC_BUTTON_REFRESH, OnBnClickedButtonRefresh)
	ON_BN_CLICKED(IDC_BUTTON_STATION_PRIORITIES, OnBnClickedButtonStationPriorities)
	ON_BN_CLICKED(IDC_BUTTON_SYNOP_SINGLE_COLOR, OnBnClickedButtonSynopSingleColor)
	ON_WM_HSCROLL()
	ON_BN_CLICKED(IDC_CHECK_TOGGLE_ALL_SYNOP_PARAMS, OnBnClickedCheckToggleAllSynopParams)
	ON_BN_CLICKED(IDC_BUTTON_METAR_SINGLE_COLOR, OnBnClickedButtonMetarSingleColor)
	ON_BN_CLICKED(IDC_CHECK_TOGGLE_ALL_METAR_PARAMS, OnBnClickedCheckToggleAllMetarParams)
END_MESSAGE_MAP()


// CFmiSynopPlotSettingsDlg message handlers

BOOL CFmiSynopPlotSettingsDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	CFmiWin32Helpers::SetUsedWindowIconDynamically(this);

	InitDialogTexts();
	InitFromDoc();
	UpdateData(FALSE);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CFmiSynopPlotSettingsDlg::InitPlotControls(bool synopPlotCase, bool useSingleColor, double fontSize, double plotSpacing, const NFmiColor & singleColor, NFmiPlotRelatedControls &plotControls)
{
	plotControls.itsFontSizeSlider.SetRange(10, 100); // 1-10 mm, jaetaan kymmenellä
	plotControls.itsPlotSpacingSlider.SetRange(0, 20); // arvot 0 - 2, jaetaan kymmenellä
	plotControls.fSetAllParamState = true;

	plotControls.fSingleColor = useSingleColor;
	plotControls.itsFontSizeSlider.SetPos(static_cast<int>(fontSize * 10));
	plotControls.itsPlotSpacingSlider.SetPos(static_cast<int>(plotSpacing * 10));
	UpdateFontSizeString(synopPlotCase);
	UpdatePlotSpacingString(synopPlotCase);
	plotControls.itsSingleColorRef = CtrlView::Color2ColorRef(singleColor);
	CtrlView::InitialButtonColorUpdate(NFmiColorButtonDrawingData(this, plotControls.itsSingleColorRef, &plotControls.itsSingleColorBitmap, plotControls.itsSingleColorRect, plotControls.itsSingleColorChangeButtom));
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
	InitPlotControls(true, synopSettings.UseSingleColor(), synopSettings.FontSize(), synopSettings.PlotSpacing(), synopSettings.SingleColor(), itsSynopControls);

	NFmiMetarPlotSettings& metarSettings = synopSettings.MetarPlotSettings();
	fMetarShow_SkyInfo = metarSettings.Show_SkyInfo();
	fMetarShow_TT = metarSettings.Show_TT();
	fMetarShow_TdTd = metarSettings.Show_TdTd();
	fMetarShow_PhPhPhPh = metarSettings.Show_PhPhPhPh();
	fMetarShow_dddff = metarSettings.Show_dddff();
	fMetarShow_Gff = metarSettings.Show_Gff();
	fMetarShow_VVVV = metarSettings.Show_VVVV();
	fMetarShow_ww = metarSettings.Show_ww();
	fMetarShow_Status = metarSettings.Show_Status();
	InitPlotControls(false, metarSettings.UseSingleColor(), metarSettings.FontSize(), metarSettings.PlotSpacing(), metarSettings.SingleColor(), itsMetarControls);

	UpdateData(FALSE);
}

template<typename PlotSettings>
void StoreToDoc(const NFmiPlotRelatedControls& plotControls, PlotSettings& plotSettings)
{
	plotSettings.UseSingleColor(plotControls.fSingleColor == TRUE);
	plotSettings.FontSize(plotControls.itsFontSizeSlider.GetPos() / 10.);
	plotSettings.PlotSpacing(plotControls.itsPlotSpacingSlider.GetPos() / 10.);
	NFmiColor tColor;
	tColor.SetRGB((float(GetRValue(plotControls.itsSingleColorRef)) / float(255.0)),
		(float(GetGValue(plotControls.itsSingleColorRef)) / float(255.0)),
		(float(GetBValue(plotControls.itsSingleColorRef)) / float(255.0)));
	plotSettings.SingleColor(tColor);
	plotSettings.Store();
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
	::StoreToDoc(itsSynopControls, synopSettings);

	NFmiMetarPlotSettings& metarSettings = synopSettings.MetarPlotSettings();
	metarSettings.Show_SkyInfo(fMetarShow_SkyInfo == TRUE);
	metarSettings.Show_TT(fMetarShow_TT == TRUE);
	metarSettings.Show_TdTd(fMetarShow_TdTd == TRUE);
	metarSettings.Show_PhPhPhPh(fMetarShow_PhPhPhPh == TRUE);
	metarSettings.Show_dddff(fMetarShow_dddff == TRUE);
	metarSettings.Show_Gff(fMetarShow_Gff == TRUE);
	metarSettings.Show_VVVV(fMetarShow_VVVV == TRUE);
	metarSettings.Show_ww(fMetarShow_ww == TRUE);
	metarSettings.Show_Status(fMetarShow_Status == TRUE);
	::StoreToDoc(itsMetarControls, metarSettings);
}

void CFmiSynopPlotSettingsDlg::Update(void)
{
    InitFromDoc();
}

void CFmiSynopPlotSettingsDlg::UpdateFontSizeString(bool synopPlotCase)
{
	auto usedControlId = synopPlotCase ? IDC_STATIC_SYNOP_FONT_SIZE_STR : IDC_STATIC_METAR_FONT_SIZE_STR;
	CWnd *win = GetDlgItem(usedControlId);
	if(win)
	{
		auto& usedPlotControl = synopPlotCase ? itsSynopControls : itsMetarControls;
		std::string str = NFmiStringTools::Convert(usedPlotControl.itsFontSizeSlider.GetPos() / 10.);
		win->SetWindowText(CA2T(str.c_str()));
	}
}

void CFmiSynopPlotSettingsDlg::UpdatePlotSpacingString(bool synopPlotCase)
{
	auto usedControlId = synopPlotCase ? IDC_STATIC_SYNOP_PLOT_SPACING_STR : IDC_STATIC_METAR_PLOT_SPACING_STR;
	CWnd *win = GetDlgItem(usedControlId);
	if(win)
	{
		auto& usedPlotControl = synopPlotCase ? itsSynopControls : itsMetarControls;
		std::string str = NFmiStringTools::Convert(usedPlotControl.itsPlotSpacingSlider.GetPos() / 10.);
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

void CFmiSynopPlotSettingsDlg::OnBnClickedButtonMetarSingleColor()
{
	CtrlView::ColorButtonPressed(NFmiColorButtonDrawingData(this, itsMetarControls.itsSingleColorRef, &itsMetarControls.itsSingleColorBitmap, itsMetarControls.itsSingleColorRect, itsMetarControls.itsSingleColorChangeButtom));
}

void CFmiSynopPlotSettingsDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	// Käydään kaikki mnahdolliset kontrollit läpi synop + metar ja niissä font-size ja plot-spacing
	UpdateFontSizeString(true);
	UpdatePlotSpacingString(true);
	UpdateFontSizeString(false);
	UpdatePlotSpacingString(false);
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

void CFmiSynopPlotSettingsDlg::OnBnClickedCheckToggleAllMetarParams()
{
	UpdateData(TRUE);

	SetAllMetarParamStates(itsMetarControls.fSetAllParamState);
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

void CFmiSynopPlotSettingsDlg::SetAllMetarParamStates(BOOL newState)
{
	UpdateData(TRUE);

	fMetarShow_SkyInfo = newState;
	fMetarShow_TT = newState;
	fMetarShow_TdTd = newState;
	fMetarShow_PhPhPhPh = newState;
	fMetarShow_dddff = newState;
	fMetarShow_Gff = newState;
	fMetarShow_VVVV = newState;
	fMetarShow_ww = newState;
	fMetarShow_Status = newState;

	UpdateData(FALSE);
}
