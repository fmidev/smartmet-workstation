// FmiTempSettingsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "FmiTempSettingsDlg.h"
#include "NFmiMTATempSystem.h"
#include "NFmiStringTools.h"
#include "FmiTempLineSettings.h"
#include "NFmiDictionaryFunction.h"
#include "FmiWin32Helpers.h"
#include "FmiTempDlg.h"
#include "CtrlViewWin32Functions.h"
#include "boost\math\special_functions\round.hpp"
#include "CtrlViewGdiPlusFunctions.h"


// CFmiTempSettingsDlg dialog

IMPLEMENT_DYNAMIC(CFmiTempSettingsDlg, CDialog)
CFmiTempSettingsDlg::CFmiTempSettingsDlg(NFmiMTATempSystem *theTempSystem, CFmiTempDlg* theTempDialog)
: CDialog(CFmiTempSettingsDlg::IDD, theTempDialog)
	, itsTempSystem(theTempSystem)
	, itsTStart1(0)
	, itsTEnd1(0)
	, itsTStart2(0)
	, itsTEnd2(0)
	, itsTStart3(0)
	, itsTEnd3(0)
	, itsPStart(0)
	, itsPEnd(0)
	, itsWindVectorSizeX(0)
	, itsWindVectorSizeY(0)
	, itsLegendTextSize(0)
	, itsHelpLineTStartValue(0)
	, itsHelpLineTEndValue(0)
	, itsHelpLineTStepValue(0)
    , itsHelpLineValuesPU_(_T(""))
    , itsHelpLineValuesDryU_(_T(""))
    , itsHelpLineValuesMoistU_(_T(""))
    , itsHelpLineValuesMixU_(_T(""))
	, fDrawWindVector(FALSE)
	, fDrawLegend(FALSE)
	, fDrawHeightValuesOnlyForFirstSounding(FALSE)
	, itsColorBitmap1(0)
	, itsColorBitmap2(0)
	, itsColorBitmap3(0)
	, itsColorBitmap4(0)
	, itsColorBitmap5(0)
	, itsColorBitmap6(0)
	, itsColorBitmap7(0)
	, itsColorBitmap8(0)
	, itsColorBitmap9(0)
	, itsColorBitmap10(0)
	, itsMaxTempsOnView(0)
	, itsIndexiesFontSize(0)
	, fShowCondensationTrailProbabilityLines(FALSE)
	, itsSoundingTextFontSize(0)
	, fShowKilometerScale(FALSE)
	, fShowFlightLevelScale(FALSE)
	, fShowOnlyFirstSoundingInHodograf(FALSE)
	, itsResetScalesStartP(0)
	, itsResetScalesEndP(0)
	, itsResetScalesStartT(0)
	, itsResetScalesEndT(0)
	, itsResetScalesSkewTStartT(0)
	, itsResetScalesSkewTEndT(0)
    , fShowSecondaryDataView(FALSE)
    , itsSecondaryDataViewWidth(0)
    , itsTempDialog(theTempDialog)
    , itsOrigMTATempSystem(*theTempSystem)
    , fRefreshPressed(false)
{
}

CFmiTempSettingsDlg::~CFmiTempSettingsDlg()
{
	CtrlView::DestroyBitmap(&itsColorBitmap1);
    CtrlView::DestroyBitmap(&itsColorBitmap2);
    CtrlView::DestroyBitmap(&itsColorBitmap3);
    CtrlView::DestroyBitmap(&itsColorBitmap4);
    CtrlView::DestroyBitmap(&itsColorBitmap5);
    CtrlView::DestroyBitmap(&itsColorBitmap6);
    CtrlView::DestroyBitmap(&itsColorBitmap7);
    CtrlView::DestroyBitmap(&itsColorBitmap8);
    CtrlView::DestroyBitmap(&itsColorBitmap9);
    CtrlView::DestroyBitmap(&itsColorBitmap10);
}

void CFmiTempSettingsDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_SLIDER_T_DEGREE, itsTDegreeSlider);
    DDX_Text(pDX, IDC_EDIT_T_START1, itsTStart1);
    DDX_Text(pDX, IDC_EDIT_T_END1, itsTEnd1);
    DDX_Text(pDX, IDC_EDIT_T_START2, itsTStart2);
    DDX_Text(pDX, IDC_EDIT_T_END2, itsTEnd2);
    DDX_Text(pDX, IDC_EDIT_T_START3, itsTStart3);
    DDX_Text(pDX, IDC_EDIT_T_END3, itsTEnd3);
    DDX_Text(pDX, IDC_EDIT_P_START, itsPStart);
    DDX_Text(pDX, IDC_EDIT_P_END, itsPEnd);
    DDX_Text(pDX, IDC_EDIT_WIND_VECTOR_SIZE_X, itsWindVectorSizeX);
    DDX_Text(pDX, IDC_EDIT_WIND_VECTOR_SIZE_Y, itsWindVectorSizeY);
    DDX_Text(pDX, IDC_EDIT_LEGEND_TEXT_SIZE, itsLegendTextSize);
    DDX_Text(pDX, IDC_EDIT_T_HELP_START, itsHelpLineTStartValue);
    DDX_Text(pDX, IDC_EDIT_T_HELP_END, itsHelpLineTEndValue);
    DDX_Text(pDX, IDC_EDIT_T_HELP_STEP, itsHelpLineTStepValue);
    DDX_Text(pDX, IDC_EDIT_HELP_LINE_VALUES_P, itsHelpLineValuesPU_);
    DDX_Text(pDX, IDC_EDIT_HELP_LINE_VALUES_DRY, itsHelpLineValuesDryU_);
    DDX_Text(pDX, IDC_EDIT_HELP_LINE_VALUES_MOIST, itsHelpLineValuesMoistU_);
    DDX_Text(pDX, IDC_EDIT_HELP_LINE_VALUES_MIX, itsHelpLineValuesMixU_);
    DDX_Check(pDX, IDC_CHECK_DRAW_WIND_VECTOR, fDrawWindVector);
    DDX_Check(pDX, IDC_CHECK_DRAW_LEGEND, fDrawLegend);
    DDX_Check(pDX, IDC_CHECK_DRAW_Z_VALUES_ONLY_FOR_FIRST_SOUNDING, fDrawHeightValuesOnlyForFirstSounding);
    DDX_Control(pDX, IDC_BUTTON_COLOR_TEMP1, itsColorButtomTemp1);
    DDX_Control(pDX, IDC_BUTTON_COLOR_TEMP2, itsColorButtomTemp2);
    DDX_Control(pDX, IDC_BUTTON_COLOR_TEMP3, itsColorButtomTemp3);
    DDX_Control(pDX, IDC_BUTTON_COLOR_TEMP4, itsColorButtomTemp4);
    DDX_Control(pDX, IDC_BUTTON_COLOR_TEMP5, itsColorButtomTemp5);
    DDX_Control(pDX, IDC_BUTTON_COLOR_TEMP6, itsColorButtomTemp6);
    DDX_Control(pDX, IDC_BUTTON_COLOR_TEMP7, itsColorButtomTemp7);
    DDX_Control(pDX, IDC_BUTTON_COLOR_TEMP8, itsColorButtomTemp8);
    DDX_Control(pDX, IDC_BUTTON_COLOR_TEMP9, itsColorButtomTemp9);
    DDX_Control(pDX, IDC_BUTTON_COLOR_TEMP10, itsColorButtomTemp10);
    DDX_Text(pDX, IDC_EDIT_MAX_TEMPS_ON_VIEW, itsMaxTempsOnView);
    DDX_Text(pDX, IDC_EDIT_INDEX_TEXT_FONT_SIZE, itsIndexiesFontSize);
    DDX_Text(pDX, IDC_EDIT_SOUNDING_TEXT_FONT_SIZE, itsSoundingTextFontSize);
    DDX_Check(pDX, IDC_CHECK_DRAW_CONDENSATION_TRAIL_PROB_LINES, fShowCondensationTrailProbabilityLines);
    DDX_Check(pDX, IDC_CHECK_DRAW_KILOMETER_SCALE, fShowKilometerScale);
    DDX_Check(pDX, IDC_CHECK_DRAW_FLIGHTLEVEL_SCALE, fShowFlightLevelScale);
    DDX_Check(pDX, IDC_CHECK_SHOW_ONLY_FIRST_SOUNDING_IN_HODOGRAF, fShowOnlyFirstSoundingInHodograf);
    DDX_Text(pDX, IDC_EDIT_RESET_SCALES_END_P, itsResetScalesEndP);
    DDX_Text(pDX, IDC_EDIT_RESET_SCALES_START_P, itsResetScalesStartP);
    DDX_Text(pDX, IDC_EDIT_RESET_SCALES_EMAGRAMMI_START_T, itsResetScalesStartT);
    DDX_Text(pDX, IDC_EDIT_RESET_SCALES_EMAGRAMMI_END_T, itsResetScalesEndT);
    DDX_Text(pDX, IDC_EDIT_RESET_SCALES_SKEWT_START_T, itsResetScalesSkewTStartT);
    DDX_Text(pDX, IDC_EDIT_RESET_SCALES_SKEWT_END_T, itsResetScalesSkewTEndT);
    DDX_Check(pDX, IDC_CHECK_SHOW_SECONDARY_DATA, fShowSecondaryDataView);
    DDX_Text(pDX, IDC_EDIT_SOUNDING_SECONDARY_VIEW_WIDTH, itsSecondaryDataViewWidth);

    DDV_MinMaxInt(pDX, itsMaxTempsOnView, 1, 10);
    DDV_MinMaxInt(pDX, itsSecondaryDataViewWidth, 10, 40);
}


BEGIN_MESSAGE_MAP(CFmiTempSettingsDlg, CDialog)
	ON_BN_CLICKED(IDC_BUTTON_SET_HELP_LINE_SETTINGS_T, OnBnClickedButtonSetHelpLineSettingsT)
	ON_BN_CLICKED(IDC_BUTTON_SET_HELP_LINE_SETTINGS_P, OnBnClickedButtonSetHelpLineSettingsP)
	ON_BN_CLICKED(IDC_BUTTON_SET_HELP_LINE_SETTINGS_DRY, OnBnClickedButtonSetHelpLineSettingsDry)
	ON_BN_CLICKED(IDC_BUTTON_SET_HELP_LINE_SETTINGS_MOIST, OnBnClickedButtonSetHelpLineSettingsMoist)
	ON_BN_CLICKED(IDC_BUTTON_SET_HELP_LINE_SETTINGS_MIX, OnBnClickedButtonSetHelpLineSettingsMix)
	ON_WM_HSCROLL()
	ON_BN_CLICKED(IDC_BUTTON_SET_LINE_SETTINGS_T, OnBnClickedButtonSetLineSettingsT)
	ON_BN_CLICKED(IDC_BUTTON_SET_LINE_SETTINGS_TD, OnBnClickedButtonSetLineSettingsTd)
	ON_BN_CLICKED(IDC_BUTTON_SET_LINE_SETTINGS_Z, OnBnClickedButtonSetLineSettingsZ)
	ON_BN_CLICKED(IDC_BUTTON_COLOR_TEMP1, OnBnClickedButtonColorTemp1)
	ON_BN_CLICKED(IDC_BUTTON_COLOR_TEMP2, OnBnClickedButtonColorTemp2)
	ON_BN_CLICKED(IDC_BUTTON_COLOR_TEMP3, OnBnClickedButtonColorTemp3)
	ON_BN_CLICKED(IDC_BUTTON_COLOR_TEMP4, OnBnClickedButtonColorTemp4)
	ON_BN_CLICKED(IDC_BUTTON_COLOR_TEMP5, OnBnClickedButtonColorTemp5)
	ON_BN_CLICKED(IDC_BUTTON_COLOR_TEMP6, OnBnClickedButtonColorTemp6)
	ON_BN_CLICKED(IDC_BUTTON_COLOR_TEMP7, OnBnClickedButtonColorTemp7)
	ON_BN_CLICKED(IDC_BUTTON_COLOR_TEMP8, OnBnClickedButtonColorTemp8)
	ON_BN_CLICKED(IDC_BUTTON_COLOR_TEMP9, OnBnClickedButtonColorTemp9)
	ON_BN_CLICKED(IDC_BUTTON_COLOR_TEMP10, OnBnClickedButtonColorTemp10)
	ON_WM_PAINT()
	ON_BN_CLICKED(IDC_BUTTON_SET_HELP_LINE_SETTINGS_AIR_PARCEL1, OnBnClickedButtonSetHelpLineSettingsAirParcel1)
	ON_BN_CLICKED(IDC_BUTTON_SET_HELP_LINE_SETTINGS_AIR_PARCEL2, OnBnClickedButtonSetHelpLineSettingsAirParcel2)
	ON_BN_CLICKED(IDC_BUTTON_SET_HELP_LINE_SETTINGS_AIR_PARCEL3, OnBnClickedButtonSetHelpLineSettingsAirParcel3)
	ON_BN_CLICKED(IDC_BUTTON_SET_HELP_LINE_SETTINGS_WIND_MODIFICATION_AREA, OnBnClickedButtonSetHelpLineSettingsWindModificationArea)
    ON_BN_CLICKED(IDC_BUTTON_SET_HELP_LINE_SETTINGS_WS, &CFmiTempSettingsDlg::OnBnClickedButtonSetHelpLineSettingsWs)
    ON_BN_CLICKED(IDC_BUTTON_SET_HELP_LINE_SETTINGS_N, &CFmiTempSettingsDlg::OnBnClickedButtonSetHelpLineSettingsN)
    ON_BN_CLICKED(IDC_BUTTON_SET_HELP_LINE_SETTINGS_RH, &CFmiTempSettingsDlg::OnBnClickedButtonSetHelpLineSettingsRh)
    ON_BN_CLICKED(IDC_BUTTON_REFRASH_SOUNDING_VIEW, &CFmiTempSettingsDlg::OnBnClickedButtonRefrashSoundingView)
    ON_WM_CLOSE()
END_MESSAGE_MAP()

static std::string MakeValueStr(const checkedVector<double> &theValues)
{
	std::string str;
	size_t ssize = theValues.size();
	for(size_t i = 0; i < ssize; i++)
	{
		str += NFmiStringTools::Convert<double>(theValues[i]);
		if(i<ssize-1) // viimeisen perään ei laiteta pilkkua
			str += ",";
	}
	return str;
}

// CFmiTempSettingsDlg message handlers

BOOL CFmiTempSettingsDlg::Create(void) // modaalittomaa varten
{
    return CDialog::Create(CFmiTempSettingsDlg::IDD);
}

BOOL CFmiTempSettingsDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	InitDialogTexts();

	itsTDegreeSlider.SetRange(-150, 150);
	itsTDegreeSlider.SetPos(static_cast<int>(itsTempSystem->SkewTDegree()));

	double oldDegreeValue = itsTempSystem->SkewTDegree();
	itsTempSystem->SkewTDegree(-1); // kikka, pitää asettaa negatiivinen luku, jotta saadaan sitä vastaavat alku ja loppu arvot ulos
	itsTStart1 = itsTempSystem->TAxisStart();
	itsTEnd1 = itsTempSystem->TAxisEnd();
	itsTempSystem->SkewTDegree(0); // kikka, pitää asettaa negatiivinen luku, jotta saadaan sitä vastaavat alku ja loppu arvot ulos
	itsTStart2 = itsTempSystem->TAxisStart();
	itsTEnd2 = itsTempSystem->TAxisEnd();
	itsTempSystem->SkewTDegree(1); // kikka, pitää asettaa negatiivinen luku, jotta saadaan sitä vastaavat alku ja loppu arvot ulos
	itsTStart3 = itsTempSystem->TAxisStart();
	itsTEnd3 = itsTempSystem->TAxisEnd();
	itsTempSystem->SkewTDegree(oldDegreeValue); // lopuksi vanha arvo takaisin
	itsPStart = itsTempSystem->PAxisStart();
	itsPEnd = itsTempSystem->PAxisEnd();
	itsWindVectorSizeX = static_cast<int>(itsTempSystem->WindvectorSizeInPixels().X());
	itsWindVectorSizeY = static_cast<int>(itsTempSystem->WindvectorSizeInPixels().Y());
	itsLegendTextSize = static_cast<int>(itsTempSystem->LegendTextSize().X());
	itsHelpLineTStartValue = itsTempSystem->TemperatureHelpLineStart();
	itsHelpLineTEndValue = itsTempSystem->TemperatureHelpLineEnd();
	itsHelpLineTStepValue = itsTempSystem->TemperatureHelpLineStep();
	itsHelpLineValuesPU_ = CA2T(MakeValueStr(itsTempSystem->PressureValues()).c_str());
    itsHelpLineValuesDryU_ = CA2T(MakeValueStr(itsTempSystem->DryAdiabaticValues()).c_str());
    itsHelpLineValuesMoistU_ = CA2T(MakeValueStr(itsTempSystem->MoistAdiabaticValues()).c_str());
    itsHelpLineValuesMixU_ = CA2T(MakeValueStr(itsTempSystem->MixingRatioValues()).c_str());
	fDrawWindVector = itsTempSystem->DrawWinds();
	fDrawLegend = itsTempSystem->DrawLegendText();
	fDrawHeightValuesOnlyForFirstSounding = itsTempSystem->DrawOnlyHeightValuesOfFirstDrawedSounding();
	itsMaxTempsOnView = itsTempSystem->MaxTempsShowed();
	itsIndexiesFontSize = itsTempSystem->IndexiesFontSize();
	itsSoundingTextFontSize = itsTempSystem->SoundingTextFontSize();
	fShowCondensationTrailProbabilityLines = itsTempSystem->ShowCondensationTrailProbabilityLines();

	fShowKilometerScale = itsTempSystem->ShowKilometerScale();
	fShowFlightLevelScale = itsTempSystem->ShowFlightLevelScale();
	fShowOnlyFirstSoundingInHodograf = itsTempSystem->ShowOnlyFirstSoundingInHodograf();

	itsResetScalesStartP = itsTempSystem->ResetScalesStartP();
	itsResetScalesEndP = itsTempSystem->ResetScalesEndP();
	itsResetScalesStartT = itsTempSystem->ResetScalesStartT();
	itsResetScalesEndT = itsTempSystem->ResetScalesEndT();
	itsResetScalesSkewTStartT = itsTempSystem->ResetScalesSkewTStartT();
	itsResetScalesSkewTEndT = itsTempSystem->ResetScalesSkewTEndT();

	// huom indeksi SoundingColor-metodiin yhden pienempi kuin muuttuja nimissä!!
	CtrlView::InitialButtonColorUpdate(this, itsTempSystem->SoundingColor(0), itsColorRef1, &itsColorBitmap1, itsColorRect1, itsColorButtomTemp1);
    CtrlView::InitialButtonColorUpdate(this, itsTempSystem->SoundingColor(1), itsColorRef2, &itsColorBitmap2, itsColorRect2, itsColorButtomTemp2);
    CtrlView::InitialButtonColorUpdate(this, itsTempSystem->SoundingColor(2), itsColorRef3, &itsColorBitmap3, itsColorRect3, itsColorButtomTemp3);
    CtrlView::InitialButtonColorUpdate(this, itsTempSystem->SoundingColor(3), itsColorRef4, &itsColorBitmap4, itsColorRect4, itsColorButtomTemp4);
    CtrlView::InitialButtonColorUpdate(this, itsTempSystem->SoundingColor(4), itsColorRef5, &itsColorBitmap5, itsColorRect5, itsColorButtomTemp5);
    CtrlView::InitialButtonColorUpdate(this, itsTempSystem->SoundingColor(5), itsColorRef6, &itsColorBitmap6, itsColorRect6, itsColorButtomTemp6);
    CtrlView::InitialButtonColorUpdate(this, itsTempSystem->SoundingColor(6), itsColorRef7, &itsColorBitmap7, itsColorRect7, itsColorButtomTemp7);
    CtrlView::InitialButtonColorUpdate(this, itsTempSystem->SoundingColor(7), itsColorRef8, &itsColorBitmap8, itsColorRect8, itsColorButtomTemp8);
    CtrlView::InitialButtonColorUpdate(this, itsTempSystem->SoundingColor(8), itsColorRef9, &itsColorBitmap9, itsColorRect9, itsColorButtomTemp9);
    CtrlView::InitialButtonColorUpdate(this, itsTempSystem->SoundingColor(9), itsColorRef10, &itsColorBitmap10, itsColorRect10, itsColorButtomTemp10);

    fShowSecondaryDataView = itsTempSystem->DrawSecondaryData();
    itsSecondaryDataViewWidth = boost::math::iround(itsTempSystem->SecondaryDataFrameWidthFactor() * 100.);

	UpdateData(FALSE);
	UpdateDegreeStr();
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CFmiTempSettingsDlg::UpdateDegreeStr(void)
{
	CWnd *win = GetDlgItem(IDC_STATIC_T_DEGREE_STR);
	if(win)
	{
		UpdateData(TRUE);
        CString strU_ = CA2T(NFmiStringTools::Convert<int>(itsTDegreeSlider.GetPos()).c_str());
        strU_ += _TEXT("º");
        win->SetWindowText(strU_);
		UpdateData(FALSE);
	}
}

static NFmiColor ColorRef2Color(COLORREF color)
{
	NFmiColor tColor;
	tColor.SetRGB((float(GetRValue(color))/float(255.0)),
				  (float(GetGValue(color))/float(255.0)),
				  (float(GetBValue(color))/float(255.0)));
	return tColor;
}

void CFmiTempSettingsDlg::OnOK()
{
    GetSettingsFromDialog();
    CDialog::OnOK();
}

void CFmiTempSettingsDlg::GetSettingsFromDialog()
{
	UpdateData(TRUE);
	// Kopioidaan mahd. muutokset takaisin alkuperäisiin
	itsTempSystem->SkewTDegree(itsTDegreeSlider.GetPos());
	double oldDegreeValue = itsTempSystem->SkewTDegree();
	itsTempSystem->SkewTDegree(-1); // kikka, pitää asettaa negatiivinen luku, jotta saadaan sitä vastaavat alku ja loppu arvot ulos
	itsTempSystem->TAxisStart(itsTStart1);
	itsTempSystem->TAxisEnd(itsTEnd1);
	itsTempSystem->SkewTDegree(0); // kikka, pitää asettaa negatiivinen luku, jotta saadaan sitä vastaavat alku ja loppu arvot ulos
	itsTempSystem->TAxisStart(itsTStart2);
	itsTempSystem->TAxisEnd(itsTEnd2);
	itsTempSystem->SkewTDegree(1); // kikka, pitää asettaa negatiivinen luku, jotta saadaan sitä vastaavat alku ja loppu arvot ulos
	itsTempSystem->TAxisStart(itsTStart3);
	itsTempSystem->TAxisEnd(itsTEnd3);
	itsTempSystem->SkewTDegree(oldDegreeValue); // lopuksi vanha arvo takaisin
	itsTempSystem->PAxisStart(itsPStart);
	itsTempSystem->PAxisEnd(itsPEnd);
	NFmiPoint p1(itsWindVectorSizeX, itsWindVectorSizeY);
	itsTempSystem->WindvectorSizeInPixels(p1);
	NFmiPoint p2(itsLegendTextSize, itsLegendTextSize);
	itsTempSystem->LegendTextSize(p2);
	itsTempSystem->TemperatureHelpLineStart(itsHelpLineTStartValue);
	itsTempSystem->TemperatureHelpLineEnd(itsHelpLineTEndValue);
	itsTempSystem->TemperatureHelpLineStep(itsHelpLineTStepValue);
	try
	{
        std::string tmp = CT2A(itsHelpLineValuesPU_);
		itsTempSystem->PressureValues() = NFmiStringTools::Split<checkedVector<double> >(tmp);
        tmp = CT2A(itsHelpLineValuesDryU_);
		itsTempSystem->DryAdiabaticValues() = NFmiStringTools::Split<checkedVector<double> >(tmp);
        tmp = CT2A(itsHelpLineValuesMoistU_);
		itsTempSystem->MoistAdiabaticValues() = NFmiStringTools::Split<checkedVector<double> >(tmp);
        tmp = CT2A(itsHelpLineValuesMixU_);
		itsTempSystem->MixingRatioValues() = NFmiStringTools::Split<checkedVector<double> >(tmp);
	}
	catch(std::exception & /* e */)
	{
	}
	itsTempSystem->DrawWinds(fDrawWindVector == TRUE);
	itsTempSystem->DrawLegendText(fDrawLegend == TRUE);
	itsTempSystem->DrawOnlyHeightValuesOfFirstDrawedSounding(fDrawHeightValuesOnlyForFirstSounding == TRUE);
	itsTempSystem->MaxTempsShowed(itsMaxTempsOnView);
	itsTempSystem->IndexiesFontSize(itsIndexiesFontSize);
	itsTempSystem->SoundingTextFontSize(itsSoundingTextFontSize);
	itsTempSystem->ShowCondensationTrailProbabilityLines(fShowCondensationTrailProbabilityLines == TRUE);

	itsTempSystem->ShowKilometerScale(fShowKilometerScale == TRUE);
	itsTempSystem->ShowFlightLevelScale(fShowFlightLevelScale == TRUE);
	itsTempSystem->ShowOnlyFirstSoundingInHodograf(fShowOnlyFirstSoundingInHodograf == TRUE);

	itsTempSystem->ResetScalesStartP(itsResetScalesStartP);
	itsTempSystem->ResetScalesEndP(itsResetScalesEndP);
	itsTempSystem->ResetScalesStartT(itsResetScalesStartT);
	itsTempSystem->ResetScalesEndT(itsResetScalesEndT);
	itsTempSystem->ResetScalesSkewTStartT(itsResetScalesSkewTStartT);
	itsTempSystem->ResetScalesSkewTEndT(itsResetScalesSkewTEndT);

	// huom indeksi SoundingColor-metodiin yhden pienempi kuin muuttuja nimissä!!
	itsTempSystem->SoundingColor(0, ColorRef2Color(itsColorRef1));
	itsTempSystem->SoundingColor(1, ColorRef2Color(itsColorRef2));
	itsTempSystem->SoundingColor(2, ColorRef2Color(itsColorRef3));
	itsTempSystem->SoundingColor(3, ColorRef2Color(itsColorRef4));
	itsTempSystem->SoundingColor(4, ColorRef2Color(itsColorRef5));
	itsTempSystem->SoundingColor(5, ColorRef2Color(itsColorRef6));
	itsTempSystem->SoundingColor(6, ColorRef2Color(itsColorRef7));
	itsTempSystem->SoundingColor(7, ColorRef2Color(itsColorRef8));
	itsTempSystem->SoundingColor(8, ColorRef2Color(itsColorRef9));
	itsTempSystem->SoundingColor(9, ColorRef2Color(itsColorRef10));

    itsTempSystem->DrawSecondaryData(fShowSecondaryDataView == TRUE);
    itsTempSystem->SecondaryDataFrameWidthFactor(itsSecondaryDataViewWidth / 100.);
}

void CFmiTempSettingsDlg::DoOnCancel()
{
    if(fRefreshPressed)
        *itsTempSystem = itsOrigMTATempSystem;
}

void CFmiTempSettingsDlg::OnCancel()
{
    DoOnCancel();

	CDialog::OnCancel();
}

CRect CFmiTempSettingsDlg::GetDldResourceRectInClientSpace(int helpImageResourceId)
{
	CRect winRect;
	CWnd *win = GetDlgItem(helpImageResourceId);
	if(win)
	{
		win->GetWindowRect(winRect);
		this->ScreenToClient(winRect);
	}
	return winRect;
}

void CFmiTempSettingsDlg::DoLineSettingDialog(int helpImageResourceId, const CString &theTitle, bool isHelpLine, bool isHelpText,
    NFmiTempLineInfo *theLineInfo,
    NFmiTempLabelInfo *theLabelInfo)
{
	CFmiTempLineSettings dlg(theTitle, isHelpLine, isHelpText, theLineInfo, theLabelInfo, this);
	if(dlg.DoModal() == IDOK)
	{
		if(helpImageResourceId != 0)
		{
			InvalidateRect(GetDldResourceRectInClientSpace(helpImageResourceId));
		}
	}
}

void CFmiTempSettingsDlg::OnBnClickedButtonSetHelpLineSettingsT()
{
    DoLineSettingDialog(IDC_STATIC_SAMPLE_HELP_T, CString(CA2T(::GetDictionaryString("TempViewSettingDlgHelpLineThelpTitleStr").c_str())), true, false, &(itsTempSystem->TemperatureHelpLineInfo()), &(itsTempSystem->TemperatureHelpLabelInfo()));
}

void CFmiTempSettingsDlg::OnBnClickedButtonSetHelpLineSettingsP()
{
    DoLineSettingDialog(IDC_STATIC_SAMPLE_HELP_P, CString(CA2T(::GetDictionaryString("TempViewSettingDlgHelpLinePTitleStr").c_str())), true, false, &(itsTempSystem->PressureLineInfo()), &(itsTempSystem->PressureLabelInfo()));
}

void CFmiTempSettingsDlg::OnBnClickedButtonSetHelpLineSettingsDry()
{
    DoLineSettingDialog(IDC_STATIC_SAMPLE_HELP_DRY, CString(CA2T(::GetDictionaryString("TempViewSettingDlgHelpLineDryTitleStr").c_str())), true, false, &(itsTempSystem->DryAdiabaticLineInfo()), &(itsTempSystem->DryAdiabaticLabelInfo()));
}

void CFmiTempSettingsDlg::OnBnClickedButtonSetHelpLineSettingsMoist()
{
    DoLineSettingDialog(IDC_STATIC_SAMPLE_HELP_MOIST, CString(CA2T(::GetDictionaryString("TempViewSettingDlgHelpLineMoistTitleStr").c_str())), true, false, &(itsTempSystem->MoistAdiabaticLineInfo()), &(itsTempSystem->MoistAdiabaticLabelInfo()));
}

void CFmiTempSettingsDlg::OnBnClickedButtonSetHelpLineSettingsMix()
{
    DoLineSettingDialog(IDC_STATIC_SAMPLE_HELP_MIX, CString(CA2T(::GetDictionaryString("TempViewSettingDlgHelpLineMixTitleStr").c_str())), true, false, &(itsTempSystem->MixingRatioLineInfo()), &(itsTempSystem->MixingRatioLabelInfo()));
}

void CFmiTempSettingsDlg::OnBnClickedButtonSetHelpLineSettingsAirParcel1()
{
    DoLineSettingDialog(IDC_STATIC_SAMPLE_HELP_AIR_PARCEL1, CString(CA2T(::GetDictionaryString("TempViewSettingDlgHelpLineAirParcel1TitleStr").c_str())), true, false, &(itsTempSystem->AirParcel1LineInfo()), &(itsTempSystem->AirParcel1LabelInfo()));
}

void CFmiTempSettingsDlg::OnBnClickedButtonSetHelpLineSettingsAirParcel2()
{
    DoLineSettingDialog(IDC_STATIC_SAMPLE_HELP_AIR_PARCEL2, CString(CA2T(::GetDictionaryString("TempViewSettingDlgHelpLineAirParcel2TitleStr").c_str())), true, false, &(itsTempSystem->AirParcel2LineInfo()), &(itsTempSystem->AirParcel2LabelInfo()));
}

void CFmiTempSettingsDlg::OnBnClickedButtonSetHelpLineSettingsAirParcel3()
{
    DoLineSettingDialog(IDC_STATIC_SAMPLE_HELP_AIR_PARCEL3, CString(CA2T(::GetDictionaryString("TempViewSettingDlgHelpLineAirParcel3TitleStr").c_str())), true, false, &(itsTempSystem->AirParcel3LineInfo()), &(itsTempSystem->AirParcel3LabelInfo()));
}

void CFmiTempSettingsDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	UpdateDegreeStr();

	CDialog::OnHScroll(nSBCode, nPos, pScrollBar);
}

void CFmiTempSettingsDlg::OnBnClickedButtonSetLineSettingsT()
{
	// label info on feikki, koska sitä ei voi kuitenkaan muokata
    DoLineSettingDialog(IDC_STATIC_SAMPLE_T, CString(CA2T(::GetDictionaryString("TempViewSettingDlgHelpLineTTitleStr").c_str())), false, false, &(itsTempSystem->TemperatureLineInfo()), &(itsTempSystem->PressureLabelInfo()));
}

void CFmiTempSettingsDlg::OnBnClickedButtonSetLineSettingsTd()
{
	// labelInfo on feikki, koska sitä ei voi kuitenkaan muokata
    DoLineSettingDialog(IDC_STATIC_SAMPLE_TD, CString(CA2T(::GetDictionaryString("TempViewSettingDlgHelpLineTdTitleStr").c_str())), false, false, &(itsTempSystem->DewPointLineInfo()), &(itsTempSystem->PressureLabelInfo()));
}

void CFmiTempSettingsDlg::OnBnClickedButtonSetLineSettingsZ()
{
	// lineInfo on feikki, koska sitä ei voi kuitenkaan muokata
	// samoin id resurssi 0 on feikki
    DoLineSettingDialog(0, CString(CA2T(::GetDictionaryString("TempViewSettingDlgHelpLineZTitleStr").c_str())), false, true, &(itsTempSystem->DewPointLineInfo()), &(itsTempSystem->HeightValueLabelInfo()));
}

void CFmiTempSettingsDlg::OnBnClickedButtonSetHelpLineSettingsWindModificationArea()
{
	// labelInfo on feikki, koska sitä ei voi kuitenkaan muokata
    DoLineSettingDialog(IDC_STATIC_SAMPLE_HELP_WIND_MODIFICATION_AREA, CString(CA2T(::GetDictionaryString("TempViewSettingDlgHelpLineWindModificationAreaTitleStr").c_str())), true, false, &(itsTempSystem->WindModificationAreaLineInfo()), &(itsTempSystem->PressureLabelInfo()));
}

void CFmiTempSettingsDlg::OnBnClickedButtonColorTemp1()
{
    CtrlView::ColorButtonPressed(this, itsColorRef1, &itsColorBitmap1, itsColorRect1, itsColorButtomTemp1);
}

void CFmiTempSettingsDlg::OnBnClickedButtonColorTemp2()
{
    CtrlView::ColorButtonPressed(this, itsColorRef2, &itsColorBitmap2, itsColorRect2, itsColorButtomTemp2);
}

void CFmiTempSettingsDlg::OnBnClickedButtonColorTemp3()
{
    CtrlView::ColorButtonPressed(this, itsColorRef3, &itsColorBitmap3, itsColorRect3, itsColorButtomTemp3);
}

void CFmiTempSettingsDlg::OnBnClickedButtonColorTemp4()
{
    CtrlView::ColorButtonPressed(this, itsColorRef4, &itsColorBitmap4, itsColorRect4, itsColorButtomTemp4);
}

void CFmiTempSettingsDlg::OnBnClickedButtonColorTemp5()
{
    CtrlView::ColorButtonPressed(this, itsColorRef5, &itsColorBitmap5, itsColorRect5, itsColorButtomTemp5);
}

void CFmiTempSettingsDlg::OnBnClickedButtonColorTemp6()
{
    CtrlView::ColorButtonPressed(this, itsColorRef6, &itsColorBitmap6, itsColorRect6, itsColorButtomTemp6);
}

void CFmiTempSettingsDlg::OnBnClickedButtonColorTemp7()
{
    CtrlView::ColorButtonPressed(this, itsColorRef7, &itsColorBitmap7, itsColorRect7, itsColorButtomTemp7);
}

void CFmiTempSettingsDlg::OnBnClickedButtonColorTemp8()
{
    CtrlView::ColorButtonPressed(this, itsColorRef8, &itsColorBitmap8, itsColorRect8, itsColorButtomTemp8);
}

void CFmiTempSettingsDlg::OnBnClickedButtonColorTemp9()
{
    CtrlView::ColorButtonPressed(this, itsColorRef9, &itsColorBitmap9, itsColorRect9, itsColorButtomTemp9);
}

void CFmiTempSettingsDlg::OnBnClickedButtonColorTemp10()
{
    CtrlView::ColorButtonPressed(this, itsColorRef10, &itsColorBitmap10, itsColorRect10, itsColorButtomTemp10);
}

void CFmiTempSettingsDlg::OnPaint()
{
	CPaintDC dc(this); // device context for painting

	DrawHelperLineSampleImages(&dc, IDC_STATIC_SAMPLE_T, &itsTempSystem->TemperatureLineInfo(), 0);
	DrawHelperLineSampleImages(&dc, IDC_STATIC_SAMPLE_TD, &itsTempSystem->DewPointLineInfo(), 0);
	DrawHelperLineSampleImages(&dc, IDC_STATIC_SAMPLE_HELP_T, &itsTempSystem->TemperatureHelpLineInfo(), &itsTempSystem->TemperatureHelpLabelInfo());
	DrawHelperLineSampleImages(&dc, IDC_STATIC_SAMPLE_HELP_P, &itsTempSystem->PressureLineInfo(), &itsTempSystem->PressureLabelInfo());
	DrawHelperLineSampleImages(&dc, IDC_STATIC_SAMPLE_HELP_DRY, &itsTempSystem->DryAdiabaticLineInfo(), &itsTempSystem->DryAdiabaticLabelInfo());
	DrawHelperLineSampleImages(&dc, IDC_STATIC_SAMPLE_HELP_MOIST, &itsTempSystem->MoistAdiabaticLineInfo(), &itsTempSystem->MoistAdiabaticLabelInfo());
	DrawHelperLineSampleImages(&dc, IDC_STATIC_SAMPLE_HELP_MIX, &itsTempSystem->MixingRatioLineInfo(), &itsTempSystem->MixingRatioLabelInfo());
	DrawHelperLineSampleImages(&dc, IDC_STATIC_SAMPLE_HELP_AIR_PARCEL1, &itsTempSystem->AirParcel1LineInfo(), &itsTempSystem->AirParcel1LabelInfo());
	DrawHelperLineSampleImages(&dc, IDC_STATIC_SAMPLE_HELP_AIR_PARCEL2, &itsTempSystem->AirParcel2LineInfo(), &itsTempSystem->AirParcel2LabelInfo());
	DrawHelperLineSampleImages(&dc, IDC_STATIC_SAMPLE_HELP_AIR_PARCEL3, &itsTempSystem->AirParcel3LineInfo(), &itsTempSystem->AirParcel3LabelInfo());
	DrawHelperLineSampleImages(&dc, IDC_STATIC_SAMPLE_HELP_WIND_MODIFICATION_AREA, &itsTempSystem->WindModificationAreaLineInfo(), 0);
    DrawHelperLineSampleImages(&dc, IDC_STATIC_SAMPLE_HELP_WS, &itsTempSystem->WSLineInfo(), 0);
    DrawHelperLineSampleImages(&dc, IDC_STATIC_SAMPLE_HELP_N, &itsTempSystem->NLineInfo(), 0);
    DrawHelperLineSampleImages(&dc, IDC_STATIC_SAMPLE_HELP_RH, &itsTempSystem->RHLineInfo(), 0);

	// Do not call CDialog::OnPaint() for painting messages
}

void CFmiTempSettingsDlg::DrawHelperLineSampleImages(CDC *dc, int imageHolderResourceId,
    NFmiTempLineInfo *theLineInfo,
    NFmiTempLabelInfo *theLabelInfo)
{
	CRect winRect(GetDldResourceRectInClientSpace(imageHolderResourceId));
	if(theLineInfo)
	{
		CPen myPen;

		// GDI ei osaa piirtää yhtä pikseliä paksumpia erikois viivoja (eli type muu kuin yhtenäinen
		// joten jouduin virittämään vähän piirtoa ja näissä demo kuvissa pitää kanssa piirtää feikaten millaiset
		// viivat oikeasti piirtyvät luotaus kuviin)
		bool drawSpecialLine = (theLineInfo->LineType() != 0) && (theLineInfo->Thickness() > 1);
		if(drawSpecialLine)
			myPen.CreatePen(theLineInfo->LineType(), 1, CtrlView::Color2ColorRef(theLineInfo->Color()));
		else
			myPen.CreatePen(theLineInfo->LineType(), theLineInfo->Thickness(), CtrlView::Color2ColorRef(theLineInfo->Color()));
		CPen *oldPen = dc->SelectObject(&myPen);
		dc->SetBkMode(TRANSPARENT);
		int h = winRect.top + (winRect.bottom - winRect.top)/3 - 2;
		if(drawSpecialLine)
		{
			dc->MoveTo(winRect.left + 2, h);
			dc->LineTo(winRect.right - 2, h);
			if(theLineInfo->Thickness() >= 2)
			{
				dc->MoveTo(winRect.left + 2, h+1);
				dc->LineTo(winRect.right - 2, h+1);
				if(theLineInfo->Thickness() >= 3)
				{
					dc->MoveTo(winRect.left + 2, h-1);
					dc->LineTo(winRect.right - 2, h-1);
				}
			}
		}
		else
		{
			dc->MoveTo(winRect.left + 2, h);
			dc->LineTo(winRect.right - 2, h);
		}

		dc->SelectObject(oldPen); // laitetaan vanha kynä takaisin
		myPen.DeleteObject(); // resurssit pitää vapauttaa vielä

		if(theLabelInfo)
		{
			dc->SetTextColor(CtrlView::Color2ColorRef(theLineInfo->Color()));
			dc->TextOut((winRect.left+winRect.right)/2 + 12, (winRect.bottom+winRect.top)/2 - 3, _TEXT("5"));
		}

		CPen myPen2;
		myPen2.CreatePen(0, 2, RGB(255, 0, 0));
		oldPen = dc->SelectObject(&myPen2);
		if(!theLineInfo->DrawLine())
		{ // vedetään ruksi koko jutun päälle, jos apu viivaa ei piirretä
			dc->MoveTo(winRect.left, winRect.bottom);
			dc->LineTo(winRect.right, winRect.top);
			dc->MoveTo(winRect.left, winRect.top);
			dc->LineTo(winRect.right, winRect.bottom);

		}
		else if(theLabelInfo && theLabelInfo->DrawLabelText() == false)
		{ // tällöin piirretään ruksi vain demo labelin päälle, että sitä ei piirretä
			dc->MoveTo(winRect.left + 35, winRect.bottom - 2);
			dc->LineTo(winRect.right - 5, winRect.top + 10);
			dc->MoveTo(winRect.left + 35, winRect.top + 10);
			dc->LineTo(winRect.right - 5, winRect.bottom - 2);
		}
		dc->SelectObject(oldPen); // laitetaan vanha kynä takaisin
		myPen2.DeleteObject();
	}
}

// Tämä funktio alustaa kaikki dialogin tekstit editoriin valitulla kielellä.
// Tämä on ikävä kyllä tehtävä erikseen dialogin muokkaus työkalusta, eli
// tekijän pitää lisätä erikseen tänne kaikki dialogin osat, joihin
// kieli valinta voi vaikuttaa.
void CFmiTempSettingsDlg::InitDialogTexts(void)
{
	SetWindowText(CA2T(::GetDictionaryString("TempViewSettingDlgTitle").c_str()));
	CFmiWin32Helpers::SetDialogItemText(this, IDOK, "IDOK");
	CFmiWin32Helpers::SetDialogItemText(this, IDCANCEL, "IDCANCEL");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_TEMP_SETTINGS_T_DEGREE_STR, "IDC_STATIC_TEMP_SETTINGS_T_DEGREE_STR");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_TEMP_SETTINGS_T_DEGREE_LESS_ZERO_STR, "IDC_STATIC_TEMP_SETTINGS_T_DEGREE_LESS_ZERO_STR");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_TEMP_SETTINGS_T_DEGREE_EQUAL_ZERO_STR, "IDC_STATIC_TEMP_SETTINGS_T_DEGREE_EQUAL_ZERO_STR");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_TEMP_SETTINGS_T_DEGREE_GREATER_ZERO_STR, "IDC_STATIC_TEMP_SETTINGS_T_DEGREE_GREATER_ZERO_STR");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_TEMP_SETTINGS_SOUNDING_COUNT_STR, "IDC_STATIC_TEMP_SETTINGS_SOUNDING_COUNT_STR");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_TEMP_SETTINGS_START1, "IDC_STATIC_TEMP_SETTINGS_START1");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_TEMP_SETTINGS_END1, "IDC_STATIC_TEMP_SETTINGS_END1");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_TEMP_SETTINGS_START2, "IDC_STATIC_TEMP_SETTINGS_START2");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_TEMP_SETTINGS_END2, "IDC_STATIC_TEMP_SETTINGS_END2");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_TEMP_SETTINGS_START3, "IDC_STATIC_TEMP_SETTINGS_START3");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_TEMP_SETTINGS_END3, "IDC_STATIC_TEMP_SETTINGS_END3");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_TEMP_SETTINGS_START4, "IDC_STATIC_TEMP_SETTINGS_START4");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_TEMP_SETTINGS_END4, "IDC_STATIC_TEMP_SETTINGS_END4");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_TEMP_SETTINGS_P_SCALE_STR, "IDC_STATIC_TEMP_SETTINGS_P_SCALE_STR");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_TEMP_SETTINGS_WIND_SIZE_STR, "IDC_STATIC_TEMP_SETTINGS_WIND_SIZE_STR");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_TEMP_SETTINGS_WIND_X_STR, "IDC_STATIC_TEMP_SETTINGS_WIND_X_STR");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_TEMP_SETTINGS_WIND_Y_STR, "IDC_STATIC_TEMP_SETTINGS_WIND_Y_STR");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_TEMP_SETTINGS_LEGEND_FONT_SIZE_STR, "IDC_STATIC_TEMP_SETTINGS_LEGEND_FONT_SIZE_STR");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_CHECK_DRAW_WIND_VECTOR, "IDC_CHECK_DRAW_WIND_VECTOR");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_CHECK_DRAW_LEGEND, "IDC_CHECK_DRAW_LEGEND");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_TEMP_SETTINGS_USED_COLORS_STR, "IDC_STATIC_TEMP_SETTINGS_USED_COLORS_STR");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_BUTTON_SET_LINE_SETTINGS_T, "IDC_BUTTON_SET_LINE_SETTINGS_T");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_BUTTON_SET_LINE_SETTINGS_T, "IDC_BUTTON_SET_LINE_SETTINGS_T");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_BUTTON_SET_HELP_LINE_SETTINGS_T, "IDC_BUTTON_SET_HELP_LINE_SETTINGS_T");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_BUTTON_SET_HELP_LINE_SETTINGS_P, "IDC_BUTTON_SET_HELP_LINE_SETTINGS_P");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_BUTTON_SET_HELP_LINE_SETTINGS_DRY, "IDC_BUTTON_SET_HELP_LINE_SETTINGS_DRY");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_BUTTON_SET_HELP_LINE_SETTINGS_MOIST, "IDC_BUTTON_SET_HELP_LINE_SETTINGS_MOIST");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_BUTTON_SET_HELP_LINE_SETTINGS_MIX, "IDC_BUTTON_SET_HELP_LINE_SETTINGS_MIX");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_BUTTON_SET_LINE_SETTINGS_Z, "IDC_BUTTON_SET_LINE_SETTINGS_Z");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_CHECK_DRAW_Z_VALUES_ONLY_FOR_FIRST_SOUNDING, "IDC_CHECK_DRAW_Z_VALUES_ONLY_FOR_FIRST_SOUNDING");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_TEMP_SETTINGS_SOUNDING_INDEX_FONT_SIZE_STR, "Stab. Index font size");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_TEMP_SETTINGS_SOUNDING_DATA_FONT_SIZE_STR, "Text data font size");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_TEMP_SETTINGS_INDEX_FONT_SIZE_STR2, "IDC_STATIC_TEMP_SETTINGS_INDEX_FONT_SIZE_STR2");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_CHECK_DRAW_CONDENSATION_TRAIL_PROB_LINES, "IDC_CHECK_DRAW_CONDENSATION_TRAIL_PROB_LINES");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_CHECK_DRAW_KILOMETER_SCALE, "IDC_CHECK_DRAW_KILOMETER_SCALE");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_CHECK_DRAW_FLIGHTLEVEL_SCALE, "IDC_CHECK_DRAW_FLIGHTLEVEL_SCALE");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_CHECK_SHOW_ONLY_FIRST_SOUNDING_IN_HODOGRAF, "IDC_CHECK_SHOW_ONLY_FIRST_SOUNDING_IN_HODOGRAF");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_TEMP_SETTINGS_SCALE_DEFAULTS_STR, "IDC_STATIC_TEMP_SETTINGS_SCALE_DEFAULTS_STR");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_TEMP_SETTINGS_SCALE_DEFAULTS_P_UP_STR, "IDC_STATIC_TEMP_SETTINGS_SCALE_DEFAULTS_P_UP_STR");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_TEMP_SETTINGS_SCALE_DEFAULTS_P_DOWN_STR, "IDC_STATIC_TEMP_SETTINGS_SCALE_DEFAULTS_P_DOWN_STR");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_TEMP_SETTINGS_SCALE_DEFAULTS_T_EMA_START_STR, "IDC_STATIC_TEMP_SETTINGS_SCALE_DEFAULTS_T_EMA_START_STR");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_TEMP_SETTINGS_SCALE_DEFAULTS_T_EMA_END_STR, "IDC_STATIC_TEMP_SETTINGS_SCALE_DEFAULTS_T_EMA_END_STR");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_TEMP_SETTINGS_SCALE_DEFAULTS_T_SKEWT_START_STR, "IDC_STATIC_TEMP_SETTINGS_SCALE_DEFAULTS_T_SKEWT_START_STR");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_TEMP_SETTINGS_SCALE_DEFAULTS_T_SKEWT_END_STR, "IDC_STATIC_TEMP_SETTINGS_SCALE_DEFAULTS_T_SKEWT_END_STR");

	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_TEMP_SETTINGS_START5, "IDC_STATIC_TEMP_SETTINGS_START5");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_TEMP_SETTINGS_END5, "IDC_STATIC_TEMP_SETTINGS_END5");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_TEMP_SETTINGS_STEP5, "IDC_STATIC_TEMP_SETTINGS_STEP5");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_TEMP_SETTINGS_HELP_LINE_VALUES_STR, "IDC_STATIC_TEMP_SETTINGS_HELP_LINE_VALUES_STR");

	CFmiWin32Helpers::SetDialogItemText(this, IDC_BUTTON_SET_HELP_LINE_SETTINGS_AIR_PARCEL1, "IDC_BUTTON_SET_HELP_LINE_SETTINGS_AIR_PARCEL1");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_BUTTON_SET_HELP_LINE_SETTINGS_AIR_PARCEL2, "IDC_BUTTON_SET_HELP_LINE_SETTINGS_AIR_PARCEL2");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_BUTTON_SET_HELP_LINE_SETTINGS_AIR_PARCEL3, "IDC_BUTTON_SET_HELP_LINE_SETTINGS_AIR_PARCEL3");

    CFmiWin32Helpers::SetDialogItemText(this, IDC_BUTTON_SET_HELP_LINE_SETTINGS_WS, "WS");
    CFmiWin32Helpers::SetDialogItemText(this, IDC_BUTTON_SET_HELP_LINE_SETTINGS_N, "N");
    CFmiWin32Helpers::SetDialogItemText(this, IDC_BUTTON_SET_HELP_LINE_SETTINGS_RH, "RH");
    CFmiWin32Helpers::SetDialogItemText(this, IDC_BUTTON_REFRASH_SOUNDING_VIEW, "Refresh");
    CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_SECONDARY_DATA_GROUP_TEXT, "Secondary data");
    CFmiWin32Helpers::SetDialogItemText(this, IDC_CHECK_SHOW_SECONDARY_DATA, "Show view");
    CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_TEMP_SETTINGS_SECONDARY_VIEW_WIDTH_TEXT, "View width [10-40 %]");
}




void CFmiTempSettingsDlg::OnBnClickedButtonSetHelpLineSettingsWs()
{
    // labelInfo on feikki, koska sitä ei voi kuitenkaan muokata
    NFmiTempLabelInfo *dummyLabelInfo = &itsTempSystem->PressureLabelInfo();
    DoLineSettingDialog(IDC_STATIC_SAMPLE_HELP_WS, CString(CA2T(::GetDictionaryString("WS line settings").c_str())), false, false, &(itsTempSystem->WSLineInfo()), dummyLabelInfo);
}


void CFmiTempSettingsDlg::OnBnClickedButtonSetHelpLineSettingsN()
{
    // labelInfo on feikki, koska sitä ei voi kuitenkaan muokata
    NFmiTempLabelInfo *dummyLabelInfo = &itsTempSystem->PressureLabelInfo();
    DoLineSettingDialog(IDC_STATIC_SAMPLE_HELP_N, CString(CA2T(::GetDictionaryString("N line settings").c_str())), false, false, &(itsTempSystem->NLineInfo()), dummyLabelInfo);
}


void CFmiTempSettingsDlg::OnBnClickedButtonSetHelpLineSettingsRh()
{
    // labelInfo on feikki, koska sitä ei voi kuitenkaan muokata
    NFmiTempLabelInfo *dummyLabelInfo = &itsTempSystem->PressureLabelInfo();
    DoLineSettingDialog(IDC_STATIC_SAMPLE_HELP_RH, CString(CA2T(::GetDictionaryString("RH line settings").c_str())), false, false, &(itsTempSystem->RHLineInfo()), dummyLabelInfo);
}


// HUOM! En saanut luotausnäytön ruudunpäivitystä toimimaan, joten disabloin Refresh -nappulan ja laitoin sen näkymättömäksi.
void CFmiTempSettingsDlg::OnBnClickedButtonRefrashSoundingView()
{
    fRefreshPressed = true;
    GetSettingsFromDialog();
    UpdateTempView();
}

void CFmiTempSettingsDlg::UpdateTempView(void)
{
    if(itsTempDialog)
    {
        itsTempDialog->Update();
        itsTempDialog->Invalidate(FALSE);
    }
}


void CFmiTempSettingsDlg::OnClose()
{
    DoOnCancel();

    CDialog::OnClose();
}
