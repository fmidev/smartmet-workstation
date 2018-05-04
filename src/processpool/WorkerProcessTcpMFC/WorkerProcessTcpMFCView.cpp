
// WorkerProcessTcpMFCView.cpp : implementation of the CWorkerProcessTcpMFCView class
//

#include "stdafx.h"
// SHARED_HANDLERS can be defined in an ATL project implementing preview, thumbnail
// and search filter handlers and allows sharing of document code with that project.
#ifndef SHARED_HANDLERS
#include "WorkerProcessTcpMFC.h"
#endif

#include "WorkerProcessTcpMFCDoc.h"
#include "WorkerProcessTcpMFCView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CWorkerProcessTcpMFCView

IMPLEMENT_DYNCREATE(CWorkerProcessTcpMFCView, CView)

BEGIN_MESSAGE_MAP(CWorkerProcessTcpMFCView, CView)
END_MESSAGE_MAP()

// CWorkerProcessTcpMFCView construction/destruction

CWorkerProcessTcpMFCView::CWorkerProcessTcpMFCView()
{
	// TODO: add construction code here

}

CWorkerProcessTcpMFCView::~CWorkerProcessTcpMFCView()
{
}

BOOL CWorkerProcessTcpMFCView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CView::PreCreateWindow(cs);
}

// CWorkerProcessTcpMFCView drawing

void CWorkerProcessTcpMFCView::OnDraw(CDC* /*pDC*/)
{
	CWorkerProcessTcpMFCDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	// TODO: add draw code for native data here
}


// CWorkerProcessTcpMFCView diagnostics

#ifdef _DEBUG
void CWorkerProcessTcpMFCView::AssertValid() const
{
	CView::AssertValid();
}

void CWorkerProcessTcpMFCView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CWorkerProcessTcpMFCDoc* CWorkerProcessTcpMFCView::GetDocument() const // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CWorkerProcessTcpMFCDoc)));
	return (CWorkerProcessTcpMFCDoc*)m_pDocument;
}
#endif //_DEBUG


// CWorkerProcessTcpMFCView message handlers
