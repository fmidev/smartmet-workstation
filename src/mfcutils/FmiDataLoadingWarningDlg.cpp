// FmiDataLoadingWarningDlg.cpp : implementation file
//

#include "stdafx.h"
#include "FmiDataLoadingWarningDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CFmiDataLoadingWarningDlg dialog


CFmiDataLoadingWarningDlg::CFmiDataLoadingWarningDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CFmiDataLoadingWarningDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CFmiDataLoadingWarningDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CFmiDataLoadingWarningDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CFmiDataLoadingWarningDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CFmiDataLoadingWarningDlg, CDialog)
	//{{AFX_MSG_MAP(CFmiDataLoadingWarningDlg)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFmiDataLoadingWarningDlg message handlers
