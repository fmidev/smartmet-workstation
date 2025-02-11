#pragma once

// ZoomView.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CZoomView view
#include "boost/shared_ptr.hpp"

class NFmiToolBox;
class NFmiCtrlView;
class NFmiZoomView;
class NFmiArea;
class NFmiDrawParam;
class NFmiDibMapHandler;
class SmartMetDocumentInterface;

class CZoomView : public CView
{
protected:
	CZoomView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CZoomView)

// Attributes
public:

// Operations
public:
	 CZoomView(CWnd* theParent, SmartMetDocumentInterface *smartMetDocumentInterface);
	 boost::shared_ptr<NFmiArea> ZoomedArea(void); // suunta: view -> doc
	 virtual void Update(void);
	 void Update(int theSelectedMapViewIndex);
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CZoomView)
	public:
	virtual void OnInitialUpdate();
	protected:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CZoomView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
protected:
	afx_msg int OnMouseActivate(CWnd* pDesktopWnd, UINT nHitTest, UINT message);
	//{{AFX_MSG(CZoomView)
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
protected:
	bool fMouseDragOn;
	virtual void CreateZoomView(void);
	NFmiCtrlView* itsZoomView;
	NFmiToolBox* itsToolBox;
	boost::shared_ptr<NFmiDrawParam> itsDrawParam;
    SmartMetDocumentInterface *itsSmartMetDocumentInterface;
public:
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

