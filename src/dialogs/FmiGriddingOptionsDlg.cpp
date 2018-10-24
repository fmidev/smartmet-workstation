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

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CFmiGriddingOptionsDlg dialog


CFmiGriddingOptionsDlg::CFmiGriddingOptionsDlg(const NFmiGriddingProperties &griddingProperties, CWnd* pParent)
:CDialog(CFmiGriddingOptionsDlg::IDD, pParent)
,itsGriddingProperties(griddingProperties)
{
	//{{AFX_DATA_INIT(CFmiGriddingOptionsDlg)
	itsGriddingFunction = -1;
	//}}AFX_DATA_INIT
}


void CFmiGriddingOptionsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CFmiGriddingOptionsDlg)
	DDX_Radio(pDX, IDC_RADIO_GRID_FUNCTION1, itsGriddingFunction);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CFmiGriddingOptionsDlg, CDialog)
	//{{AFX_MSG_MAP(CFmiGriddingOptionsDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFmiGriddingOptionsDlg message handlers

BOOL CFmiGriddingOptionsDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	HICON hIcon = CCloneBitmap::BitmapToIcon(FMI_LOGO_BITMAP, ColorPOD(160, 160, 164));
	this->SetIcon(hIcon, FALSE);

	InitDialogTexts();
	itsGriddingFunction = itsGriddingProperties.function();
	UpdateData(FALSE);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CFmiGriddingOptionsDlg::OnOK() 
{
	UpdateData(TRUE);
	itsGriddingProperties.function(static_cast<FmiGriddingFunction>(itsGriddingFunction));
	
	CDialog::OnOK();
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
}
