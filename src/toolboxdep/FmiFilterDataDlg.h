#pragma once

// FmiFilterDataDlg.h : header file
//
#include "SmartMetToolboxDep_resource.h"
#include "NFmiTimeBag.h"

class SmartMetDocumentInterface;

/////////////////////////////////////////////////////////////////////////////
// CFmiFilterDataDlg dialog

class CFmiFilterDataDlg : public CDialog
{
// Construction
public:
	CFmiFilterDataDlg(SmartMetDocumentInterface *smartMetDocumentInterface, CWnd* pParent = NULL);   // standard constructor
    BOOL Create(CWnd* pParentWnd = NULL);

	void Update(void);
	void EnableButtons(void);
	void InitDialogFromDoc(void);
	void SetDocumentValues(void);
	void RefreshApplicationViews(const std::string &reasonForUpdate, bool clearEditedDependentMacroParamCacheData);

// Dialog Data
	//{{AFX_DATA(CFmiFilterDataDlg)
	enum { IDD = IDD_DIALOG_FILTER_DATA };
	CStatic	itsSelectedParamsCtrl;
	CComboBox	itsFilterSelection;
	BOOL	fFilterSelectedLocations;
	BOOL	fUseMasks;
	int		itsParameterSelection;
	BOOL	fUseTimeInterpolation;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFmiFilterDataDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CFmiFilterDataDlg)
	afx_msg void OnButtonRedo();
	afx_msg void OnButtonUndo();
	virtual BOOL OnInitDialog();
	afx_msg void OnButtonAreaFilterAction();
	afx_msg void OnButtonTimeFilterAction();
	afx_msg void OnButtonReset();
	afx_msg void OnRadioParameterSelectionActive();
	afx_msg void OnRadioParameterSelectionAll();
	afx_msg void OnRadioParameterSelectionSelected();
	afx_msg void OnButtonParamSelection();
	afx_msg void OnPaint();
	afx_msg void OnCheckUseTimeInterpolation();
	afx_msg void OnButtonKlapse();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	BOOL IsNewTimeControlNeeded(void);
	void CreateTimeControlView(void);
	void UpdateEditedParamsString(void);
	void SetParamSelectionButtonState(void);
	void InitDialogTexts(void);

	NFmiTimeBag itsEditedDataTimeBag;
	CView* itsAreaFilterView1; // 1. pieni ruudukkoikkuna (alkuaika)
	CView* itsAreaFilterView2; // 2. pieni ruudukkoikkuna (loppuaika)
	CView* itsTimeFilterView1; // 1. pieni aikaan liittyv‰ ruudukkoikkuna (alkuaika)
	CView* itsTimeFilterView2; // 2. pieni aikaan liittyv‰ ruudukkoikkuna (loppuaika)
	CView* itsTimeControlView; // aika s‰‰dot
    SmartMetDocumentInterface *itsSmartMetDocumentInterface;
	int itsMapViewDescTopIndex;

    CString itsSelectedParamsStrU_;

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

