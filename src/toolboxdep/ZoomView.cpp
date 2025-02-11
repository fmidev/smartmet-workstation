// ZoomView.cpp : implementation file
//

#include "stdafx.h"
#include "ZoomView.h"
#include "NFmiToolBox.h"
#include "NFmiZoomView.h"
#include "NFmiBitmapZoomView.h"
#include "ZoomDlg.h"
#include "NFmiDrawParam.h"
#include "SmartMetDocumentInterface.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CZoomView

IMPLEMENT_DYNCREATE(CZoomView, CView)

CZoomView::CZoomView()
:itsToolBox(0)
,itsZoomView(0)
,fMouseDragOn(FALSE)
,itsDrawParam(new NFmiDrawParam())
,itsSmartMetDocumentInterface(nullptr)
{
}

CZoomView::~CZoomView()
{
	delete itsToolBox;
	delete itsZoomView;
}


BEGIN_MESSAGE_MAP(CZoomView, CView)
	ON_WM_MOUSEACTIVATE()
	//{{AFX_MSG_MAP(CZoomView)
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDOWN()
	ON_WM_RBUTTONUP()
	//}}AFX_MSG_MAP
	ON_WM_ERASEBKGND()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CZoomView drawing

void CZoomView::OnDraw(CDC* pDC)
{
	itsToolBox->SetDC(pDC);
	if(itsZoomView)
		itsZoomView->Draw(itsToolBox);
}

/////////////////////////////////////////////////////////////////////////////
// CZoomView diagnostics

#ifdef _DEBUG
void CZoomView::AssertValid() const
{
	CView::AssertValid();
}

void CZoomView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CZoomView message handlers

void CZoomView::OnInitialUpdate()
{
	CView::OnInitialUpdate();

	// TODO: Add your specialized code here and/or call the base class
	if(itsToolBox)
		delete itsToolBox;
	itsToolBox = new NFmiToolBox(this);
	CreateZoomView();
}

CZoomView::CZoomView(CWnd * /* theParent */, SmartMetDocumentInterface *smartMetDocumentInterface)
:CView()
,itsToolBox(0)
,itsZoomView(0)
,fMouseDragOn(FALSE)
,itsDrawParam(new NFmiDrawParam())
,itsSmartMetDocumentInterface(smartMetDocumentInterface)
{

}

void CZoomView::CreateZoomView()
{
	CZoomDlg* parent = (CZoomDlg*) GetParent();
	if(parent)
	{
		if(itsZoomView)
			delete itsZoomView;

		itsZoomView = new NFmiBitmapZoomView(itsToolBox
									  ,itsDrawParam);
	}
}

// tämä täytyy tehdä doc-view -applikaatiossa, kun halutaan tehdä dialogi, jolla
// on view (tässä toolbox vaatii CView:sta perityn olion käyttämistä).
// Muuten mainframe haluaisi hiirtä klikatessa activoida tämän dialogissa
// olevan view:n, mikä ei voi toimia.
int CZoomView::OnMouseActivate(CWnd* pDesktopWnd, UINT nHitTest, UINT message)
{
	return 1;
}

boost::shared_ptr<NFmiArea> CZoomView::ZoomedArea(void)
{
	// tämä menee herran haltuun
	return ((NFmiZoomView*)itsZoomView)->ZoomedArea();
}

void CZoomView::OnLButtonDown(UINT nFlags, CPoint point)
{
	CDC *theDC = GetDC();
	if(!theDC)
		return;
	itsToolBox->SetDC(theDC);

	// tässä tapauksessa leftbuttondown palauttaa arvon, laitetaanko mouse drag päälle
	fMouseDragOn = itsZoomView ? itsZoomView->LeftButtonDown(itsToolBox->ToViewPoint(point.x, point.y)
		,itsToolBox->ConvertCtrlKey(nFlags)) : false;
	if(fMouseDragOn)
		SetCapture();
	ReleaseDC(theDC);
}

void CZoomView::OnLButtonUp(UINT nFlags, CPoint point)
{
	CDC *theDC = GetDC();
	if(!theDC)
		return;
	itsToolBox->SetDC(theDC);

	bool needsUpdate = itsZoomView ? itsZoomView->LeftButtonUp(itsToolBox->ToViewPoint(point.x, point.y)
		,itsToolBox->ConvertCtrlKey(nFlags)) : false;
	ReleaseDC(theDC);
	fMouseDragOn = FALSE;
	ReleaseCapture();
	if(needsUpdate)
	{
		Invalidate(FALSE);
	}
}

void CZoomView::OnMouseMove(UINT nFlags, CPoint point)
{
	if(fMouseDragOn)
	{
		CDC *theDC = GetDC();
		if(!theDC)
			return;
		itsToolBox->SetDC(theDC);

		if(itsZoomView)
			itsZoomView->MouseMove(itsToolBox->ToViewPoint(point.x, point.y)
							  ,itsToolBox->ConvertCtrlKey(nFlags));
		ReleaseDC(theDC);
	}
}

void CZoomView::Update(void)
{
	itsZoomView->Update();
}

void CZoomView::Update(int theSelectedMapViewIndex)
{
	if(itsZoomView)
	{
		CDC *theDC = GetDC();
		if(!theDC)
			return;
		itsToolBox->SetDC(theDC);
		itsZoomView->SetToolBox(itsToolBox);
		itsZoomView->MapViewDescTopIndex(theSelectedMapViewIndex);
		itsZoomView->Update();
		ReleaseDC(theDC);
	}
}

void CZoomView::OnRButtonUp(UINT nFlags, CPoint point)
{
	CDC *theDC = GetDC();
	if(!theDC)
		return;
	itsToolBox->SetDC(theDC);

	bool needsUpdate = itsZoomView ? itsZoomView->RightButtonUp(itsToolBox->ToViewPoint(point.x, point.y)
		,itsToolBox->ConvertCtrlKey(nFlags)) : false;
	ReleaseDC(theDC);
	fMouseDragOn = FALSE;
	ReleaseCapture();
	if(needsUpdate)
	{
		Invalidate(FALSE);
	}
}

BOOL CZoomView::OnEraseBkgnd(CDC* pDC)
{
	return FALSE;

//	return CView::OnEraseBkgnd(pDC);
}
