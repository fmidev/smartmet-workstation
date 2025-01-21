// FmiCPLocationView.cpp : implementation file
//

#include "stdafx.h"
#include "FmiCPLocationView.h"
#include "NFmiCtrlView.h"
#include "NFmiMovingCPLocationView.h"
#include "NFmiToolBox.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CFmiCPLocationView

IMPLEMENT_DYNCREATE(CFmiCPLocationView, CView)

CFmiCPLocationView::CFmiCPLocationView()
:CFmiFilterDataView()
,itsCPManager(0)
{
}

CFmiCPLocationView::CFmiCPLocationView(CWnd* theParent, SmartMetDocumentInterface *smartMetDocumentInterface, int theIndex, NFmiCtrlView::ViewType theViewType, NFmiEditorControlPointManager* theCPManager)
:CFmiFilterDataView(0, theParent, smartMetDocumentInterface, theIndex, theViewType)
,itsCPManager(theCPManager)
{
}

CFmiCPLocationView::~CFmiCPLocationView()
{
}


BEGIN_MESSAGE_MAP(CFmiCPLocationView, CView)
	ON_WM_MOUSEACTIVATE()
	//{{AFX_MSG_MAP(CFmiCPLocationView)
	ON_WM_LBUTTONUP()
	ON_WM_RBUTTONUP()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFmiCPLocationView drawing

void CFmiCPLocationView::OnDraw(CDC* pDC)
{
	CFmiFilterDataView::OnDraw(pDC);
}

/////////////////////////////////////////////////////////////////////////////
// CFmiCPLocationView diagnostics

#ifdef _DEBUG
void CFmiCPLocationView::AssertValid() const
{
	CView::AssertValid();
}

void CFmiCPLocationView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CFmiCPLocationView message handlers

void CFmiCPLocationView::CreateZoomView(void)
{
	if(itsZoomView)
	{
		delete itsZoomView;
		itsZoomView = 0;
	}
	NFmiRect rect(0,0,1,1);

	if(itsViewType == NFmiCtrlView::kMovingCPLocationView)
	{
			itsZoomView = new NFmiMovingCPLocationView(itsToolBox
														,itsDrawParam
														,rect
														,itsIndex
														,itsCPManager);
	}
	else
		CFmiFilterDataView::CreateZoomView();
}

// tämä täytyy tehdä doc-view -applikaatiossa, kun halutaan tehdä dialogi, jolla
// on view (tässä toolbox vaatii CView:sta perityn olion käyttämistä).
// Muuten mainframe haluaisi hiirtä klikatessa activoida tämän dialogissa
// olevan view:n, mikä ei voi toimia.
int CFmiCPLocationView::OnMouseActivate(CWnd* pDesktopWnd, UINT nHitTest, UINT message)
{
	return 1;
}

void CFmiCPLocationView::OnLButtonUp(UINT nFlags, CPoint point)
{
	CDC *theDC = GetDC();
	if(!theDC)
		return;
	itsToolBox->SetDC(theDC);

	bool needsUpdate = itsZoomView ? itsZoomView->LeftButtonUp(itsToolBox->ToViewPoint(point.x, point.y)
		,itsToolBox->ConvertCtrlKey(nFlags)) : false;

	ReleaseDC(theDC);
	if(needsUpdate)
		Invalidate(FALSE);
}

void CFmiCPLocationView::OnRButtonUp(UINT nFlags, CPoint point)
{
	CDC *theDC = GetDC();
	if(!theDC)
		return;
	itsToolBox->SetDC(theDC);

	bool needsUpdate = itsZoomView ? itsZoomView->RightButtonUp(itsToolBox->ToViewPoint(point.x, point.y)
		,itsToolBox->ConvertCtrlKey(nFlags)) : false;
	ReleaseDC(theDC);
	if(needsUpdate)
		Invalidate(FALSE);
}

NFmiMovingCPLocationView* CFmiCPLocationView::GetCPView(void)
{
	return (NFmiMovingCPLocationView*)itsZoomView;
}
