// FmiGriddingOptionsDlg.cpp : implementation file
//
#ifdef _MSC_VER
#pragma warning(disable : 4786) // poistaa n kpl VC++ kääntäjän "liian pitkä tyyppi nimi" varoitusta
#endif

#include "stdafx.h"
#include "FmiGriddingOptionsDlg.h"
#include "NFmiEditorControlPointManager.h"
#include "NFmiDictionaryFunction.h"
#include "FmiWin32Helpers.h"
#include "CloneBitmap.h"
#include "SmartMetDocumentInterface.h"
#include "NFmiApplicationWinRegistry.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CFmiGriddingOptionsDlg dialog


CFmiGriddingOptionsDlg::CFmiGriddingOptionsDlg(SmartMetDocumentInterface *smartMetDocumentInterface, CWnd* pParent)
:CDialogEx(CFmiGriddingOptionsDlg::IDD, pParent)
,itsSmartMetDocumentInterface(smartMetDocumentInterface)
,itsGriddingProperties()
{
	//{{AFX_DATA_INIT(CFmiGriddingOptionsDlg)
	itsGriddingFunction = -1;
	//}}AFX_DATA_INIT
    itsGriddingProperties = itsSmartMetDocumentInterface->ApplicationWinRegistry().GriddingProperties(true);
}

void CFmiGriddingOptionsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CFmiGriddingOptionsDlg)
	DDX_Radio(pDX, IDC_RADIO_GRID_FUNCTION1, itsGriddingFunction);
    DDX_Text(pDX, IDC_EDIT_RANGE_LIMIT_IN_KM, itsRangeLimitInKm);
    DDV_MinMaxDouble(pDX, itsRangeLimitInKm, 0., 10000.);
    DDX_Control(pDX, IDC_COMBO_LOCAL_FIT_METHOD, itsLocalFitMethodSelector);
    DDX_Text(pDX, IDC_EDIT_LOCAL_FIT_DELTA, itsLocalFitDelta);
    DDV_MinMaxDouble(pDX, itsLocalFitDelta, 0.01, 5.);
    DDX_Text(pDX, IDC_EDIT_SMOOTH_LEVEL, itsSmoothLevel);
    DDV_MinMaxInt(pDX, itsSmoothLevel, 0, 5);

    DDX_Text(pDX, IDC_EDIT_LOCAL_FIT_FILTER_RANGE, itsLocalFitFilterRadius);
    DDV_MinMaxDouble(pDX, itsLocalFitFilterRadius, 0.01, 5.);
    DDX_Text(pDX, IDC_EDIT_LOCAL_FIT_FILTER_FACTOR, itsLocalFitFilterFactor);
    DDV_MinMaxDouble(pDX, itsLocalFitFilterFactor, 0.01, 5.);
    //}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CFmiGriddingOptionsDlg, CDialogEx)
	//{{AFX_MSG_MAP(CFmiGriddingOptionsDlg)
	//}}AFX_MSG_MAP
    ON_BN_CLICKED(IDC_BUTTON_DEFAULT_VALUES, &CFmiGriddingOptionsDlg::OnBnClickedButtonDefaultValues)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFmiGriddingOptionsDlg message handlers

BOOL CFmiGriddingOptionsDlg::OnInitDialog() 
{
	CDialogEx::OnInitDialog();
	
	HICON hIcon = CCloneBitmap::BitmapToIcon(FMI_LOGO_BITMAP, ColorPOD(160, 160, 164));
	this->SetIcon(hIcon, FALSE);

	InitDialogTexts();
    InitLocalFitMethodSelector();
    InitControlValuesFromGriddingPropertiesObject();
	UpdateData(FALSE);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CFmiGriddingOptionsDlg::InitControlValuesFromGriddingPropertiesObject()
{
    itsGriddingFunction = itsGriddingProperties.function();
    itsRangeLimitInKm = itsGriddingProperties.rangeLimitInKm();
    // Local fit methods are from 1 to 6, and you have to substract 1 to get 0 based index
    itsLocalFitMethodSelector.SetCurSel(itsGriddingProperties.localFitMethod() - 1);
    itsLocalFitDelta = itsGriddingProperties.localFitDelta();
    itsSmoothLevel = itsGriddingProperties.smoothLevel();
    itsLocalFitFilterRadius = itsGriddingProperties.localFitFilterRadius();
    itsLocalFitFilterFactor = itsGriddingProperties.localFitFilterFactor();

    UpdateData(FALSE);
}

void CFmiGriddingOptionsDlg::InitLocalFitMethodSelector()
{
    itsLocalFitMethodSelector.Clear();
    itsLocalFitMethodSelector.InsertString(-1, CA2T(::GetDictionaryString("Weighted Avg").c_str()));
    itsLocalFitMethodSelector.InsertString(-1, CA2T(::GetDictionaryString("Bilinear").c_str()));
    itsLocalFitMethodSelector.InsertString(-1, CA2T(::GetDictionaryString("Bilinear strict").c_str()));
    itsLocalFitMethodSelector.InsertString(-1, CA2T(::GetDictionaryString("W-Avg 2 quads").c_str()));
    itsLocalFitMethodSelector.InsertString(-1, CA2T(::GetDictionaryString("W-Avg 3 quads").c_str()));
    itsLocalFitMethodSelector.InsertString(-1, CA2T(::GetDictionaryString("W-Avg 4 quads").c_str()));
}

void CFmiGriddingOptionsDlg::DoWhenClosing()
{
    UpdateData(TRUE);
    itsGriddingProperties.function(static_cast<FmiGriddingFunction>(itsGriddingFunction));
    itsGriddingProperties.rangeLimitInKm(itsRangeLimitInKm);
    // Local fit methods are from 1 to 6, and you have to add 1 to selected index to get real method value
    itsGriddingProperties.localFitMethod(itsLocalFitMethodSelector.GetCurSel() + 1);
    itsGriddingProperties.localFitDelta(itsLocalFitDelta);
    itsGriddingProperties.smoothLevel(itsSmoothLevel);
    itsGriddingProperties.localFitFilterRadius(itsLocalFitFilterRadius);
    itsGriddingProperties.localFitFilterFactor(itsLocalFitFilterFactor);

    itsSmartMetDocumentInterface->ApplicationWinRegistry().SetGriddingProperties(true, itsGriddingProperties);
}

void CFmiGriddingOptionsDlg::OnOK() 
{
    DoWhenClosing();

	CDialogEx::OnOK();
}

// Tämä funktio alustaa kaikki dialogin tekstit editoriin valitulla kielellä.
// Tämä on ikävä kyllä tehtävä erikseen dialogin muokkaus työkalusta, eli
// tekijän pitää lisätä erikseen tänne kaikki dialogin osat, joihin 
// kieli valinta voi vaikuttaa.
void CFmiGriddingOptionsDlg::InitDialogTexts(void)
{
	SetWindowText(CA2T(::GetDictionaryString("GriddingOptionsDlgTitle").c_str()));
	CFmiWin32Helpers::SetDialogItemText(this, IDOK, "IDOK");
	CFmiWin32Helpers::SetDialogItemText(this, IDCANCEL, "IDCANCEL");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_RADIO_GRID_FUNCTION1, "IDC_RADIO_GRID_FUNCTION1");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_RADIO_GRID_FUNCTION2, "IDC_RADIO_GRID_FUNCTION2");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_RADIO_GRID_FUNCTION3, "IDC_RADIO_GRID_FUNCTION3");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_RADIO_GRID_FUNCTION4, "IDC_RADIO_GRID_FUNCTION4");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_RADIO_GRID_FUNCTION5, "IDC_RADIO_GRID_FUNCTION5");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_RADIO_GRID_FUNCTION6, "IDC_RADIO_GRID_FUNCTION6");

    CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_RANGE_LIMIT_IN_KM_HELP_TEXT, "CP-point affect radius [km] (default 0 means limitless)");
    CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_LOCAL_FIT_METHOD_TEXT, "Local fit interpolation method (default W-Avg)");
    CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_LOCAL_FIT_DELTA_TEXT, "Local fit delta (default 0.5, a limit for forced near CP value to close grid points, range of 0.0 > x >= 5)");
    CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_SMOOTH_LEVEL_TEXT, "How many extra times [0-5] is field smoothed (default is 0 and every extra time slows calculations)");
    CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_LOCAL_FIT_FILTER_RANGE_TEXT, "Local fit filter range, default is 1.25 (value should be in range of 1.0 > x >= 5)");
    CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_LOCAL_FIT_FILTER_FACTOR_TEXT, "Local fit filter factor, default is 0.15 (value should be in range of 0.0 > x >= 5)");
    CFmiWin32Helpers::SetDialogItemText(this, IDC_BUTTON_DEFAULT_VALUES, "Default values");
}



void CFmiGriddingOptionsDlg::OnBnClickedButtonDefaultValues()
{
    itsGriddingProperties = NFmiGriddingProperties(itsGriddingProperties.toolMasterAvailable());
    InitControlValuesFromGriddingPropertiesObject();
}
