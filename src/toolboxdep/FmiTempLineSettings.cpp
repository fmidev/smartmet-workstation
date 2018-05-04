// FmiTempLineSettings.cpp : implementation file
//

#include "stdafx.h"
#include "FmiTempLineSettings.h"
#include "NFmiStringTools.h"
#include "NFmiDictionaryFunction.h"
#include "FmiWin32Helpers.h"
#include "CtrlViewGdiPlusFunctions.h"
#include "CtrlViewWin32Functions.h"


// CFmiTempLineSettings dialog

IMPLEMENT_DYNAMIC(CFmiTempLineSettings, CDialog)
CFmiTempLineSettings::CFmiTempLineSettings(const CString &theTitleStr, bool isHelpLine, bool isHelpText,
    NFmiTempLineInfo *theLineInfo,
    NFmiTempLabelInfo *theLabelInfo, CWnd* pParent /*=NULL*/)
	: CDialog(CFmiTempLineSettings::IDD, pParent)
	, itsTitleStrU_(theTitleStr)
	, fIsHelpLine(isHelpLine)
	, fIsHelpText(isHelpText)
	, itsLineInfoOrig(theLineInfo)
	, itsLabelInfoOrig(theLabelInfo)
	, itsLineInfo(*theLineInfo)
	, itsLabelInfo(*theLabelInfo)
	, fDrawLine(FALSE)
	, itsLineType(0)
	, itsLabelOffsetX(0)
	, itsLabelOffsetY(0)
	, itsLabelFontSize(0)
	, fDrawLabel(FALSE)
	, fDrawLabelInDataBox(FALSE)
	, itsColorBitmap(0)
    , itsLabelFontAlignmentStrU_(_T(""))
{
}

CFmiTempLineSettings::~CFmiTempLineSettings()
{
	if(itsColorBitmap)
		itsColorBitmap->DeleteObject();
	delete itsColorBitmap;
}

void CFmiTempLineSettings::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_SLIDER_LINE_WIDTH, itsLineWidthSlider);
	DDX_Text(pDX, IDC_EDIT_LINE_TYPE, itsLineType);
	DDX_Text(pDX, IDC_EDIT_LABEL_OFFSET_X, itsLabelOffsetX);
	DDX_Text(pDX, IDC_EDIT_LABEL_OFFSET_Y, itsLabelOffsetY);
	DDX_Text(pDX, IDC_EDIT_LABEL_FONT_SIZE, itsLabelFontSize);
	DDX_Check(pDX, IDC_CHECK_DRAW_LINE, fDrawLine);
	DDX_Control(pDX, IDC_BUTTON_COLOR, itsColorButtom);
	DDX_Check(pDX, IDC_CHECK_DRAW_LABEL, fDrawLabel);
	DDX_Check(pDX, IDC_CHECK_CLIP_LABEL, fDrawLabelInDataBox);
    DDX_Text(pDX, IDC_EDIT_LABEL_FONT_ALIGMENT, itsLabelFontAlignmentStrU_);
    DDV_MaxChars(pDX, itsLabelFontAlignmentStrU_, 2);
}


BEGIN_MESSAGE_MAP(CFmiTempLineSettings, CDialog)
	ON_WM_HSCROLL()
	ON_BN_CLICKED(IDC_BUTTON_COLOR, OnBnClickedButtonColor)
END_MESSAGE_MAP()


// CFmiTempLineSettings message handlers

static CString GetAlignmentStr(FmiDirection theTextAlignment)
{
    CString strU_(_TEXT("L"));
	if(theTextAlignment == kRight)
		strU_ = _TEXT("R");
	else if(theTextAlignment == kCenter)
        strU_ = _TEXT("C");
	else if(theTextAlignment == kTopLeft)
        strU_ = _TEXT("TL");
	else if(theTextAlignment == kTopCenter)
        strU_ = _TEXT("TC");
	else if(theTextAlignment == kTopRight)
        strU_ = _TEXT("TR");
	else if(theTextAlignment == kBottomLeft)
        strU_ = _TEXT("BL");
	else if(theTextAlignment == kBottomCenter)
        strU_ = _TEXT("BC");
	else if(theTextAlignment == kBottomRight)
        strU_ = _TEXT("BR");
    return strU_;
}

static FmiDirection GetAlignmentFromStr(const CString &theAlignStr)
{
    CString compStrU_ = theAlignStr;
    compStrU_.MakeUpper();
	FmiDirection textAlignment = kLeft;
    if(compStrU_ == CString(_TEXT("R")))
		textAlignment = kRight;
    else if(compStrU_ == CString(_TEXT("C")))
		textAlignment = kCenter;
    else if(compStrU_ == CString(_TEXT("TL")))
		textAlignment = kTopLeft;
    else if(compStrU_ == CString(_TEXT("TC")))
		textAlignment = kTopCenter;
    else if(compStrU_ == CString(_TEXT("TR")))
		textAlignment = kTopRight;
    else if(compStrU_ == CString(_TEXT("BL")))
		textAlignment = kBottomLeft;
    else if(compStrU_ == CString(_TEXT("BC")))
		textAlignment = kBottomCenter;
    else if(compStrU_ == CString(_TEXT("BR")))
		textAlignment = kBottomRight;
	return textAlignment;
}

BOOL CFmiTempLineSettings::OnInitDialog()
{
	CDialog::OnInitDialog();

	InitDialogTexts();
	CWnd *win = GetDlgItem(IDC_STATIC_TITLE_STR);
	if(win)
        win->SetWindowText(itsTitleStrU_);

	itsLineWidthSlider.SetRange(1, 5);
	itsLineWidthSlider.SetPos(itsLineInfo.Thickness());

	fDrawLine = itsLineInfo.DrawLine();
	itsLineType = itsLineInfo.LineType();
	itsLabelOffsetX = static_cast<int>(itsLabelInfo.StartPointPixelOffSet().X());
	itsLabelOffsetY = static_cast<int>(itsLabelInfo.StartPointPixelOffSet().Y());
	itsLabelFontSize = itsLabelInfo.FontSize();
	fDrawLabel = itsLabelInfo.DrawLabelText();
	fDrawLabelInDataBox = itsLabelInfo.ClipWithDataRect();
    itsLabelFontAlignmentStrU_ = ::GetAlignmentStr(itsLabelInfo.TextAlignment());

	NFmiColor tColor = itsLineInfo.Color();
	itsColorRef = CtrlView::Color2ColorRef(tColor);
	CtrlView::InitialButtonColorUpdate(this, itsColorRef, &itsColorBitmap, itsColorRect, itsColorButtom);

	UpdateData(FALSE);

	EnableControls();
	UpdateLineThicknessStr();
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CFmiTempLineSettings::UpdateLineThicknessStr(void)
{
	CWnd *win = GetDlgItem(IDC_STATIC_LINE_WIDTH_STR);
	if(win)
	{
		UpdateData(TRUE);
        CString strU_ = CA2T(NFmiStringTools::Convert<int>(itsLineWidthSlider.GetPos()).c_str());
        strU_ += _TEXT(" pix");
        win->SetWindowText(strU_);
		UpdateData(FALSE);
	}
}

void CFmiTempLineSettings::EnableControls(void)
{
	// jos tavallinen help line piirto asetukset (esim. mix/dry/moist jne.), kaikki kontrollit ovat käytössä.
	// Mutta jos lämpötilan tai kastepiste viivan asetuksista kyse, saa säätää vain tiettyjä juttuja
	if(fIsHelpText)
	{
		CWnd *win = GetDlgItem(IDC_BUTTON_COLOR);
		if(win)
			win->EnableWindow(FALSE);
		win = GetDlgItem(IDC_SLIDER_LINE_WIDTH);
		if(win)
			win->EnableWindow(FALSE);
		win = GetDlgItem(IDC_CHECK_DRAW_LINE);
		if(win)
			win->EnableWindow(FALSE);
		win = GetDlgItem(IDC_EDIT_LINE_TYPE);
		if(win)
			win->EnableWindow(FALSE);
		win = GetDlgItem(IDC_STATIC_WIDTH_SLIDER_CAPTION);
		if(win)
			win->EnableWindow(FALSE);
		win = GetDlgItem(IDC_STATIC_LINE_WIDTH_STR);
		if(win)
			win->EnableWindow(FALSE);
	}
	else if(!fIsHelpLine)
	{
		CWnd *win = GetDlgItem(IDC_BUTTON_COLOR);
		if(win)
			win->EnableWindow(FALSE);
		win = GetDlgItem(IDC_EDIT_LABEL_OFFSET_X);
		if(win)
			win->EnableWindow(FALSE);
		win = GetDlgItem(IDC_EDIT_LABEL_OFFSET_Y);
		if(win)
			win->EnableWindow(FALSE);
		win = GetDlgItem(IDC_EDIT_LABEL_FONT_SIZE);
		if(win)
			win->EnableWindow(FALSE);
		win = GetDlgItem(IDC_CHECK_DRAW_LABEL);
		if(win)
			win->EnableWindow(FALSE);
		win = GetDlgItem(IDC_CHECK_CLIP_LABEL);
		if(win)
			win->EnableWindow(FALSE);
	}
}

void CFmiTempLineSettings::OnOK()
{
	UpdateData(TRUE);
	// Kopioidaan mahd. muutokset takaisin alkuperäisiin
	itsLineInfo.Thickness(itsLineWidthSlider.GetPos());
	itsLineInfo.DrawLine(fDrawLine == TRUE);
	itsLineInfo.LineType(static_cast<FmiPattern>(itsLineType));
	NFmiPoint p(itsLabelOffsetX, itsLabelOffsetY);
	itsLabelInfo.StartPointPixelOffSet(p);
	itsLabelInfo.FontSize(itsLabelFontSize);
	itsLabelInfo.DrawLabelText(fDrawLabel == TRUE);
	itsLabelInfo.ClipWithDataRect(fDrawLabelInDataBox == TRUE);
    itsLabelInfo.TextAlignment(::GetAlignmentFromStr(itsLabelFontAlignmentStrU_));

	NFmiColor tColor;
	tColor.SetRGB((float(GetRValue(itsColorRef))/float(255.0)),
				  (float(GetGValue(itsColorRef))/float(255.0)),
				  (float(GetBValue(itsColorRef))/float(255.0)));
	itsLineInfo.Color(tColor);

	*itsLineInfoOrig = itsLineInfo;
	*itsLabelInfoOrig = itsLabelInfo;

	CDialog::OnOK();
}

void CFmiTempLineSettings::OnCancel()
{
	// ei tehdä mitään

	CDialog::OnCancel();
}

void CFmiTempLineSettings::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	UpdateLineThicknessStr();

	CDialog::OnHScroll(nSBCode, nPos, pScrollBar);
}

void CFmiTempLineSettings::OnBnClickedButtonColor()
{
    CtrlView::ColorButtonPressed(this, itsColorRef, &itsColorBitmap, itsColorRect, itsColorButtom);
}

// Tämä funktio alustaa kaikki dialogin tekstit editoriin valitulla kielellä.
// Tämä on ikävä kyllä tehtävä erikseen dialogin muokkaus työkalusta, eli
// tekijän pitää lisätä erikseen tänne kaikki dialogin osat, joihin
// kieli valinta voi vaikuttaa.
void CFmiTempLineSettings::InitDialogTexts(void)
{
    SetWindowText(CA2T(::GetDictionaryString("HelpLineSettingDlgTitle").c_str()));
	CFmiWin32Helpers::SetDialogItemText(this, IDOK, "IDOK");
	CFmiWin32Helpers::SetDialogItemText(this, IDCANCEL, "IDCANCEL");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_HELP_LINE_PROPERTIES_STR, "IDC_STATIC_HELP_LINE_PROPERTIES_STR");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_WIDTH_SLIDER_CAPTION, "IDC_STATIC_WIDTH_SLIDER_CAPTION");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_HELP_LINE_TYPE_STR, "IDC_STATIC_HELP_LINE_TYPE_STR");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_CHECK_DRAW_LINE, "IDC_CHECK_DRAW_LINE");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_HELP_LINE_LABEL_SET_STR, "IDC_STATIC_HELP_LINE_LABEL_SET_STR");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_HELP_LINE_LABEL_OFFSET_STR, "IDC_STATIC_HELP_LINE_LABEL_OFFSET_STR");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_HELP_LINE_LABEL_FONT_SIZE_STR, "IDC_STATIC_HELP_LINE_LABEL_FONT_SIZE_STR");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_CHECK_DRAW_LABEL, "IDC_CHECK_DRAW_LABEL");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_CHECK_CLIP_LABEL, "IDC_CHECK_CLIP_LABEL");
}
