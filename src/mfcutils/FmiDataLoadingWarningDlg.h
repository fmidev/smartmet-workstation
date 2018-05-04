#pragma once

#include "SmartMetMfcUtils_resource.h"

// FmiDataLoadingWarningDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CFmiDataLoadingWarningDlg dialog

class CFmiDataLoadingWarningDlg : public CDialog
{
// Construction
public:
	CFmiDataLoadingWarningDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CFmiDataLoadingWarningDlg)
	enum { IDD = IDD_DIALOG_LOAD_DATA_INCOMPLETE_WARNING };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFmiDataLoadingWarningDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CFmiDataLoadingWarningDlg)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

