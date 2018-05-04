#pragma once

// FmiLocationSelectionToolDlg2.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CFmiLocationSelectionToolDlg2 dialog

#include "SmartMetDialogs_resource.h"
#include "NFmiGlobals.h"

class SmartMetDocumentInterface;
class NFmiLocationSelectionTool;
class NFmiValueString;

class CFmiLocationSelectionToolDlg2 : public CDialog
{
// Construction
public:
	CFmiLocationSelectionToolDlg2(SmartMetDocumentInterface *smartMetDocumentInterface, CWnd* pParent = NULL);   // standard constructor
	void Update(void);

// Dialog Data
	//{{AFX_DATA(CFmiLocationSelectionToolDlg2)
	enum { IDD = IDD_DIALOG_LOCATION_SELECTION_TOOL2 };
	CStatic	itsErrorTextControl;
	CSliderCtrl	itsSearchRadiusSlider;
	CComboBox	itsParamList;
	BOOL	fUsePointedValue;
	BOOL	fSearchNeibhor;
    CString	itsLimit1U_;
    CString	itsLimit2U_;
	int		itsToolSelection;
	BOOL	fUseSearchRadius;
    CString	itsLimit1UnitStringU_;
    CString	itsLimit2UnitStringU_;
    CString	itsSearchRadiusStringU_;
    CString	itsSelectedToolExplanatoryStringU_;
    CString	itsErrorStringU_;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFmiLocationSelectionToolDlg2)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CFmiLocationSelectionToolDlg2)
	virtual BOOL OnInitDialog();
	afx_msg void OnCheckEnclosedRadius();
	afx_msg void OnCheckUseClickPointValueAsLimit1();
	afx_msg void OnCheckUseNeibhorSearch();
	afx_msg void OnSelchangeComboParamSelection();
	afx_msg void OnChangeEditRaja1();
	afx_msg void OnChangeEditRaja2();
	afx_msg void OnRadioCircleSelection();
	afx_msg void OnRadioSingleSelection();
	afx_msg void OnRadio10();
	afx_msg void OnRadio11();
	afx_msg void OnRadio12();
	afx_msg void OnRadio13();
	afx_msg void OnRadio14();
	afx_msg void OnRadio2();
	afx_msg void OnRadio3();
	afx_msg void OnRadio4();
	afx_msg void OnRadio5();
	afx_msg void OnRadio6();
	afx_msg void OnRadio7();
	afx_msg void OnRadio8();
	afx_msg void OnRadio9();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnButtonSelectionRedo();
	afx_msg void OnButtonSelectionUndo();
	virtual void OnOK();
	afx_msg void OnDropdownComboParamSelection();
	afx_msg void OnRadio16();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	void InitDialogTexts(void);
	void UpdateParamInfo(void);
	void ClearParamListCombo(void);
	void FillParamListCombo(void);
	void UpdateSearchRadiusString(void);
	void UpdateSelectionTool(void);
	void UpdateRemarkText(BOOL theFirstNumber = TRUE);
	void UpdateHeaderOfLimits(void);
	void RadioButtonPushed(void);
	FmiMaskOperation MaskOperation(void);
	void SetHeaderOfLimits(void);
	double FirstLimit(void);
	double SecondLimit(void);
	void SetRemarkText(BOOL theFirstNumber = TRUE);
	void UpdateStateOfEditCtrl(BOOL updateFromDlg = TRUE);
	void Limit(const NFmiValueString &firstLimit, const NFmiValueString &secondLimit);

    CString itsAbbreviationU_; // parametrin lyhenne
    SmartMetDocumentInterface *itsSmartMetDocumentInterface;
	NFmiLocationSelectionTool* itsSelectionTool;
	int itsParameterNameTable[30]; // laitetaan tänne talteen parametrien nimet (T = 4 = kFmiTemperature), että niitä voidaan sitten hakea selection tooliin
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

