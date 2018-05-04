#pragma once

// FmiDataValidationDlg.h : header file
//
#include "SmartMetDialogs_resource.h"

/////////////////////////////////////////////////////////////////////////////
// CFmiDataValidationDlg dialog

class NFmiMetEditorOptionsData;

class CFmiDataValidationDlg : public CDialog
{
// Construction
public:
	CFmiDataValidationDlg(NFmiMetEditorOptionsData* theOptionsData, CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CFmiDataValidationDlg)
	enum { IDD = IDD_DIALOG_DATA_VALIDATION };
	BOOL	fDataValidation_PrForm_T;
	BOOL	fDataValidation_T_DP;
	BOOL	fMakeDataValidationAtSendingDB;
    CString	itsSnowLimitStrU_;
    CString	itsRainLimitStrU_;
	BOOL	fMakeValidationsAutomatically;
	BOOL	fUseLargeScalePrecip;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFmiDataValidationDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CFmiDataValidationDlg)
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	void SetDocValues(void);
	void InitDialogTexts(void);

	NFmiMetEditorOptionsData* itsOptionsData;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

