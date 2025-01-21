// CFmiTrajectoryView.cpp : implementation file
//

#include "stdafx.h"
#include "FmiTrajectoryView.h"
#include "NFmiToolBox.h"
#include "NFmiTrajectoryView.h"
#include "SmartMetDocumentInterface.h"
#include "FmiWin32Helpers.h"
#include "WaitCursorHelper.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CFmiTrajectoryView

IMPLEMENT_DYNCREATE(CFmiTrajectoryView, CView)

CFmiTrajectoryView::CFmiTrajectoryView(CWnd* theParent
										, SmartMetDocumentInterface *smartMetDocumentInterface)
:CZoomView(theParent, smartMetDocumentInterface)
,itsView(0)
,itsMemoryBitmap(0)
,fViewDirty(true)
{
}

CFmiTrajectoryView::CFmiTrajectoryView()
{
}

CFmiTrajectoryView::~CFmiTrajectoryView()
{
	delete itsView;
	if(itsMemoryBitmap)
	{
		itsMemoryBitmap->DeleteObject();
		delete itsMemoryBitmap;
	}
}

BEGIN_MESSAGE_MAP(CFmiTrajectoryView, CView)
	//{{AFX_MSG_MAP(CFmiTrajectoryView)
	ON_WM_ERASEBKGND()
	//}}AFX_MSG_MAP
	ON_WM_LBUTTONUP()
	ON_WM_RBUTTONUP()
	ON_WM_SIZE()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFmiTrajectoryView drawing

void CFmiTrajectoryView::OnDraw(CDC* pDC)
{
	CClientDC dc(this);
	CDC dcMem;
	dcMem.CreateCompatibleDC(&dc);
	CRect clientRect;
	GetClientRect(&clientRect);
	CBitmap *oldBitmap = 0;
	if(fViewDirty)
	{
		WaitCursorHelper waitCursorHelper(itsSmartMetDocumentInterface->ShowWaitCursorWhileDrawingView());
		if(itsMemoryBitmap)
			itsMemoryBitmap->DeleteObject();
		if(!itsMemoryBitmap)
			itsMemoryBitmap = new CBitmap;

		itsMemoryBitmap->CreateCompatibleBitmap(&dc, clientRect.Width(), clientRect.Height());
		ASSERT(itsMemoryBitmap->m_hObject != 0);

		oldBitmap = dcMem.SelectObject(itsMemoryBitmap);
		itsToolBox->SetDC(&dcMem);

		itsView->Draw(itsToolBox);
	}
	else
		oldBitmap = dcMem.SelectObject(itsMemoryBitmap);


	BOOL status = dc.BitBlt(0
							 ,0
							 ,clientRect.Width()
							 ,clientRect.Height()
							 ,&dcMem
						     ,0
						     ,0
						     ,SRCCOPY);

	itsMemoryBitmap = dcMem.SelectObject(oldBitmap);
	dcMem.DeleteDC();

	fViewDirty = false; // 'putsataan' ikkuna
}

/////////////////////////////////////////////////////////////////////////////
// CFmiTrajectoryView diagnostics

#ifdef _DEBUG
void CFmiTrajectoryView::AssertValid() const
{
	CView::AssertValid();
}

void CFmiTrajectoryView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CFmiTrajectoryView message handlers

void CFmiTrajectoryView::OnInitialUpdate()
{
	CView::OnInitialUpdate(); // älä kutsu lähintä emoa!!!

	itsToolBox = new NFmiToolBox(this); // tämä on lähimmässä emossa ja luotava tässä!!
	CDC *theDC = GetDC();
	if(!theDC)
		return;
	itsToolBox->SetDC(theDC);
	NFmiRect rect(0.,0.,1.,1.); // 0,0 - 1,1 rect I hope?
	itsView = new NFmiTrajectoryView(rect, itsToolBox);
	ReleaseDC(theDC);
}

void CFmiTrajectoryView::Update(bool fMakeDirty)
{
	fViewDirty = fMakeDirty;
	if(itsView)
	{
		CDC *theDC = GetDC();
		if(!theDC)
			return;
		itsToolBox->SetDC(theDC);
		itsView->Update();
		ReleaseDC(theDC);
	}
}

BOOL CFmiTrajectoryView::OnEraseBkgnd(CDC* pDC)
{
	return FALSE;

//	return CView::OnEraseBkgnd(pDC);
}

void CFmiTrajectoryView::OnLButtonUp(UINT nFlags, CPoint point)
{
	CDC *theDC = GetDC();
	if(!theDC)
		return;
	itsToolBox->SetDC(theDC);

	bool needsUpdate = itsView ? itsView->LeftButtonUp(itsToolBox->ToViewPoint(point.x, point.y)
		,itsToolBox->ConvertCtrlKey(nFlags)) : false;
	ReleaseDC(theDC);
	if(needsUpdate)
	{
		fViewDirty = true;
		Invalidate(FALSE);
	}
}


void CFmiTrajectoryView::OnRButtonUp(UINT nFlags, CPoint point)
{
	CDC *theDC = GetDC();
	if(!theDC)
		return;
	itsToolBox->SetDC(theDC);

	bool needsUpdate = itsView ? itsView->RightButtonUp(itsToolBox->ToViewPoint(point.x, point.y)
		,itsToolBox->ConvertCtrlKey(nFlags)) : false;
	ReleaseDC(theDC);
	if(needsUpdate)
	{
		fViewDirty = true;
		Invalidate(FALSE);
	}
}

void CFmiTrajectoryView::OnSize(UINT nType, int cx, int cy)
{
	CZoomView::OnSize(nType, cx, cy);

	fViewDirty = true;
	Invalidate(FALSE);
}
