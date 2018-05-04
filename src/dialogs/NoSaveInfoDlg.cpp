// NoSaveInfoDlg.cpp : implementation file
//

#include "stdafx.h"
#include "NoSaveInfoDlg.h"
#include "NFmiDictionaryFunction.h"
#include "FmiWin32Helpers.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CNoSaveInfoDlg dialog


CNoSaveInfoDlg::CNoSaveInfoDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CNoSaveInfoDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CNoSaveInfoDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CNoSaveInfoDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CNoSaveInfoDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CNoSaveInfoDlg, CDialog)
	//{{AFX_MSG_MAP(CNoSaveInfoDlg)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CNoSaveInfoDlg message handlers

BOOL CNoSaveInfoDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	InitDialogTexts();

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

// Tämä funktio alustaa kaikki dialogin tekstit editoriin valitulla kielellä.
// Tämä on ikävä kyllä tehtävä erikseen dialogin muokkaus työkalusta, eli
// tekijän pitää lisätä erikseen tänne kaikki dialogin osat, joihin 
// kieli valinta voi vaikuttaa.
void CNoSaveInfoDlg::InitDialogTexts(void)
{
    SetWindowText(CA2T(::GetDictionaryString("CNoSaveInfoDlgTitle").c_str()));
	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_NO_SAVE_DLG_STR, "IDC_STATIC_NO_SAVE_DLG_STR");
	CFmiWin32Helpers::SetDialogItemText(this, IDOK, "IDOK");
}
