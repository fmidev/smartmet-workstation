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
	CSliderCtrl	itsSearchRadiusSlider;
	int		itsToolSelection;
    CString	itsSearchRadiusStringU_;
    CString	itsSelectedToolExplanatoryStringU_;
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
	afx_msg void OnRadioCircleSelection();
	afx_msg void OnRadioSingleSelection();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnButtonSelectionRedo();
	afx_msg void OnButtonSelectionUndo();
	virtual void OnOK();
	afx_msg void OnRadio16(); // select all radio button
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	void InitDialogTexts(void);
	void UpdateSearchRadiusString(void);
	void UpdateSelectionTool(void);
	void UpdateRemarkText(BOOL theFirstNumber = TRUE);
	void RadioButtonPushed(void);

    CString itsAbbreviationU_; // parametrin lyhenne
    SmartMetDocumentInterface *itsSmartMetDocumentInterface;
	NFmiLocationSelectionTool* itsSelectionTool;
	int itsParameterNameTable[30]; // laitetaan tänne talteen parametrien nimet (T = 4 = kFmiTemperature), että niitä voidaan sitten hakea selection tooliin
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

