#pragma once

// FmiCPLocationView.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CFmiCPLocationView view
#include "FmiFilterDataView.h"

class NFmiEditorControlPointManager;
class NFmiMovingCPLocationView;

class CFmiCPLocationView : public CFmiFilterDataView
{
protected:
	CFmiCPLocationView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CFmiCPLocationView)

// Attributes
public:

// Operations
	CFmiCPLocationView(CWnd* theParent, SmartMetDocumentInterface *smartMetDocumentInterface, int theIndex, NFmiCtrlView::ViewType theViewType, NFmiEditorControlPointManager* theCPManager);
	NFmiMovingCPLocationView* GetCPView(void);
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFmiCPLocationView)
	protected:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CFmiCPLocationView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
protected:
	afx_msg int OnMouseActivate(CWnd* pDesktopWnd, UINT nHitTest, UINT message);
	//{{AFX_MSG(CFmiCPLocationView)
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	virtual void CreateZoomView(void);

	NFmiEditorControlPointManager* itsCPManager; // ei omista
public:
    afx_msg void OnBnClickedButtonMacroParamSave();
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

