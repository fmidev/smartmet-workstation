#pragma once

// FmiGriddingOptionsDlg.h : header file
//
#include "SmartMetDialogs_resource.h"
#include "NFmiGriddingProperties.h"

class SmartMetDocumentInterface;

/////////////////////////////////////////////////////////////////////////////
// CFmiGriddingOptionsDlg dialog

class CFmiGriddingOptionsDlg : public CDialogEx
{
// Construction
public:
	CFmiGriddingOptionsDlg(SmartMetDocumentInterface *smartMetDocumentInterface, CWnd* pParent = NULL);   // standard constructor
    // Update pitää olla UpdateModalessDialog template funktiota varten, jota käytetään tälle luokalle CSmartMetDoc:in syövereissä
    void Update() {}
    const NFmiGriddingProperties& GetModifiedGriddingProperties() const { return itsGriddingProperties; }

// Dialog Data
	//{{AFX_DATA(CFmiGriddingOptionsDlg)
	enum { IDD = IDD_DIALOG_GRIDDING_OPTIONS };
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
    void InitLocalFitMethodSelector();
    void InitControlValuesFromGriddingPropertiesObject();
    void DoWhenClosing();
    void ApplyChanges();

    SmartMetDocumentInterface *itsSmartMetDocumentInterface;
    NFmiGriddingProperties itsGriddingProperties;
    int itsGriddingFunction;
    double itsRangeLimitInKm = 0.; // 0 = rajaton ja muuten > 0
    CComboBox itsLocalFitMethodSelector; // [1,2,3,4,5,6]
    double itsLocalFitDelta = 0.5; // [?]
    int itsSmoothLevel = 0; // [0 - 5]
    double itsLocalFitFilterRadius = 1.25; // > 1
    double itsLocalFitFilterFactor = 0.15; // > 0
public:
    afx_msg void OnBnClickedButtonDefaultValues();
    afx_msg void OnBnClickedButtonApplyChanges();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

