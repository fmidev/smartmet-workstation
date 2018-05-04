#pragma once

// FmiShortCutsDlg2.h : header file
//
#include "SmartMetDialogs_resource.h"

/////////////////////////////////////////////////////////////////////////////
// CFmiShortCutsDlg2 dialog

class CFmiShortCutsDlg2 : public CDialog
{
// Construction
public:
	CFmiShortCutsDlg2(CWnd* pParent = NULL);   // standard constructor

	void DeselectTexts(void);
// Dialog Data
	//{{AFX_DATA(CFmiShortCutsDlg2)
	enum { IDD = IDD_DIALOG_SHORT_CUTS };
    CString	itsTextU_;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFmiShortCutsDlg2)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CFmiShortCutsDlg2)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelectAll();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	void InitDialogTexts(void);
	void DoWhenClosing(void);
public:
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnClose();
protected:
	virtual void OnCancel();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

