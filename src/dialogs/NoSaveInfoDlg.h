#pragma once

// NoSaveInfoDlg.h : header file
//
#include "SmartMetDialogs_resource.h"

/////////////////////////////////////////////////////////////////////////////
// CNoSaveInfoDlg dialog

class CNoSaveInfoDlg : public CDialog
{
// Construction
public:
	CNoSaveInfoDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CNoSaveInfoDlg)
	enum { IDD = IDD_DIALOG_NO_SAVE_INFO };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CNoSaveInfoDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	void InitDialogTexts(void);

	// Generated message map functions
	//{{AFX_MSG(CNoSaveInfoDlg)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

