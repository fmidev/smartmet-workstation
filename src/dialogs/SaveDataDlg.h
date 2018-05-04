#pragma once

// SaveDataDlg.h : header file
//
#include "SmartMetDialogs_resource.h"

/////////////////////////////////////////////////////////////////////////////
// CSaveDataDlg dialog

class CSaveDataDlg : public CDialog
{
// Construction
public:
	CSaveDataDlg(CWnd* pParent = NULL);   // standard constructor
	virtual BOOL OnInitDialog();

// Dialog Data
	//{{AFX_DATA(CSaveDataDlg)
	enum { IDD = IDD_DIALOG_SAVEDATA };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSaveDataDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	void InitDialogTexts(void);

	// Generated message map functions
	//{{AFX_MSG(CSaveDataDlg)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

