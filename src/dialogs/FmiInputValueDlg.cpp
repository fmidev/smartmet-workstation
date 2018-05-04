// FmiInputValueDlg.cpp : implementation file
//

#include "stdafx.h"
#include "FmiInputValueDlg.h"
#include "afxdialogex.h"


// CFmiInputValueDlg dialog

IMPLEMENT_DYNAMIC(CFmiInputValueDlg, CDialogEx)

CFmiInputValueDlg::CFmiInputValueDlg(const CString &theHelpStrU_, CWnd* pParent)
	: CDialogEx(CFmiInputValueDlg::IDD, pParent)
    , itsValueU_()
    , itsHelpTextU_(theHelpStrU_)
{

}

CFmiInputValueDlg::~CFmiInputValueDlg()
{
}

void CFmiInputValueDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
    DDX_Text(pDX, IDC_EDIT_VALUE, itsValueU_);
}


BEGIN_MESSAGE_MAP(CFmiInputValueDlg, CDialogEx)
END_MESSAGE_MAP()


// CFmiInputValueDlg message handlers

BOOL CFmiInputValueDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

//	SetWindowText(itsTitleStr.c_str());
	CWnd *win = GetDlgItem(IDC_STATIC_IMPUT_VALUES_STR);
	if(win)
        win->SetWindowText(itsHelpTextU_);

	UpdateData(FALSE);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CFmiInputValueDlg::OnOK()
{
	UpdateData(TRUE);

	CDialogEx::OnOK();
}
