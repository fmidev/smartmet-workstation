
// WorkerProcessMFCView.cpp : implementation of the CWorkerProcessMFCView class
//

#include "stdafx.h"
// SHARED_HANDLERS can be defined in an ATL project implementing preview, thumbnail
// and search filter handlers and allows sharing of document code with that project.
#ifndef SHARED_HANDLERS
#include "WorkerProcessMFC.h"
#endif

#include "WorkerProcessMFCDoc.h"
#include "WorkerProcessMFCView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CWorkerProcessMFCView

IMPLEMENT_DYNCREATE(CWorkerProcessMFCView, CView)

BEGIN_MESSAGE_MAP(CWorkerProcessMFCView, CView)
END_MESSAGE_MAP()

// CWorkerProcessMFCView construction/destruction

CWorkerProcessMFCView::CWorkerProcessMFCView()
{
	// TODO: add construction code here

}

CWorkerProcessMFCView::~CWorkerProcessMFCView()
{
}

BOOL CWorkerProcessMFCView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CView::PreCreateWindow(cs);
}

// CWorkerProcessMFCView drawing

void CWorkerProcessMFCView::OnDraw(CDC* /*pDC*/)
{
	CWorkerProcessMFCDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	// TODO: add draw code for native data here
}


// CWorkerProcessMFCView diagnostics

#ifdef _DEBUG
void CWorkerProcessMFCView::AssertValid() const
{
	CView::AssertValid();
}

void CWorkerProcessMFCView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CWorkerProcessMFCDoc* CWorkerProcessMFCView::GetDocument() const // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CWorkerProcessMFCDoc)));
	return (CWorkerProcessMFCDoc*)m_pDocument;
}
#endif //_DEBUG


// CWorkerProcessMFCView message handlers
