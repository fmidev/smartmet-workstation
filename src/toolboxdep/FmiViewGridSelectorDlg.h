#pragma once

// FmiViewGridSelectorDlg.h : header file
//

#include "SmartMetToolboxDep_resource.h"
#include "NFmiPoint.h"

class SmartMetDocumentInterface;
/////////////////////////////////////////////////////////////////////////////
// CFmiViewGridSelectorDlg dialog

class CFmiViewGridSelectorDlg : public CDialog
{
// Construction
public:
	CFmiViewGridSelectorDlg(int theMapViewDescTopIndex, SmartMetDocumentInterface *smartMetDocumentInterface, CWnd* pParent = NULL);   // standard constructor

	const NFmiPoint& SelectedGridSize(void) const {return itsSelectedGridSize;}
// Dialog Data
	//{{AFX_DATA(CFmiViewGridSelectorDlg)
	enum { IDD = IDD_DIALOG_GRID_VIEW_SELECTOR_DLG };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFmiViewGridSelectorDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CFmiViewGridSelectorDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	void InitDialogTexts(void);

	CView* itsViewGridSelectorView;
    SmartMetDocumentInterface *itsSmartMetDocumentInterface;
	int itsMapViewDescTopIndex;
	NFmiPoint itsSelectedGridSize;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

