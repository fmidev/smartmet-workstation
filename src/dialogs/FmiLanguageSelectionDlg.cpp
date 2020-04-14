// FmiLanguageSelectionDlg.cpp : implementation file
//

#include "stdafx.h"
#include "FmiLanguageSelectionDlg.h"
#include "NFmiDictionaryFunction.h"
#include "FmiWin32Helpers.h"
#include "CloneBitmap.h"


// CFmiLanguageSelectionDlg dialog

IMPLEMENT_DYNAMIC(CFmiLanguageSelectionDlg, CDialog)
CFmiLanguageSelectionDlg::CFmiLanguageSelectionDlg(int theLanguageSelection, CWnd* pParent /*=NULL*/)
	: CDialog(CFmiLanguageSelectionDlg::IDD, pParent)
	, itsLanguageSelection(theLanguageSelection)
{
}

CFmiLanguageSelectionDlg::~CFmiLanguageSelectionDlg()
{
}

void CFmiLanguageSelectionDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);

	DDX_Radio(pDX, IDC_RADIO_SELECTION1, itsLanguageSelection);
}


BEGIN_MESSAGE_MAP(CFmiLanguageSelectionDlg, CDialog)
END_MESSAGE_MAP()


// CFmiLanguageSelectionDlg message handlers

BOOL CFmiLanguageSelectionDlg::OnInitDialog()
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
void CFmiLanguageSelectionDlg::InitDialogTexts(void)
{
	SetWindowText(CA2T(::GetDictionaryString("CFmiLanguageSelectionDlgTitle").c_str()));
	CFmiWin32Helpers::SetDialogItemText(this, IDOK, "IDOK");
	CFmiWin32Helpers::SetDialogItemText(this, IDCANCEL, "IDCANCEL");
}
