#pragma once

// FmiProjectionLineSetupDlg.h : header file
//
#include "SmartMetDialogs_resource.h"

/////////////////////////////////////////////////////////////////////////////
// CFmiProjectionLineSetupDlg dialog

class NFmiProjectionCurvatureInfo;
class NFmiColor;

class CFmiProjectionLineSetupDlg : public CDialog
{
// Construction
public:
	CFmiProjectionLineSetupDlg(NFmiProjectionCurvatureInfo* theProjInfo, CWnd* pParent = NULL);   // standard constructor
	virtual ~CFmiProjectionLineSetupDlg();

// Dialog Data
	//{{AFX_DATA(CFmiProjectionLineSetupDlg)
	enum { IDD = IDD_DIALOG_PROJECTION_LINE_SETTING_DLG };
	int		itsDrawingMode;
	int		itsLineDensityLat;
	int		itsLineDensityLon;
	int		itsLineSmoothness;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFmiProjectionLineSetupDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CFmiProjectionLineSetupDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnBnClickedButtonPRojectionLineColor();
	afx_msg void OnBnClickedButtonPRojectionLabelColor();

	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	void InitDialogTexts(void);

	NFmiProjectionCurvatureInfo* itsProjectionCurvatureInfo; // ei omista, ei tuhoa!!!

	// Seuraavaksi väri nappulaan liittyviä juttuja
	CButton itsProjectionLineColorButtom;
	COLORREF itsColorRefLine;
	CBitmap* itsColorBitmapLine;
	CRect itsColorRectLine;
	CButton itsProjectionLabelColorButtom;
	COLORREF itsColorRefLabel;
	CBitmap* itsColorBitmapLabel;
	CRect itsColorRectLabel;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

