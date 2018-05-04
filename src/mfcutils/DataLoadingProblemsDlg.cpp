// DataLoadingProblemsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "DataLoadingProblemsDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDataLoadingProblemsDlg dialog


CDataLoadingProblemsDlg::CDataLoadingProblemsDlg(const CString &theErrorStr, CWnd* pParent /*=NULL*/)
	: CDialog(CDataLoadingProblemsDlg::IDD, pParent)
    , itsErrorWindowStringU_(theErrorStr)
{
	//{{AFX_DATA_INIT(CDataLoadingProblemsDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CDataLoadingProblemsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDataLoadingProblemsDlg)
    DDX_Text(pDX, IDC_STATIC_DATA_LOADING_ERROR_STRING, itsErrorWindowStringU_);
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDataLoadingProblemsDlg, CDialog)
	//{{AFX_MSG_MAP(CDataLoadingProblemsDlg)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDataLoadingProblemsDlg message handlers
