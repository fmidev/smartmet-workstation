
// WorkerProcessMFCView.h : interface of the CWorkerProcessMFCView class
//

#pragma once


class CWorkerProcessMFCView : public CView
{
protected: // create from serialization only
	CWorkerProcessMFCView();
	DECLARE_DYNCREATE(CWorkerProcessMFCView)

// Attributes
public:
	CWorkerProcessMFCDoc* GetDocument() const;

// Operations
public:

// Overrides
public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:

// Implementation
public:
	virtual ~CWorkerProcessMFCView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // debug version in WorkerProcessMFCView.cpp
inline CWorkerProcessMFCDoc* CWorkerProcessMFCView::GetDocument() const
   { return reinterpret_cast<CWorkerProcessMFCDoc*>(m_pDocument); }
#endif

