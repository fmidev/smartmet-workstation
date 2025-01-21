// FmiFilterDataView.cpp : implementation file
//

#include "stdafx.h"
#include "FmiFilterDataView.h"
#include "NFmiZoomView.h"
#include "NFmiFilterGridView.h"
#include "NFmiToolBox.h"
#include "NFmiTimeFilterView.h"
#include "FmiFilterDataDlg.h"
#include "NFmiGridViewSelectorView.h"
#include "NFmiTimeControlView.h"
#include "SmartMetDocumentInterface.h"
#include "NFmiModelCombineStatusView.h"
#include "NFmiModelProducerIndexView.h"
#include "NFmiModelProducerIndexViewWCTR.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CFmiFilterDataView

IMPLEMENT_DYNCREATE(CFmiFilterDataView, CView)

CFmiFilterDataView::CFmiFilterDataView()
:CZoomView()
,itsMapViewDescTopIndex(0)
,itsIndex(0)
,itsViewType(NFmiCtrlView::kAreaFilterView)
,itsMemoryBitmap(new CBitmap())
{
}

CFmiFilterDataView::CFmiFilterDataView(int theMapViewDescTopIndex, CWnd* theParent
									  , SmartMetDocumentInterface *smartMetDocumentInterface
									  ,int theIndex
									  ,NFmiCtrlView::ViewType theViewType)
:CZoomView(theParent, smartMetDocumentInterface)
,itsMapViewDescTopIndex(theMapViewDescTopIndex)
,itsIndex(theIndex)
,itsViewType(theViewType)
,itsMemoryBitmap(new CBitmap())
{
}

CFmiFilterDataView::~CFmiFilterDataView()
{
	if(itsMemoryBitmap)
	{
		itsMemoryBitmap->DeleteObject();
		delete itsMemoryBitmap;
	}
}


BEGIN_MESSAGE_MAP(CFmiFilterDataView, CView)
	ON_WM_MOUSEACTIVATE()
	//{{AFX_MSG_MAP(CFmiFilterDataView)
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_RBUTTONUP()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONDBLCLK()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFmiFilterDataView drawing

void CFmiFilterDataView::OnDraw(CDC* pDC)
{
	if(itsZoomView && itsToolBox)
	{
		CClientDC dc(this);
		CDC dcMem;
		dcMem.CreateCompatibleDC(&dc);
		CRect clientArea;
		GetClientRect(&clientArea);

		if(itsMemoryBitmap)
			itsMemoryBitmap->DeleteObject();
		itsMemoryBitmap->CreateCompatibleBitmap(&dc,clientArea.Width()
												,clientArea.Height());
		ASSERT(itsMemoryBitmap->m_hObject != 0);
		CBitmap *oldBitmap = dcMem.SelectObject(itsMemoryBitmap);

		itsToolBox->SetDC(&dcMem);
		itsZoomView->Draw(itsToolBox);

		BOOL status = pDC->BitBlt(0
								 ,0
								 ,clientArea.Width()
								 ,clientArea.Height()
								 ,&dcMem
								 ,0
								 ,0
								 ,SRCCOPY);

		itsMemoryBitmap = dcMem.SelectObject(oldBitmap);
		dcMem.DeleteDC();
	}
}

/////////////////////////////////////////////////////////////////////////////
// CFmiFilterDataView diagnostics

#ifdef _DEBUG
void CFmiFilterDataView::AssertValid() const
{
	CView::AssertValid();
}

void CFmiFilterDataView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CFmiFilterDataView message handlers

void CFmiFilterDataView::OnLButtonDown(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default

	CZoomView::OnLButtonDown(nFlags, point);
	Invalidate(FALSE);
}

void CFmiFilterDataView::OnLButtonUp(UINT nFlags, CPoint point)
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
	CFmiFilterDataDlg* parent = (CFmiFilterDataDlg*) GetParent();
	if(parent && itsViewType < NFmiCtrlView::kGridViewSelectorView) // jos == kGridViewSelectorView, ei tarvitse päivittää isäntä-dialogia
	{
		parent->Update();
	}
	else if(parent && itsViewType == NFmiCtrlView::kTimeControlView)
	{
		parent->RefreshApplicationViews(__FUNCTION__, false);
	}
}

void CFmiFilterDataView::OnMouseMove(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default

	CZoomView::OnMouseMove(nFlags, point);
	Invalidate(FALSE);
}

void CFmiFilterDataView::OnRButtonUp(UINT nFlags, CPoint point)
{
//	CZoomView::OnLButtonUp(nFlags, point);
	CDC *theDC = GetDC();
	if(!theDC)
		return;
	itsToolBox->SetDC(theDC);

	bool needsUpdate = itsZoomView ? itsZoomView->RightButtonUp(itsToolBox->ToViewPoint(point.x, point.y)
		,itsToolBox->ConvertCtrlKey(nFlags)) : false;
	ReleaseDC(theDC);
	fMouseDragOn = FALSE;
	ReleaseCapture();
	CFmiFilterDataDlg* parent = (CFmiFilterDataDlg*) GetParent();
	if(parent && itsViewType < NFmiCtrlView::kGridViewSelectorView) // jos == kGridViewSelectorView, ei tarvitse päivittää isäntä-dialogia
	{
		parent->Update();
	}
	else if(parent && itsViewType == NFmiCtrlView::kTimeControlView)
	{
		parent->RefreshApplicationViews(__FUNCTION__, false);
	}
}

void CFmiFilterDataView::CreateZoomView(void)
{
	if(itsZoomView)
		delete itsZoomView;
	NFmiRect rect(0,0,1,1);
    auto defaultEditedDrawParam = itsSmartMetDocumentInterface->DefaultEditedDrawParam();

	switch(itsViewType)
	{
		case NFmiCtrlView::kAreaFilterView:
			itsZoomView = new NFmiFilterGridView(itsToolBox
												,itsDrawParam
												,rect
												,itsIndex);
			break;
		case NFmiCtrlView::kTimeFilterView:
			itsZoomView = new NFmiTimeFilterView(itsToolBox
												,itsDrawParam
												,rect
												,itsIndex);
			break;
		case NFmiCtrlView::kGridViewSelectorView:
			itsZoomView = new NFmiGridViewSelectorView(itsMapViewDescTopIndex, itsToolBox
													,itsDrawParam
													,rect
													,5
													,10);
			break;
		case NFmiCtrlView::kTimeControlView:
			itsZoomView = new NFmiTimeControlView(itsMapViewDescTopIndex, rect
												 ,itsToolBox
												 ,defaultEditedDrawParam ? defaultEditedDrawParam : itsDrawParam
												 ,false
												 ,false
												 ,true
												 ,true
												 ,true);
			static_cast<NFmiTimeControlView*>(itsZoomView)->Initialize(true, false);
			break;
		case NFmiCtrlView::kModelCombineStatusView:
			itsZoomView = new NFmiModelCombineStatusView(itsToolBox
														 ,defaultEditedDrawParam ? defaultEditedDrawParam : itsDrawParam
														 ,rect);
			break;
		case NFmiCtrlView::kMovingCPLocationView:
			itsZoomView = new NFmiModelProducerIndexView(itsToolBox
														 ,defaultEditedDrawParam ? defaultEditedDrawParam : itsDrawParam
														 ,rect);
			break;
		case NFmiCtrlView::kModelProducerIndexViewWCTR:
			itsZoomView = new NFmiModelProducerIndexViewWCTR(itsToolBox
														    ,defaultEditedDrawParam ? defaultEditedDrawParam : itsDrawParam
														    ,rect);
			break;
		default:
			itsZoomView = new NFmiTimeControlView(itsMapViewDescTopIndex, rect
												 ,itsToolBox
												 ,defaultEditedDrawParam ? defaultEditedDrawParam : itsDrawParam
												 ,false
												 ,false
												 ,true
												 ,true
												 ,true);
			break;
	}
}

void CFmiFilterDataView::Update(void)
{
	if(itsZoomView)
		itsZoomView->Update();
	Invalidate(FALSE);
}

void CFmiFilterDataView::OnInitialUpdate()
{
	CZoomView::OnInitialUpdate();
}

// tämä täytyy tehdä doc-view -applikaatiossa, kun halutaan tehdä dialogi, jolla
// on view (tässä toolbox vaatii CView:sta perityn olion käyttämistä).
// Muuten mainframe haluaisi hiirtä klikatessa activoida tämän dialogissa
// olevan view:n, mikä ei voi toimia.
int CFmiFilterDataView::OnMouseActivate(CWnd* pDesktopWnd, UINT nHitTest, UINT message)
{
	return 1;
}

void CFmiFilterDataView::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	CDC *theDC = GetDC();
	if(!theDC)
		return;
	itsToolBox->SetDC(theDC);

	bool needsUpdate = itsZoomView ? itsZoomView->LeftDoubleClick(itsToolBox->ToViewPoint(point.x, point.y)
		,itsToolBox->ConvertCtrlKey(nFlags)) : false;
	ReleaseDC(theDC);
	fMouseDragOn = FALSE;
	ReleaseCapture();

    //	CView::OnLButtonDblClk(nFlags, point);
}
