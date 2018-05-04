// fmiGetDirectoryNameDlg.cpp : implementation file
//

#include "stdafx.h"
#include "fmiGetDirectoryNameDlg.h"


// CfmiGetDirectoryNameDlg dialog

IMPLEMENT_DYNAMIC(CfmiGetDirectoryNameDlg, CDialog)
CfmiGetDirectoryNameDlg::CfmiGetDirectoryNameDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CfmiGetDirectoryNameDlg::IDD, pParent)
    , itsDirectoryNameU_(_T(""))
{
}

CfmiGetDirectoryNameDlg::~CfmiGetDirectoryNameDlg()
{
}

void CfmiGetDirectoryNameDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
    DDX_Text(pDX, IDC_EDIT_DIRECTORY_NAME, itsDirectoryNameU_);
}


BEGIN_MESSAGE_MAP(CfmiGetDirectoryNameDlg, CDialog)
	ON_BN_CLICKED(IDOK, OnBnClickedOk)
END_MESSAGE_MAP()


// CfmiGetDirectoryNameDlg message handlers

void CfmiGetDirectoryNameDlg::OnBnClickedOk()
{
	UpdateData(TRUE);
	OnOK();
}
