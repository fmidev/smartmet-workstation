#pragma once
// DataLoadingProblemsDlg.h : header file
//

#include "SmartMetMfcUtils_resource.h"

/////////////////////////////////////////////////////////////////////////////
// CDataLoadingProblemsDlg dialog

class CDataLoadingProblemsDlg : public CDialog
{
// Construction
public:
	CDataLoadingProblemsDlg(const CString &theErrorStr, CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDataLoadingProblemsDlg)
	enum { IDD = IDD_DIALOG_DATA_LOADINGPROBLEMS };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDataLoadingProblemsDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CDataLoadingProblemsDlg)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	// Tähän talletetaan laatikossa oleva stringi
    CString itsErrorWindowStringU_;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

