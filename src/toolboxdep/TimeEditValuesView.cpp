// TimeEditValuesView.cpp : implementation file
//

#include "stdafx.h"
#include "TimeEditValuesView.h"
#include "NFmiToolBox.h"
#include "SmartMetDocumentInterface.h"
#include "NFmiTimeValueEditManagerView.h"
#include "CFmiMenu.h"
#include "NFmiMenuItemList.h"
#include "NFmiMetEditorOptionsData.h"
#include <cassert>
#include "NFmiMapViewDescTop.h"
#include "FmiWin32Helpers.h"
#include "FmiWin32TemplateHelpers.h"
#include "ApplicationInterface.h"
#include "SpecialDesctopIndex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
// CTimeEditValuesView

IMPLEMENT_DYNCREATE(CTimeEditValuesView, CView)

CTimeEditValuesView::CTimeEditValuesView()
:CZoomView()
,itsDrawParam()
,itsManagerView(0)
,itsMemoryBitmap(0)
{
	itsSmartMetDocumentInterface = 0;
}

CTimeEditValuesView::CTimeEditValuesView(CWnd* theParent
										,boost::shared_ptr<NFmiDrawParam> &theDrawParam
										, SmartMetDocumentInterface *smartMetDocumentInterface)
:CZoomView(theParent, smartMetDocumentInterface)
,itsDrawParam(theDrawParam)
,itsManagerView(0)
,itsMemoryBitmap(0)
{
}

CTimeEditValuesView::~CTimeEditValuesView()
{
	delete itsManagerView;
	if(itsMemoryBitmap)
	{
		itsMemoryBitmap->DeleteObject();
		delete itsMemoryBitmap;
	}
}


BEGIN_MESSAGE_MAP(CTimeEditValuesView, CView)
	//{{AFX_MSG_MAP(CTimeEditValuesView)
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_RBUTTONUP()
	ON_WM_SIZE()
	ON_WM_LBUTTONDOWN()
	ON_WM_CHAR()
	ON_WM_KEYUP()
	ON_WM_KEYDOWN()
	ON_WM_GETDLGCODE()
	ON_WM_ERASEBKGND()
	//}}AFX_MSG_MAP
	ON_WM_MOUSEWHEEL()
	ON_NOTIFY (UDM_TOOLTIP_DISPLAY, NULL, NotifyDisplayTooltip)
	ON_WM_MBUTTONUP()
	ON_WM_LBUTTONDBLCLK()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTimeEditValuesView drawing

void CTimeEditValuesView::OnDraw(CDC* pDC)
{
	if(!itsToolBox)
		return;

	CClientDC dc(this);
	CDC dcMem;
	dcMem.CreateCompatibleDC(&dc);
	CRect clientArea;
	GetClientRect(&clientArea);
	CBitmap *oldBitmap = 0;
	if(itsSmartMetDocumentInterface->TimeSerialViewDirty())
	{

		if(itsMemoryBitmap)
			itsMemoryBitmap->DeleteObject();
		else
			itsMemoryBitmap = new CBitmap;

		itsMemoryBitmap->CreateCompatibleBitmap(&dc, clientArea.Width(), clientArea.Height());
		ASSERT(itsMemoryBitmap->m_hObject != 0);

		oldBitmap = dcMem.SelectObject(itsMemoryBitmap);
		itsToolBox->SetDC(&dcMem);

		if(itsManagerView)
		{
			// Laitetaan tiimalasi piirron ajaksi tiedottamaan käyttäjälle, että piirto voi kestää kauann
			WaitCursorHelper waitCursorHelper(itsSmartMetDocumentInterface->ShowWaitCursorWhileDrawingView());

			DoDraw();
		}
        itsSmartMetDocumentInterface->TimeSerialViewDirty(false);
	}
	else
		oldBitmap = dcMem.SelectObject(itsMemoryBitmap);

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


void CTimeEditValuesView::DoDraw(void)
{
    try
    {
        itsManagerView->Draw(itsToolBox);
    }
    catch(std::exception& e)
    {
        std::string errorTitleStr("Error with time-serial-view drawing");
        std::string errorStr("Error while drawing sounding-view, there won't be more reports of these:\n");
        errorStr += e.what();
        itsSmartMetDocumentInterface->LogAndWarnUser(errorStr, errorTitleStr, CatLog::Severity::Error, CatLog::Category::Visualization, true);
    }
    catch(...)
    {
        std::string errorTitleStr("Unknown error with time-serial-view drawing");
        std::string errorStr("Unknown error while drawing sounding-view, there won't be more reports of these.");
        itsSmartMetDocumentInterface->LogAndWarnUser(errorStr, errorTitleStr, CatLog::Severity::Error, CatLog::Category::Visualization, true);
    }
}

void CTimeEditValuesView::DrawOverBitmapThings(NFmiToolBox * /* theToolBox */ )
{
}

int CTimeEditValuesView::MapViewDescTopIndex(void) 
{
    return CtrlViewUtils::kFmiTimeSerialView; 
}

/////////////////////////////////////////////////////////////////////////////
// CTimeEditValuesView diagnostics

#ifdef _DEBUG
void CTimeEditValuesView::AssertValid() const
{
	CView::AssertValid();
}

void CTimeEditValuesView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CTimeEditValuesView message handlers

static const int TIMEVIEW_TOOLTIP_ID = 1234569;

void CTimeEditValuesView::OnInitialUpdate()
{
	CView::OnInitialUpdate();

	itsToolBox = new NFmiToolBox(this);
	NFmiRect rect(0.,0.,1.,1.); // 0,0 - 1,1 rect I hope?

	if(itsSmartMetDocumentInterface)
		itsManagerView = new NFmiTimeValueEditManagerView(rect
														 ,itsToolBox
														 ,&itsDrawingEnvironment);
	CFmiWin32Helpers::InitializeCPPTooltip(this, m_tooltip, TIMEVIEW_TOOLTIP_ID, 500);
	CRect winRec;
	GetWindowRect(winRec);
	OnSize(SW_RESTORE, winRec.Width(), winRec.Height()); // jostain syystä on pakko tehdä onsize, että tooltip toimii varmasti koko ikkunan alueella?!?!?
}

void CTimeEditValuesView::SetToolsDCs(CDC* theDC)
{
	if(itsToolBox)
		itsToolBox->SetDC(theDC);
}

void CTimeEditValuesView::OnLButtonUp(UINT nFlags, CPoint point)
{
	try
	{
		CtrlView::DeviceContextHandler<CTimeEditValuesView> deviceContextHandler(this);
		bool needsUpdate = itsManagerView ? itsManagerView->LeftButtonUp(itsToolBox->ToViewPoint(point.x, point.y)
			,itsToolBox->ConvertCtrlKey(nFlags)) : false;

		itsManagerView->MapViewDescTopIndex(CtrlViewUtils::kFmiTimeSerialView);

		if(needsUpdate)
		{
			Invalidate(FALSE);
			if(itsSmartMetDocumentInterface->TimeSerialViewDirty())
			{
				MakeAllMapViewsUpdated(itsManagerView->TimeScaleUpdated()); // tämä on pika viritys optimointi, että joka klikkauksella aikasarjaikkunassa ei piirrettäisi karttanäyttöä
				itsManagerView->TimeScaleUpdated(false);
                itsSmartMetDocumentInterface->RefreshApplicationViewsAndDialogs("TimeSerialView: Left mouse button up action", SmartMetViewId::AllMapViews | SmartMetViewId::TimeSerialView);
			}

			if(itsSmartMetDocumentInterface->MetEditorOptionsData().ControlPointMode())
                itsSmartMetDocumentInterface->DrawOverBitmapThings(0);
		}
		else
		{
		//	CView::OnLButtonUp(nFlags, point); // tarvitaaanko????
		}
	}
	catch(...)
	{
		ReleaseCapture(); // vapautetaan lopuksi hiiren viestit muidenkin ikkunoiden käyttöön
		throw ; // laitetaan poikkeus eteenpäin
	}
	ReleaseCapture(); // vapautetaan lopuksi hiiren viestit muidenkin ikkunoiden käyttöön (OnLButtonDown:issa laitettiin SetCapture päälle)
}

void CTimeEditValuesView::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	CtrlView::DeviceContextHandler<CTimeEditValuesView> deviceContextHandler(this);
	bool needsUpdate = itsManagerView ? itsManagerView->LeftDoubleClick(itsToolBox->ToViewPoint(point.x, point.y)
		, itsToolBox->ConvertCtrlKey(nFlags)) : false;
	if (needsUpdate)
	{
		if (itsSmartMetDocumentInterface->ActivateParamSelectionDlgAfterLeftDoubleClick())
		{
			itsSmartMetDocumentInterface->ActivateParamSelectionDlgAfterLeftDoubleClick(false);
			itsSmartMetDocumentInterface->ActivateParameterSelectionDlg(itsManagerView->MapViewDescTopIndex());
			return;
		}
		Invalidate(FALSE);
		itsSmartMetDocumentInterface->RefreshApplicationViewsAndDialogs(__FUNCTION__, SmartMetViewId::AllMapViews | SmartMetViewId::TimeSerialView);
	}
}

void CTimeEditValuesView::OnMButtonUp(UINT nFlags, CPoint point)
{
	CtrlView::DeviceContextHandler<CTimeEditValuesView> deviceContextHandler(this);
	bool needsUpdate = itsManagerView ? itsManagerView->MiddleButtonUp(itsToolBox->ToViewPoint(point.x, point.y)
		, itsToolBox->ConvertCtrlKey(nFlags)) : false;
	if(needsUpdate)
	{
		Invalidate(FALSE);
		if(itsSmartMetDocumentInterface->TimeSerialViewDirty())
			itsSmartMetDocumentInterface->RefreshApplicationViewsAndDialogs("TimeSerialView: Middle mouse button up action", SmartMetViewId::AllMapViews | SmartMetViewId::TimeSerialView);
	}
	else
	{
		//	CZoomView::OnMButtonUp(nFlags, point);
	}
}

void CTimeEditValuesView::OnMouseMove(UINT nFlags, CPoint point)
{
    if(itsSmartMetDocumentInterface->Printing())
        return;
    if(!itsSmartMetDocumentInterface->MouseCapturedInTimeWindow())
		return;
	CtrlView::DeviceContextHandler<CTimeEditValuesView> deviceContextHandler(this);
	bool needsUpdate = itsManagerView ? itsManagerView->MouseMove(itsToolBox->ToViewPoint(point.x, point.y)
		,itsToolBox->ConvertCtrlKey(nFlags)) : false;
	if(needsUpdate)
	{
		Invalidate(FALSE);
		if(itsSmartMetDocumentInterface->TimeSerialViewDirty())
		{
			Update(); // Tässä oli ennen RefreshApplicationViewsAndDialogs-kutsu, mutta ei ole minusta syytä päivittää muita näyttöjä MouseMove:a tehtäessä. 
		}
		else if(itsSmartMetDocumentInterface->MetEditorOptionsData().ControlPointMode())
            itsSmartMetDocumentInterface->DrawOverBitmapThings(0);
	}
}

void CTimeEditValuesView::OnLButtonDown(UINT nFlags, CPoint point)
{
	CtrlView::DeviceContextHandler<CTimeEditValuesView> deviceContextHandler(this);
	bool needsUpdate = itsManagerView ? itsManagerView->LeftButtonDown(itsToolBox->ToViewPoint(point.x, point.y)
		,itsToolBox->ConvertCtrlKey(nFlags)) : false;
	if(needsUpdate)
	{
		Invalidate(FALSE);
	}
	else
	{
//	CView::OnLButtonUp(nFlags, point); // tarvitaaanko????
	}
	SetCapture(); // otetaan hiiren liikkeet/viestit talteeen toistaiseksi tähän ikkunaan
}

void CTimeEditValuesView::OnRButtonUp(UINT nFlags, CPoint point)
{
	CtrlView::DeviceContextHandler<CTimeEditValuesView> deviceContextHandler(this);
	bool needsUpdate = itsManagerView ? itsManagerView->RightButtonUp(itsToolBox->ToViewPoint(point.x, point.y)
		,itsToolBox->ConvertCtrlKey(nFlags)) : false;
	if(needsUpdate)
	{
		if(itsSmartMetDocumentInterface && itsSmartMetDocumentInterface->OpenPopupMenu())
		{
			CFmiMenu menu;
			menu.Init(itsSmartMetDocumentInterface->PopupMenu());
			CMenu* pPopup = menu.Popup();

			CRect tempRect;
			GetWindowRect(tempRect);
			pPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x+tempRect.left, point.y+tempRect.top, this);
            itsSmartMetDocumentInterface->OpenPopupMenu(false);
		}
		else
		{
			Invalidate(FALSE);
			if(itsSmartMetDocumentInterface->TimeSerialViewDirty())
			{
				MakeAllMapViewsUpdated(itsManagerView->TimeScaleUpdated()); // tämä on pika viritys optimointi, että joka klikkauksella aikasarjaikkunassa ei piirrettäisi karttanäyttöä
				itsManagerView->TimeScaleUpdated(false);
                itsSmartMetDocumentInterface->RefreshApplicationViewsAndDialogs("TimeSerialView: Right mouse button up action", SmartMetViewId::AllMapViews | SmartMetViewId::TimeSerialView);
			}
		}
	}
	else
	{
//	CView::OnRButtonUp(nFlags, point); // tarvitaaanko????
	}
}

bool CTimeEditValuesView::ChangeTimeSeriesValues(void)
{
	if(itsManagerView)
		return itsManagerView->ChangeTimeSeriesValues();
	return false;
}

void CTimeEditValuesView::ResetAllModifyFactorValues(void)
{
	if(itsManagerView)
		itsManagerView->ResetAllModifyFactorValues();
}

void CTimeEditValuesView::OnSize(UINT nType, int cx, int cy)
{
	CView::OnSize(nType, cx, cy);
    itsSmartMetDocumentInterface->TimeSerialViewDirty(true);
	if(itsToolBox && itsManagerView)
	{
		CtrlView::DeviceContextHandler<CTimeEditValuesView> deviceContextHandler(this);
		NFmiRect rect(0.,0.,1.,1.); // 0,0 - 1,1 rect I hope?
		itsManagerView->Update(rect,itsToolBox);
	}
	CRect rect;
	GetClientRect(rect);
	m_tooltip.SetToolRect(this, TIMEVIEW_TOOLTIP_ID, rect);
	Invalidate(FALSE);
}

void CTimeEditValuesView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint)
{
	if(itsManagerView)
		itsManagerView->Update();
	Invalidate(FALSE);
}

void CTimeEditValuesView::Update(void)
{
	if(itsManagerView)
		itsManagerView->Update();
	Invalidate(FALSE);
}

BOOL CTimeEditValuesView::OnCommand(WPARAM wParam, LPARAM lParam)
{
	if(itsSmartMetDocumentInterface && itsSmartMetDocumentInterface->PopupMenu())
	{
		NFmiMenuItemList* menuItemList = itsSmartMetDocumentInterface->PopupMenu();
		unsigned short nID = short (wParam);
		if (nID >= menuItemList->MinId() && nID <= menuItemList->MaxId())
		{
			if(itsSmartMetDocumentInterface->MakePopUpCommandUsingRowIndex(nID)) // UsingRowIndex-versiota ei vaikuta lopputulokseen
			{
				itsManagerView->Update();
				Invalidate(FALSE);
			}
		}
	}

	return CView::OnCommand(wParam, lParam);
}


void CTimeEditValuesView::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	// TODO: Add your message handler code here and/or call default

	CView::OnChar(nChar, nRepCnt, nFlags);
}

UINT CTimeEditValuesView::OnGetDlgCode()
{
	UINT result = CZoomView::OnGetDlgCode();
	result = result | DLGC_WANTARROWS;
	return result;
}

void CTimeEditValuesView::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	// TODO: Add your message handler code here and/or call default
	switch(nChar)
	{
	case VK_LEFT:
		//TRACE("LEFT\n");
		break;

	}
	CView::OnKeyUp(nChar, nRepCnt, nFlags);
}

void CTimeEditValuesView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	// TODO: Add your message handler code here and/or call default

	CView::OnKeyDown(nChar, nRepCnt, nFlags);
}

void CTimeEditValuesView::OnPrint(CDC* pDC, CPrintInfo* pInfo)
{
	CFmiWin32TemplateHelpers::OnPrintMapView(this, pDC, pInfo);
}

void CTimeEditValuesView::OnPrepareDC(CDC* pDC, CPrintInfo* pInfo)
{
	CFmiWin32TemplateHelpers::OnPrepareDCMapView(this, pDC, pInfo);

	CView::OnPrepareDC(pDC, pInfo);
}

BOOL CTimeEditValuesView::OnPreparePrinting(CPrintInfo* pInfo)
{
	return CFmiWin32TemplateHelpers::OnPreparePrintingMapView(this, pInfo, true); // true = ei tueta vielä monisivu tulosteita täällä
}

void CTimeEditValuesView::OnEndPrinting(CDC* pDC, CPrintInfo* pInfo)
{
	if(itsSmartMetDocumentInterface)
	{
        itsSmartMetDocumentInterface->Printing(false);
	}

	CView::OnEndPrinting(pDC, pInfo);
}

void CTimeEditValuesView::DoPrint(void)
{
	// TÄMÄ on armoton kikka vitonen!!!!
	// m_pDocument -pointteriin pitää laittaa dokumentti pointteri osoittamaan printtauksen ajaksi
	// koska muuten moni sivuisten printtausten tulostus ei onnistu (CView::OnFilePrint vaatii tässä dokumenttia käyttöön)
	// Yritin tehdä ensin omaa printtaus funktiota kopioimalla originaali osion mutta niin että dokumenttia ei tarvittaisi.
	// Tämä onnistui mutta vain yhdelle paperille kerrallaan. Range print vaatii alkuperäisen CView::OnFilePrint -metodin 
	// käyttöä ja niin että m_pDocument -pointteri osoittaa applikaation dokumentti olioon.
	m_pDocument = ApplicationInterface::GetApplicationInterfaceImplementation()->GetDocumentAsCDocument();

	try
	{
		CView::OnFilePrint();
		Invalidate(FALSE); // ruutu pitää lopuksi päivittää
	}
	catch(...)
	{
	}
	// Lopuksi pointteri on aina nollattava, muuten luokan destruktori yrittää tuhota dokumenttia ja se ei käy.
	m_pDocument = 0;

}

void CTimeEditValuesView::CurrentPrintTime(const NFmiMetTime & /* theTime */ )
{
	//GetGeneralDoc()->CurrentTime(itsMapViewDescTopIndex, theTime);
}

const NFmiRect* CTimeEditValuesView::RelativePrintRect(void) 
{
	return 0; 
}

CSize CTimeEditValuesView::GetPrintedAreaOnScreenSizeInPixels(void)
{
	if(itsMemoryBitmap)
	{
		BITMAP bm;
		itsMemoryBitmap->GetBitmap(&bm);
		return CSize(bm.bmWidth, bm.bmHeight);
	}
	else
		return CSize(0, 0);
}

NFmiMetTime CTimeEditValuesView::CalcPrintingStartTime(void)
{
	return itsSmartMetDocumentInterface->CurrentTime(0); // haetaan pääkarttanäytön alkuaika
}

int CTimeEditValuesView::CalcPrintingPageShiftInMinutes(void)
{
	return FmiRound(itsSmartMetDocumentInterface->MapViewDescTop(0)->TimeControlTimeStep() * 60.);
}

NFmiPoint CTimeEditValuesView::PrintViewSizeInPixels(void)
{
	if(itsMemoryBitmap)
	{
		BITMAP bm;
		itsMemoryBitmap->GetBitmap(&bm);
		return NFmiPoint(bm.bmWidth, bm.bmHeight);
	}
	else 
		return NFmiPoint(0, 0);
}

void CTimeEditValuesView::PrintViewSizeInPixels(const NFmiPoint & /* theSize */ )
{
}

void CTimeEditValuesView::RelativePrintRect(const NFmiRect & /* theRect */ )
{
}

CtrlViewUtils::GraphicalInfo& CTimeEditValuesView::GetGraphicalInfo()
{
	static CtrlViewUtils::GraphicalInfo graphicalInfoDummy;
	return graphicalInfoDummy;
}

TrueMapViewSizeInfo& CTimeEditValuesView::GetTrueMapViewSizeInfo()
{
	static TrueMapViewSizeInfo trueMapViewSizeInfoDummy(MapViewDescTopIndex());
	return trueMapViewSizeInfoDummy;;
}

void CTimeEditValuesView::SetPrintCopyCDC(CDC* /* pDC */ )
{
}

void CTimeEditValuesView::MakePrintViewDirty(bool /* fViewDirty */ , bool /* fCacheDirty */ )
{
//	fViewDirty = fViewDirty;
}

void CTimeEditValuesView::OldWayPrintUpdate(void)
{
}

BOOL CTimeEditValuesView::OnEraseBkgnd(CDC* pDC)
{
	return FALSE;
//	return CView::OnEraseBkgnd(pDC);
}


BOOL CTimeEditValuesView::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	CtrlView::DeviceContextHandler<CTimeEditValuesView> deviceContextHandler(this);
	// Poiketen "button clik" -metodeista, piste tulee tänne absoluuttisena paikkana
	// ja joudun muuttamaan sen suhteelliseksi tähän ikkunaan ensin.
	CRect screenRect;
	GetWindowRect(screenRect);
	CRect client;
	GetClientRect(client);
	CPoint windowPoint(pt.x - (screenRect.left - client.left), pt.y - (screenRect.top - client.top));

	bool needsUpdate = itsManagerView ? itsManagerView->MouseWheel(itsToolBox->ToViewPoint(windowPoint.x, windowPoint.y)
		,itsToolBox->ConvertCtrlKey(nFlags), zDelta) : false;
	if(needsUpdate)
	{
		Invalidate(FALSE);
		if(itsSmartMetDocumentInterface->TimeSerialViewDirty())
		{
			MakeAllMapViewsUpdated(itsManagerView->TimeScaleUpdated()); // tämä on pika viritys optimointi, että joka klikkauksella aikasarjaikkunassa ei piirrettäisi karttanäyttöä
			itsManagerView->TimeScaleUpdated(false);
            itsSmartMetDocumentInterface->RefreshApplicationViewsAndDialogs("TimeSerialView: Mouse wheel action", SmartMetViewId::AllMapViews | SmartMetViewId::TimeSerialView);
		}
		else if(itsSmartMetDocumentInterface->MetEditorOptionsData().ControlPointMode())
            itsSmartMetDocumentInterface->DrawOverBitmapThings(0);
	}
	return TRUE;
}

void CTimeEditValuesView::MakeAllMapViewsUpdated(bool newValue)
{
    auto &mapViewDescTopList = itsSmartMetDocumentInterface->MapViewDescTopList();
    for(auto &mapViewDescTop : mapViewDescTopList)
        mapViewDescTop->SetRedrawMapView(newValue); // tämä on pika viritys optimointi, että joka klikkauksella aikasarjaikkunassa ei piirrettäisi karttanäyttöä
}

BOOL CTimeEditValuesView::PreTranslateMessage(MSG* pMsg)
{
	if(itsSmartMetDocumentInterface && itsSmartMetDocumentInterface->ShowToolTipTimeView())
		m_tooltip.RelayEvent(pMsg);

	return CZoomView::PreTranslateMessage(pMsg);
}

void CTimeEditValuesView::NotifyDisplayTooltip(NMHDR * pNMHDR, LRESULT * result)
{
    if(!CFmiWin32TemplateHelpers::AllowTooltipDisplay(itsSmartMetDocumentInterface))
        return;

	*result = 0;
	NM_PPTOOLTIP_DISPLAY * pNotify = (NM_PPTOOLTIP_DISPLAY*)pNMHDR;

	if (pNotify->ti->nIDTool == TIMEVIEW_TOOLTIP_ID)
	{
		CPoint pt = *pNotify->pt;
		ScreenToClient(&pt);

        CString strU_;

		try
		{
			CtrlView::DeviceContextHandler<CTimeEditValuesView> deviceContextHandler(this);
			NFmiPoint relativePoint(itsToolBox->ToViewPoint(pt.x, pt.y));
            strU_ = CA2T(itsManagerView->ComposeToolTipText(relativePoint).c_str());
		}
		catch(std::exception &e)
		{
            strU_ = _TEXT("Error while making the tooltip string:\n");
            strU_ += CA2T(e.what());
		}
		catch(...)
		{
            strU_ = _TEXT("Error (unknown) while making the tooltip string");
		}

        if(strU_.IsEmpty())
		{
			if(itsSmartMetDocumentInterface->EditedSmartInfo() == 0)
                strU_ = _TEXT("No data in system");
		}
        pNotify->ti->sTooltip = strU_;

	} //if
} //End NotifyDisplayTooltip

void CTimeEditValuesView::HideToolTip(void)
{
	m_tooltip.HideTooltip();
}

void CTimeEditValuesView::AutoAdjustValueScales(bool justActiveRow, bool redrawOnSuccess)
{
	CtrlView::DeviceContextHandler<CTimeEditValuesView> deviceContextHandler(this);
	if(itsManagerView->AutoAdjustValueScales(justActiveRow))
	{
		if(redrawOnSuccess)
		{
			Invalidate(FALSE);
		}
	}
}

std::string CTimeEditValuesView::MakeCsvDataString()
{
	return itsManagerView->MakeCsvDataString();
}
