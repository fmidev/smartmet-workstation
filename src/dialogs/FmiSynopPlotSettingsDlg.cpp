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

// CFmiSynopPlotSettingsDlg dialog

IMPLEMENT_DYNAMIC(CFmiSynopPlotSettingsDlg, CDialog)
CFmiSynopPlotSettingsDlg::CFmiSynopPlotSettingsDlg(SmartMetDocumentInterface *smartMetDocumentInterface, CWnd* pParent /*=NULL*/)
	: CDialog(CFmiSynopPlotSettingsDlg::IDD, pParent)
	,itsSmartMetDocumentInterface(smartMetDocumentInterface)
	,itsSingleColorBitmap(0)
	,itsSingleColorRef(0)
	, fSetAllParamState(FALSE)
{
}

CFmiSynopPlotSettingsDlg::~CFmiSynopPlotSettingsDlg()
{
	if(itsSingleColorBitmap)
		itsSingleColorBitmap->DeleteObject();
	delete itsSingleColorBitmap;
}

void CFmiSynopPlotSettingsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_SLIDER_FONT_SIZE, itsFontSizeSlider);
	DDX_Control(pDX, IDC_SLIDER_PLOT_SPACING, itsPlotSpacingSlider);
	DDX_Control(pDX, IDC_BUTTON_SINGLE_COLOR, itsSingleColorChangeButtom);
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
	DDX_Check(pDX, IDC_CHECK_SHOW_SINGLE_COLOR, fSingleColor);
	DDX_Check(pDX, IDC_CHECK_TOGGLE_ALL_PARAMS, fSetAllParamState);
}


BEGIN_MESSAGE_MAP(CFmiSynopPlotSettingsDlg, CDialog)
	ON_BN_CLICKED(IDOK, OnBnClickedOk)
	ON_BN_CLICKED(IDC_BUTTON_REFRESH, OnBnClickedButtonRefresh)
	ON_BN_CLICKED(IDC_BUTTON_STATION_PRIORITIES, OnBnClickedButtonStationPriorities)
	ON_BN_CLICKED(IDC_CHECK_SHOW_V, OnBnClickedCheckShowV)
	ON_BN_CLICKED(IDC_CHECK_SHOW_T, OnBnClickedCheckShowT)
	ON_BN_CLICKED(IDC_CHECK_SHOW_WW, OnBnClickedCheckShowWw)
	ON_BN_CLICKED(IDC_CHECK_SHOW_TD, OnBnClickedCheckShowTd)
	ON_BN_CLICKED(IDC_CHECK_SHOW_CH, OnBnClickedCheckShowCh)
	ON_BN_CLICKED(IDC_CHECK_SHOW_CM, OnBnClickedCheckShowCm)
	ON_BN_CLICKED(IDC_CHECK_SHOW_N, OnBnClickedCheckShowN)
	ON_BN_CLICKED(IDC_CHECK_SHOW_DDFF, OnBnClickedCheckShowDdff)
	ON_BN_CLICKED(IDC_CHECK_SHOW_CL, OnBnClickedCheckShowCl)
	ON_BN_CLICKED(IDC_CHECK_SHOW_H, OnBnClickedCheckShowH)
	ON_BN_CLICKED(IDC_CHECK_SHOW_RR, OnBnClickedCheckShowRr)
	ON_BN_CLICKED(IDC_CHECK_SHOW_PPPP, OnBnClickedCheckShowPppp)
	ON_BN_CLICKED(IDC_CHECK_SHOW_PPP, OnBnClickedCheckShowPpp)
	ON_BN_CLICKED(IDC_CHECK_SHOW_NH, OnBnClickedCheckShowNh)
	ON_BN_CLICKED(IDC_CHECK_SHOW_TW, OnBnClickedCheckShowTw)
	ON_BN_CLICKED(IDC_CHECK_SHOW_A, OnBnClickedCheckShowA)
	ON_BN_CLICKED(IDC_CHECK_SHOW_W1, OnBnClickedCheckShowW1)
	ON_BN_CLICKED(IDC_CHECK_SHOW_W2, OnBnClickedCheckShowW2)
	ON_BN_CLICKED(IDC_CHECK_SHOW_SINGLE_COLOR, OnBnClickedCheckShowSingleColor)
	ON_BN_CLICKED(IDC_BUTTON_SINGLE_COLOR, OnBnClickedButtonSingleColor)
	ON_WM_HSCROLL()
	ON_BN_CLICKED(IDC_CHECK_TOGGLE_ALL_PARAMS, OnBnClickedCheckToggleAllParams)
END_MESSAGE_MAP()


// CFmiSynopPlotSettingsDlg message handlers

BOOL CFmiSynopPlotSettingsDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	CFmiWin32Helpers::SetUsedWindowIconDynamically(this);
	itsFontSizeSlider.SetRange(10, 100); // 1-10 mm, jaetaan kymmenellä
	itsPlotSpacingSlider.SetRange(0, 20); // arvot 0 - 2, jaetaan kymmenellä

	InitDialogTexts();
	InitFromDoc();
	fSetAllParamState = true;
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
	fSingleColor = synopSettings.UseSingleColor();

	itsFontSizeSlider.SetPos(static_cast<int>(synopSettings.FontSize() * 10));
	itsPlotSpacingSlider.SetPos(static_cast<int>(synopSettings.PlotSpacing() * 10));

	UpdateFontSizeString();
	UpdatePlotSpacingString();

	NFmiColor tColor = synopSettings.SingleColor();
	itsSingleColorRef = CtrlView::Color2ColorRef(tColor);
	CtrlView::InitialButtonColorUpdate(this, itsSingleColorRef, &itsSingleColorBitmap, itsSingleColorRect, itsSingleColorChangeButtom);

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
	synopSettings.UseSingleColor(fSingleColor == TRUE);

	synopSettings.FontSize(itsFontSizeSlider.GetPos() / 10.);
	synopSettings.PlotSpacing(itsPlotSpacingSlider.GetPos() / 10.);

	NFmiColor tColor;
	tColor.SetRGB((float(GetRValue(itsSingleColorRef))/float(255.0)),
				  (float(GetGValue(itsSingleColorRef))/float(255.0)),
				  (float(GetBValue(itsSingleColorRef))/float(255.0)));
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
		std::string str = NFmiStringTools::Convert(itsFontSizeSlider.GetPos() / 10.);
		win->SetWindowText(CA2T(str.c_str()));
	}
}

void CFmiSynopPlotSettingsDlg::UpdatePlotSpacingString(void)
{
	CWnd *win = GetDlgItem(IDC_STATIC_PLOT_SPACING_STR);
	if(win)
	{
		std::string str = NFmiStringTools::Convert(itsPlotSpacingSlider.GetPos() / 10.);
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

void CFmiSynopPlotSettingsDlg::OnBnClickedCheckShowV()
{
	// TODO: Add your control notification handler code here
}

void CFmiSynopPlotSettingsDlg::OnBnClickedCheckShowT()
{
	// TODO: Add your control notification handler code here
}

void CFmiSynopPlotSettingsDlg::OnBnClickedCheckShowWw()
{
	// TODO: Add your control notification handler code here
}

void CFmiSynopPlotSettingsDlg::OnBnClickedCheckShowTd()
{
	// TODO: Add your control notification handler code here
}

void CFmiSynopPlotSettingsDlg::OnBnClickedCheckShowCh()
{
	// TODO: Add your control notification handler code here
}

void CFmiSynopPlotSettingsDlg::OnBnClickedCheckShowCm()
{
	// TODO: Add your control notification handler code here
}

void CFmiSynopPlotSettingsDlg::OnBnClickedCheckShowN()
{
	// TODO: Add your control notification handler code here
}

void CFmiSynopPlotSettingsDlg::OnBnClickedCheckShowDdff()
{
	// TODO: Add your control notification handler code here
}

void CFmiSynopPlotSettingsDlg::OnBnClickedCheckShowCl()
{
	// TODO: Add your control notification handler code here
}

void CFmiSynopPlotSettingsDlg::OnBnClickedCheckShowH()
{
	// TODO: Add your control notification handler code here
}

void CFmiSynopPlotSettingsDlg::OnBnClickedCheckShowRr()
{
	// TODO: Add your control notification handler code here
}

void CFmiSynopPlotSettingsDlg::OnBnClickedCheckShowPppp()
{
	// TODO: Add your control notification handler code here
}

void CFmiSynopPlotSettingsDlg::OnBnClickedCheckShowPpp()
{
	// TODO: Add your control notification handler code here
}

void CFmiSynopPlotSettingsDlg::OnBnClickedCheckShowNh()
{
	// TODO: Add your control notification handler code here
}

void CFmiSynopPlotSettingsDlg::OnBnClickedCheckShowTw()
{
	// TODO: Add your control notification handler code here
}

void CFmiSynopPlotSettingsDlg::OnBnClickedCheckShowA()
{
	// TODO: Add your control notification handler code here
}

void CFmiSynopPlotSettingsDlg::OnBnClickedCheckShowW1()
{
	// TODO: Add your control notification handler code here
}

void CFmiSynopPlotSettingsDlg::OnBnClickedCheckShowW2()
{
	// TODO: Add your control notification handler code here
}

void CFmiSynopPlotSettingsDlg::OnBnClickedCheckShowSingleColor()
{
	// TODO: Add your control notification handler code here
}

void CFmiSynopPlotSettingsDlg::OnBnClickedButtonSingleColor()
{
    CtrlView::ColorButtonPressed(this, itsSingleColorRef, &itsSingleColorBitmap, itsSingleColorRect, itsSingleColorChangeButtom);
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
	CFmiWin32Helpers::SetDialogItemText(this, IDC_CHECK_SHOW_SINGLE_COLOR, "IDC_CHECK_SHOW_SINGLE_COLOR");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_SYNOP_PLOT_FONT_SIZE_STR, "IDC_STATIC_SYNOP_PLOT_FONT_SIZE_STR");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_SYNOP_PLOT_SPACING, "IDC_STATIC_SYNOP_PLOT_SPACING");
}

void CFmiSynopPlotSettingsDlg::OnBnClickedCheckToggleAllParams()
{
	UpdateData(TRUE);

	SetAllParamStates(fSetAllParamState);
}

void CFmiSynopPlotSettingsDlg::SetAllParamStates(BOOL newState)
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
