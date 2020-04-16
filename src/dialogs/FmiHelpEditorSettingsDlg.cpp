// FmiHelpEditorSettingsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "FmiHelpEditorSettingsDlg.h"
#include "NFmiHelpEditorSystem.h"
#include "CloneBitmap.h"
#include "XFolderDialog.h"
#include "NFmiDictionaryFunction.h"
#include "FmiWin32Helpers.h"

// CFmiHelpEditorSettingsDlg dialog

IMPLEMENT_DYNAMIC(CFmiHelpEditorSettingsDlg, CDialog)
CFmiHelpEditorSettingsDlg::CFmiHelpEditorSettingsDlg(NFmiHelpEditorSystem &theHelpEditorSystem, CWnd* pParent /*=NULL*/)
:CDialog(CFmiHelpEditorSettingsDlg::IDD, pParent)
,itsHelpEditorSystem(theHelpEditorSystem)
, fUseSystem(FALSE)
, fHelpModeOn(FALSE)
, itsHelpDataPathStrU_(_T(""))
, itsHelpDataBaseFileNameStrU_(_T(""))
{
}

CFmiHelpEditorSettingsDlg::~CFmiHelpEditorSettingsDlg()
{
}

void CFmiHelpEditorSettingsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Check(pDX, IDC_CHECK_HELP_EDITOR_MODE_USE, fUseSystem);
	DDX_Check(pDX, IDC_CHECK_HELP_EDITOR_MODE_ON, fHelpModeOn);
    DDX_Text(pDX, IDC_EDIT_HELP_EDITOR_MODE_PATH, itsHelpDataPathStrU_);
    DDX_Text(pDX, IDC_EDIT_HELP_EDITOR_MODE_FILE_NAME_BASE, itsHelpDataBaseFileNameStrU_);
}


BEGIN_MESSAGE_MAP(CFmiHelpEditorSettingsDlg, CDialog)
	ON_BN_CLICKED(IDC_BUTTON_HELP_EDITOR_MODE_BROWSE, OnBnClickedButtonHelpEditorModeBrowse)
END_MESSAGE_MAP()


// CFmiHelpEditorSettingsDlg message handlers

BOOL CFmiHelpEditorSettingsDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	CFmiWin32Helpers::SetUsedWindowIconDynamically(this);
	fUseSystem = itsHelpEditorSystem.Use();
	fHelpModeOn = itsHelpEditorSystem.HelpEditor();
    itsHelpDataPathStrU_ = CA2T(itsHelpEditorSystem.DataPath().c_str());
    itsHelpDataBaseFileNameStrU_ = CA2T(itsHelpEditorSystem.FileNameBase().c_str());

	UpdateData(FALSE);
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CFmiHelpEditorSettingsDlg::OnOK()
{
	UpdateData(TRUE);

	itsHelpEditorSystem.Use(fUseSystem == TRUE);
	itsHelpEditorSystem.HelpEditor(fHelpModeOn == TRUE);
    std::string tmp = CT2A(itsHelpDataPathStrU_);
	itsHelpEditorSystem.DataPath(tmp);
    tmp = CT2A(itsHelpDataBaseFileNameStrU_);
	itsHelpEditorSystem.FileNameBase(tmp);

	CDialog::OnOK();
}

void CFmiHelpEditorSettingsDlg::OnBnClickedButtonHelpEditorModeBrowse()
{
	UpdateData(TRUE);
    Browse(itsHelpDataPathStrU_);
	UpdateData(FALSE);
}

void CFmiHelpEditorSettingsDlg::Browse(CString & thePathU_)
{
    CXFolderDialog dlg(thePathU_);
    dlg.SetTitle(CA2T(::GetDictionaryString("Select folder").c_str()));
    if(dlg.DoModal() == IDOK)
        thePathU_ = dlg.GetPath();
}
