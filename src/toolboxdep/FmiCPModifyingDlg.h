#pragma once

// FmiCPModifyingDlg.h : header file
//
#include "SmartMetToolboxDep_resource.h"

/////////////////////////////////////////////////////////////////////////////
// CFmiCPModifyingDlg dialog
class NFmiEditorControlPointManager;
class NFmiPoint;
class NFmiGrid;

class CFmiCPModifyingDlg : public CDialog
{
// Construction
public:
	CFmiCPModifyingDlg(NFmiEditorControlPointManager* theCPManager, const NFmiPoint& theLaLonPoint, const NFmiGrid* theDataGrid, CWnd* pParent = NULL);   // standard constructor
	~CFmiCPModifyingDlg(void);

// Dialog Data
	//{{AFX_DATA(CFmiCPModifyingDlg)
	enum { IDD = IDD_DIALOG_CPMODIFYING_DLG };
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFmiCPModifyingDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	void EnableLocationEditView(bool newState);
	void CreateMovingCPLocationView(void); // kutsutaan vain yhden kerran!!!!
	void InitDialogTexts(void);

	// Generated message map functions
	//{{AFX_MSG(CFmiCPModifyingDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnCheckChangeInTime();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	NFmiEditorControlPointManager* itsCPManager; // ei omista
	const NFmiGrid* itsDataGrid;  // ei omista
	NFmiPoint* itsCurrentLaLonPoint; // omistaa

	CView* itsMovingCPLocationView; // ajassa liikkuvan CP:n kontrolli-ikkuna
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

