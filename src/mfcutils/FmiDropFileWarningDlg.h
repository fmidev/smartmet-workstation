#pragma once

// FmiDropFileWarningDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CFmiDropFileWarningDlg dialog

class CFmiDropFileWarningDlg : public CDialog
{
// Construction
public:
	CFmiDropFileWarningDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CFmiDropFileWarningDlg)
	enum { IDD = IDD_DIALOG_DROP_FILE_WARNING };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFmiDropFileWarningDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CFmiDropFileWarningDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

