#pragma once

// FmiFilterDataView.h : header file
//

#include "ZoomView.h"
#include "NFmiCtrlView.h"

/////////////////////////////////////////////////////////////////////////////
// CFmiFilterDataView view

class CFmiFilterDataView : public CZoomView
{
protected:
	CFmiFilterDataView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CFmiFilterDataView)

// Attributes
public:

// Operations
	CFmiFilterDataView(int theMapViewDescTopIndex, CWnd* theParent, SmartMetDocumentInterface *smartMetDocumentInterface, int theIndex, NFmiCtrlView::ViewType theViewType);
	virtual void Update(void);
	NFmiCtrlView* ZoomView(void){return itsZoomView;}; // ruma viritys, mutta tarvitsen zoomview-pointerin näytön ruudukon asetus dialogin käytön jälkeen
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFmiFilterDataView)
	public:
	virtual void OnInitialUpdate();
	protected:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CFmiFilterDataView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
protected:
	afx_msg int OnMouseActivate(CWnd* pDesktopWnd, UINT nHitTest, UINT message);
	//{{AFX_MSG(CFmiFilterDataView)
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	virtual void CreateZoomView(void);

	CBitmap* itsMemoryBitmap;
	int itsMapViewDescTopIndex;
	int itsIndex; // muokkausdialogissa on useita tälläisiä näyttöjä, tämä erottaa ne toisistaan
	NFmiCtrlView::ViewType itsViewType;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

