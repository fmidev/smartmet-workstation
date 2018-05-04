// FmiProjectionLineSetupDlg.cpp : implementation file
//

#include "stdafx.h"
#include "FmiProjectionLineSetupDlg.h"
#include "NFmiProjectionCurvatureInfo.h"
#include "NFmiDictionaryFunction.h"
#include "NFmiDrawingEnvironment.h"
#include "FmiWin32Helpers.h"
#include "CloneBitmap.h"
#include "CtrlViewWin32Functions.h"
#include "CtrlViewGdiPlusFunctions.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CFmiProjectionLineSetupDlg dialog


CFmiProjectionLineSetupDlg::CFmiProjectionLineSetupDlg(NFmiProjectionCurvatureInfo* theProjInfo, CWnd* pParent /*=NULL*/)
:CDialog(CFmiProjectionLineSetupDlg::IDD, pParent)
,itsProjectionCurvatureInfo(theProjInfo)
,itsColorBitmapLine(0)
,itsColorBitmapLabel(0)
{
	//{{AFX_DATA_INIT(CFmiProjectionLineSetupDlg)
	itsDrawingMode = -1;
	itsLineDensityLat = -1;
	itsLineDensityLon = -1;
	itsLineSmoothness = -1;
	//}}AFX_DATA_INIT
}

CFmiProjectionLineSetupDlg::~CFmiProjectionLineSetupDlg()
{
	CtrlView::DestroyBitmap(&itsColorBitmapLine);
    CtrlView::DestroyBitmap(&itsColorBitmapLabel);
}

void CFmiProjectionLineSetupDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CFmiProjectionLineSetupDlg)
	DDX_Radio(pDX, IDC_RADIO_DRAWING_MODE1, itsDrawingMode);
	DDX_Radio(pDX, IDC_RADIO_LINE_LAT_DENSITY5, itsLineDensityLat);
	DDX_Radio(pDX, IDC_RADIO_LINE_LON_DENSITY1, itsLineDensityLon);
	DDX_Radio(pDX, IDC_RADIO_LINE_SMOOTHNESS1, itsLineSmoothness);
	DDX_Control(pDX, IDC_BUTTON_PROJECTION_LINE_COLOR, itsProjectionLineColorButtom);
	DDX_Control(pDX, IDC_BUTTON_PROJECTION_LABEL_COLOR, itsProjectionLabelColorButtom);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CFmiProjectionLineSetupDlg, CDialog)
	//{{AFX_MSG_MAP(CFmiProjectionLineSetupDlg)
	ON_WM_HSCROLL()
	ON_BN_CLICKED(IDC_BUTTON_PROJECTION_LINE_COLOR, OnBnClickedButtonPRojectionLineColor)
	ON_BN_CLICKED(IDC_BUTTON_PROJECTION_LABEL_COLOR, OnBnClickedButtonPRojectionLabelColor)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFmiProjectionLineSetupDlg message handlers

void CFmiProjectionLineSetupDlg::OnBnClickedButtonPRojectionLineColor()
{
    CtrlView::ColorButtonPressed(this, itsColorRefLine, &itsColorBitmapLine, itsColorRectLine, itsProjectionLineColorButtom);
}

void CFmiProjectionLineSetupDlg::OnBnClickedButtonPRojectionLabelColor()
{
    CtrlView::ColorButtonPressed(this, itsColorRefLabel, &itsColorBitmapLabel, itsColorRectLabel, itsProjectionLabelColorButtom);
}

BOOL CFmiProjectionLineSetupDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	HICON hIcon = CCloneBitmap::BitmapToIcon(FMI_LOGO_BITMAP, ColorPOD(160, 160, 164));
	this->SetIcon(hIcon, FALSE);

	InitDialogTexts();
	if(itsProjectionCurvatureInfo)
	{
		itsDrawingMode = itsProjectionCurvatureInfo->GetDrawingMode();
		itsLineDensityLat = itsProjectionCurvatureInfo->LineDensityModeLat();
		itsLineDensityLon = itsProjectionCurvatureInfo->LineDensityModeLon();
		itsLineSmoothness = itsProjectionCurvatureInfo->GetLineSmoothness();
	}
	CtrlView::InitialButtonColorUpdate(this, itsProjectionCurvatureInfo->PrimaryLineEnvi()->GetFrameColor(), itsColorRefLine, &itsColorBitmapLine, itsColorRectLine, itsProjectionLineColorButtom);
    CtrlView::InitialButtonColorUpdate(this, itsProjectionCurvatureInfo->PrimaryLineLabelEnvi()->GetFrameColor(), itsColorRefLabel, &itsColorBitmapLabel, itsColorRectLabel, itsProjectionLabelColorButtom);

	UpdateData(FALSE);
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CFmiProjectionLineSetupDlg::OnOK()
{
	UpdateData(TRUE);
	if(itsProjectionCurvatureInfo)
	{
		itsProjectionCurvatureInfo->SetDrawingMode((NFmiProjectionCurvatureInfo::DrawingMode)itsDrawingMode);
		itsProjectionCurvatureInfo->LineDensityModeLat((NFmiProjectionCurvatureInfo::DensityMode)itsLineDensityLat);
		itsProjectionCurvatureInfo->LineDensityModeLon((NFmiProjectionCurvatureInfo::DensityMode)itsLineDensityLon);
		itsProjectionCurvatureInfo->SetLineSmoothness((NFmiProjectionCurvatureInfo::LineSmoothness)itsLineSmoothness);

		itsProjectionCurvatureInfo->PrimaryLineEnvi()->SetFrameColor(CtrlView::ColorRef2Color(itsColorRefLine));
		itsProjectionCurvatureInfo->PrimaryLineLabelEnvi()->SetFrameColor(CtrlView::ColorRef2Color(itsColorRefLabel));
	}

	CDialog::OnOK();
}

void CFmiProjectionLineSetupDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	// TODO: Add your message handler code here and/or call default

	CDialog::OnHScroll(nSBCode, nPos, pScrollBar);
}

// Tämä funktio alustaa kaikki dialogin tekstit editoriin valitulla kielellä.
// Tämä on ikävä kyllä tehtävä erikseen dialogin muokkaus työkalusta, eli
// tekijän pitää lisätä erikseen tänne kaikki dialogin osat, joihin
// kieli valinta voi vaikuttaa.
void CFmiProjectionLineSetupDlg::InitDialogTexts(void)
{
	SetWindowText(CA2T(::GetDictionaryString("ProjectionLineSettingDlgTitle").c_str()));
	CFmiWin32Helpers::SetDialogItemText(this, IDOK, "IDOK");
	CFmiWin32Helpers::SetDialogItemText(this, IDCANCEL, "IDCANCEL");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_PROJECTION_DRAW_MODE, "IDC_STATIC_PROJECTION_DRAW_MODE");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_RADIO_DRAWING_MODE1, "IDC_RADIO_DRAWING_MODE1");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_RADIO_DRAWING_MODE2, "IDC_RADIO_DRAWING_MODE2");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_RADIO_DRAWING_MODE3, "IDC_RADIO_DRAWING_MODE3");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_PROJECTION_LINE_SMOOTH_STR, "IDC_STATIC_PROJECTION_LINE_SMOOTH_STR");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_RADIO_LINE_SMOOTHNESS1, "IDC_RADIO_LINE_SMOOTHNESS1");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_RADIO_LINE_SMOOTHNESS2, "IDC_RADIO_LINE_SMOOTHNESS2");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_RADIO_LINE_SMOOTHNESS3, "IDC_RADIO_LINE_SMOOTHNESS3");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_RADIO_LINE_SMOOTHNESS4, "IDC_RADIO_LINE_SMOOTHNESS4");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_PROJECTION_LINE_DENSITY_LON_STR, "IDC_STATIC_PROJECTION_LINE_DENSITY_LON_STR");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_PROJECTION_LINE_DENSITY_LAT_STR, "IDC_STATIC_PROJECTION_LINE_DENSITY_LAT_STR");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_RADIO_LINE_LON_DENSITY1, "IDC_RADIO_LINE_LON_DENSITY1");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_RADIO_LINE_LON_DENSITY2, "IDC_RADIO_LINE_LON_DENSITY2");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_RADIO_LINE_LON_DENSITY3, "IDC_RADIO_LINE_LON_DENSITY3");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_RADIO_LINE_LON_DENSITY4, "IDC_RADIO_LINE_LON_DENSITY4");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_RADIO_LINE_LAT_DENSITY5, "IDC_RADIO_LINE_LAT_DENSITY5");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_RADIO_LINE_LAT_DENSITY6, "IDC_RADIO_LINE_LAT_DENSITY6");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_RADIO_LINE_LAT_DENSITY7, "IDC_RADIO_LINE_LAT_DENSITY7");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_RADIO_LINE_LAT_DENSITY8, "IDC_RADIO_LINE_LAT_DENSITY8");
}
