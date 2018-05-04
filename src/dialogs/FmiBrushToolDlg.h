#pragma once
// FmiBrushToolDlg.h : header file
//
#include "SmartMetDialogs_resource.h"
#include "NFmiParameterName.h"
/////////////////////////////////////////////////////////////////////////////
// CFmiBrushToolDlg dialog
class SmartMetDocumentInterface;
class NFmiStringList;

class CFmiBrushToolDlg : public CDialog
{
// Construction
public:
	CFmiBrushToolDlg(SmartMetDocumentInterface *smartMetDocumentInterface, CWnd* pParent = NULL);   // standard constructor
	~CFmiBrushToolDlg(void);
	BOOL Create(void); // modaalittomaa varten
	void Update(void);
	void EnableButtons();

// Dialog Data
	//{{AFX_DATA(CFmiBrushToolDlg)
	enum { IDD = IDD_DIALOG_BRUSH_TOOL };
	CComboBox	itsDiscreteBrushvalueCombo;
	CSliderCtrl	itsBrushSizeSlider;
	BOOL	fUseMask;
    CString	itsBrushValueStrU_;
    CString	itsBrushSizeStrU_;
    CString	itsUnitStrU_;
	int		itsLimitingOption;
    CString	itsBrushLimitingValueStrU_;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFmiBrushToolDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CFmiBrushToolDlg)
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	virtual BOOL OnInitDialog();
	afx_msg void OnClose();
	afx_msg void OnCheckUseMasks();
	afx_msg void OnChangeEditBrushValue();
	afx_msg void OnSelchangeComboDiscreteValueSelectio();
	afx_msg void OnButtonRedo();
	afx_msg void OnButtonUndo();
	afx_msg void OnChangeEditBrushLimitingValue();
	afx_msg void OnRadioLimitingOption1();
	afx_msg void OnRadioLimitingOption2();
	afx_msg void OnRadioLimitingOption3();
	afx_msg void OnRadioLimitingOption4();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	void UpdateParamUnitString(void);
	void UpdateBrushSizeString(void);
	void FillSpecialValueCombo(void);
	void SetSelectedDiscreteValueToDocument(void);
	NFmiStringList* GetDiscreteList(FmiParameterName theParamId);
	void SetLimitingOptionToDoc(void);
	bool EnableDlgItem(int theDlgId, bool fEnable);
	void DoWhenClosing(void);
	void InitDialogTexts(void);

    SmartMetDocumentInterface *itsSmartMetDocumentInterface;
	int itsMapViewDescTopIndex;
	NFmiStringList* itsFogIntensityValueList;
	NFmiStringList* itsPrecipitationTypeValueList;
	NFmiStringList* itsPrecipitationFormValueList;

	int itsLastSpecialParameterIndex;
	FmiParameterName itsLastSpecialParameterName;
	FmiParameterName itsLastParameterName;
	virtual void OnCancel();
	virtual void OnOK();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

