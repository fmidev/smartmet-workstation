#pragma once

// ZoomDlg.h : header file
//

#include "SmartMetToolboxDep_resource.h"
#include "NFmiViewPosRegistryInfo.h"

/////////////////////////////////////////////////////////////////////////////
// CZoomDlg dialog

class SmartMetDocumentInterface;

class CZoomDlg : public CDialog
{
private: // näillä talletetaan sijainti ja koko rekisteriin
    static const NFmiViewPosRegistryInfo s_ViewPosRegistryInfo;
public:
    static const NFmiViewPosRegistryInfo& ViewPosRegistryInfo(void){return s_ViewPosRegistryInfo;}

    // Construction
public:
	 ~CZoomDlg();
	CZoomDlg(SmartMetDocumentInterface *smartMetDocumentInterface, CWnd* pParent = NULL); // standard constructor

	void Update(void);
	void SetDefaultValues(void);
	void ActivateMapViewDescTop(int theDescTopIndex);
    static std::string MakeUsedWinRegistryKeyStr(unsigned int /* theMapViewDescTopIndex */) {return ViewPosRegistryInfo().WinRegistryKeyStr();}

// Dialog Data
	//{{AFX_DATA(CZoomDlg)
	enum { IDD = IDD_DIALOG_ZOOM_MAP };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CZoomDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CZoomDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	virtual void OnOK();
	afx_msg void OnShowZoomedArea();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	void Zoom(void);
	CRect CalcMapArea(void);
	void MoveMapArea(void);
	void InitDialogTexts(void);

	CView* itsClientView;
    SmartMetDocumentInterface *itsSmartMetDocumentInterface;
	int itsSelectedMapViewDescTopIndex;
public:
	afx_msg void OnBnClickedRadioZoomMapView1();
	afx_msg void OnBnClickedRadioZoomMapView2();
	afx_msg void OnBnClickedRadioZoomMapView3();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

