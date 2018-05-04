// FmiDropFileWarningDlg.cpp : implementation file
//

#include "stdafx.h"
#include "SmartMetMfcUtils_resource.h"
#include "FmiDropFileWarningDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CFmiDropFileWarningDlg dialog


CFmiDropFileWarningDlg::CFmiDropFileWarningDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CFmiDropFileWarningDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CFmiDropFileWarningDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CFmiDropFileWarningDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CFmiDropFileWarningDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CFmiDropFileWarningDlg, CDialog)
	//{{AFX_MSG_MAP(CFmiDropFileWarningDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

