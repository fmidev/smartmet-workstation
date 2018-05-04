#pragma once

// CloseDlg.h : header file
//
#include "SmartMetDialogs_resource.h"

/////////////////////////////////////////////////////////////////////////////
// CCloseDlg dialog

class CCloseDlg : public CDialog
{
// Construction
public:
	CCloseDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CCloseDlg)
	enum { IDD = IDD_DIALOG_CLOSE };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCloseDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	void InitDialogTexts(void);

	// Generated message map functions
	//{{AFX_MSG(CCloseDlg)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

