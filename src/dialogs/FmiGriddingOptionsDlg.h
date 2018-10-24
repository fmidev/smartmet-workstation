#pragma once

// FmiGriddingOptionsDlg.h : header file
//
#include "SmartMetDialogs_resource.h"
#include "NFmiGriddingProperties.h"

/////////////////////////////////////////////////////////////////////////////
// CFmiGriddingOptionsDlg dialog

class CFmiGriddingOptionsDlg : public CDialog
{
// Construction
public:
	CFmiGriddingOptionsDlg(const NFmiGriddingProperties &griddingProperties, CWnd* pParent = NULL);   // standard constructor

    const NFmiGriddingProperties& GetModifiedGriddingProperties() const { return itsGriddingProperties; }

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

    NFmiGriddingProperties itsGriddingProperties;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

