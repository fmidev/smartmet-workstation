#pragma once

#include "ZoomView.h"

class SmartMetDocumentInterface;
class NFmiTrajectoryView;

/////////////////////////////////////////////////////////////////////////////
// CFmiTrajectoryView view
// Tämä hallitseen näytön klikkaukset ja päivitykset.
// Pitää sisällään luokat, jotka huolehtivat trajektorien asetuksista ja osittaisesta piirrosta.

class CFmiTrajectoryView :  public CZoomView // public CView
{
public:
	CFmiTrajectoryView(CWnd* theParent
				   , SmartMetDocumentInterface *smartMetDocumentInterface);

	void DoPrint(void);
	CBitmap* MemoryBitmap(void){return itsMemoryBitmap;}
	void Update(bool fMakeDirty);

protected:
	CFmiTrajectoryView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CFmiTrajectoryView)

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFmiTempView)
	public:
	virtual void OnInitialUpdate();
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CFmiTrajectoryView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
protected:
	//{{AFX_MSG(CFmiTempView)
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	NFmiTrajectoryView* itsView;
	CBitmap* itsMemoryBitmap;
	bool fViewDirty;
public:
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnSize(UINT nType, int cx, int cy);
};

/////////////////////////////////////////////////////////////////////////////

