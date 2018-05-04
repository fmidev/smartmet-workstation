
// WorkerProcessTcpMFCView.h : interface of the CWorkerProcessTcpMFCView class
//

#pragma once


class CWorkerProcessTcpMFCView : public CView
{
protected: // create from serialization only
	CWorkerProcessTcpMFCView();
	DECLARE_DYNCREATE(CWorkerProcessTcpMFCView)

// Attributes
public:
	CWorkerProcessTcpMFCDoc* GetDocument() const;

// Operations
public:

// Overrides
public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:

// Implementation
public:
	virtual ~CWorkerProcessTcpMFCView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // debug version in WorkerProcessTcpMFCView.cpp
inline CWorkerProcessTcpMFCDoc* CWorkerProcessTcpMFCView::GetDocument() const
   { return reinterpret_cast<CWorkerProcessTcpMFCDoc*>(m_pDocument); }
#endif

