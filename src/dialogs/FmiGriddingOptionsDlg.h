#pragma once

// FmiGriddingOptionsDlg.h : header file
//
#include "SmartMetDialogs_resource.h"

struct NFmiCPGriddingProperties;
/////////////////////////////////////////////////////////////////////////////
// CFmiGriddingOptionsDlg dialog

class CFmiGriddingOptionsDlg : public CDialog
{
// Construction
public:
	CFmiGriddingOptionsDlg(NFmiCPGriddingProperties *theOptions, CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CFmiGriddingOptionsDlg)
	enum { IDD = IDD_DIALOG_GRIDDING_OPTIONS };
	int		itsGriddingFunction;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFmiGriddingOptionsDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CFmiGriddingOptionsDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	void InitDialogTexts(void);

	NFmiCPGriddingProperties *itsGriddingOptions;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

