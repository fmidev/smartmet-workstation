// SaveDataDlg.cpp : implementation file
//

#include "stdafx.h"
#include "SaveDataDlg.h"
#include "NFmiDictionaryFunction.h"
#include "FmiWin32Helpers.h"
#include "CloneBitmap.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSaveDataDlg dialog


CSaveDataDlg::CSaveDataDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSaveDataDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSaveDataDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CSaveDataDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSaveDataDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSaveDataDlg, CDialog)
	//{{AFX_MSG_MAP(CSaveDataDlg)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSaveDataDlg message handlers

BOOL CSaveDataDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	CFmiWin32Helpers::SetUsedWindowIconDynamically(this);
	InitDialogTexts();

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

// Tämä funktio alustaa kaikki dialogin tekstit editoriin valitulla kielellä.
// Tämä on ikävä kyllä tehtävä erikseen dialogin muokkaus työkalusta, eli
// tekijän pitää lisätä erikseen tänne kaikki dialogin osat, joihin
// kieli valinta voi vaikuttaa.
void CSaveDataDlg::InitDialogTexts(void)
{
    SetWindowText(CA2T(::GetDictionaryString("CSaveDataDlgTitle").c_str()));
	CFmiWin32Helpers::SetDialogItemText(this, IDOK, "NormalWordCapitalYes");
	CFmiWin32Helpers::SetDialogItemText(this, IDCANCEL, "NormalWordCapitalNo");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_SAVE_DATA_STR, "IDC_STATIC_SAVE_DATA_STR");
}
