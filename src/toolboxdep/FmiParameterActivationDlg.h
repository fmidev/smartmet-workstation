#pragma once

// FmiParameterActivationDlg.h : header file
//
#include "SmartMetToolboxDep_resource.h"
#include "NFmiParamBag.h"
/////////////////////////////////////////////////////////////////////////////
// CFmiParameterActivationDlg dialog

class CFmiParameterActivationDlg : public CDialog
{
// Construction
public:
//	CFmiParameterActivationDlg(CWnd* pParent = NULL);   // standard constructor
	CFmiParameterActivationDlg(const NFmiParamBag& theParamBag, const NFmiParamBag* theHelpParamBag = 0, CWnd* pParent = NULL);
	~CFmiParameterActivationDlg(void);
	const NFmiParamBag& ParamBag(void){return itsParamBag;};
	NFmiParamBag* HelpParamBag(void){return itsHelpParamBag;};

// Dialog Data
	//{{AFX_DATA(CFmiParameterActivationDlg)
	enum { IDD = IDD_DIALOG_ACTIVATE_PARAMS };
    CString	itsParamname1U_;
    CString	itsParamname2U_;
    CString	itsParamname10U_;
    CString	itsParamname3U_;
    CString	itsParamname4U_;
    CString	itsParamname5U_;
    CString	itsParamname6U_;
    CString	itsParamname7U_;
    CString	itsParamname8U_;
    CString	itsParamname9U_;
	BOOL	fParamActivation1;
	BOOL	fParamActivation10;
	BOOL	fParamActivation2;
	BOOL	fParamActivation3;
	BOOL	fParamActivation4;
	BOOL	fParamActivation5;
	BOOL	fParamActivation6;
	BOOL	fParamActivation7;
	BOOL	fParamActivation8;
	BOOL	fParamActivation9;
	BOOL	fParamActivation11;
    CString	itsParamname11U_;
	BOOL	fToggleAllParamState;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFmiParameterActivationDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CFmiParameterActivationDlg)
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	afx_msg void OnButtonSubparams1();
	afx_msg void OnButtonSubparams10();
	afx_msg void OnButtonSubparams2();
	afx_msg void OnButtonSubparams3();
	afx_msg void OnButtonSubparams4();
	afx_msg void OnButtonSubparams5();
	afx_msg void OnButtonSubparams6();
	afx_msg void OnButtonSubparams7();
	afx_msg void OnButtonSubparams8();
	afx_msg void OnButtonSubparams9();
	afx_msg void OnButtonSubparams11();
	afx_msg void OnButtonHelpParams();
	afx_msg void OnCheckToggleAllParams();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	NFmiParamBag itsParamBag;
	NFmiParamBag* itsHelpParamBag;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

