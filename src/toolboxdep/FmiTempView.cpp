// FmiTempView.cpp : implementation file
//

#include "stdafx.h"
#include "FmiTempView.h"
#include "NFmiToolBox.h"
#include "SmartMetDocumentInterface.h"
#include "NFmiTempView.h"
#include "FmiWin32Helpers.h"
#include "FmiWin32TemplateHelpers.h"
#include "NFmiMapViewDescTop.h"
#include "ApplicationInterface.h"

/*
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
*/

static const int TEMPVIEW_TOOLTIP_ID = 1234764;

/////////////////////////////////////////////////////////////////////////////
// CFmiTempView

IMPLEMENT_DYNCREATE(CFmiTempView, CView)

CFmiTempView::CFmiTempView(CWnd* theParent, SmartMetDocumentInterface *smartMetDocumentInterface)
:CZoomView(theParent, smartMetDocumentInterface)
,itsMemoryBitmap(0)
{
}

CFmiTempView::CFmiTempView()
{
}

CFmiTempView::~CFmiTempView()
{
	delete itsView;
	if(itsMemoryBitmap)
	{
		itsMemoryBitmap->DeleteObject();
		delete itsMemoryBitmap;
	}
}

BEGIN_MESSAGE_MAP(CFmiTempView, CView)
	//{{AFX_MSG_MAP(CFmiTempView)
	ON_WM_ERASEBKGND()
	//}}AFX_MSG_MAP
	ON_WM_LBUTTONUP()
	ON_WM_RBUTTONUP()
	ON_WM_SIZE()
	ON_WM_MOUSEWHEEL()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDOWN()
	ON_WM_RBUTTONDOWN()
	ON_NOTIFY (UDM_TOOLTIP_DISPLAY, NULL, NotifyDisplayTooltip)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFmiTempView drawing

void CFmiTempView::OnDraw(CDC* pDC)
{
	if(itsSmartMetDocumentInterface->Printing())
		return ; // printatessa ei saa mennä OnDraw-metodiin, koska tällöin systeemit kaatuu

	CFmiWin32Helpers::SetDescTopGraphicalInfo(GetGraphicalInfo(), pDC, PrintViewSizeInPixels(), itsSmartMetDocumentInterface->DrawObjectScaleFactor());

    CFmiWin32Helpers::MemoryBitmapHelper bitmapHelper(this, itsMemoryBitmap);
	if(fViewDirty)
	{
        bitmapHelper.SetupMemoryBitmap();
        bitmapHelper.SetMemoryBitmapInUse();

		itsToolBox->SetDC(&bitmapHelper.dcMem);
		DoDraw();
	}
	else
        bitmapHelper.SetMemoryBitmapInUse();

	BOOL status = bitmapHelper.clientDc.BitBlt(0
							 ,0
							 , bitmapHelper.clientRect.Width()
							 , bitmapHelper.clientRect.Height()
							 ,&bitmapHelper.dcMem
						     ,0
						     ,0
						     ,SRCCOPY);

    itsMemoryBitmap = bitmapHelper.GetMemoryBitmapFromUse();

	itsToolBox->SetDC(&bitmapHelper.clientDc);
	DrawOverBitmapThings(itsToolBox);
	fViewDirty = false; // 'putsataan' ikkuna
}

/////////////////////////////////////////////////////////////////////////////
// CFmiTempView diagnostics

#ifdef _DEBUG
void CFmiTempView::AssertValid() const
{
	CView::AssertValid();
}

void CFmiTempView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CFmiTempView message handlers

void CFmiTempView::DoDraw(void)
{
	static bool firstTimeInError = true;
	try
	{
		itsView->Draw(itsToolBox);
	}
	catch(std::exception &e)
	{
		if(firstTimeInError)
		{
			firstTimeInError = false;
			std::string errorTitleStr("Error with sounding-view drawing");
			std::string errorStr("Error while drawing sounding-view, there won't be more reports of these:\n");
			errorStr += e.what();
            itsSmartMetDocumentInterface->LogAndWarnUser(errorStr, errorTitleStr, CatLog::Severity::Error, CatLog::Category::Visualization, false);
		}
	}
	catch(...)
	{
		if(firstTimeInError)
		{
			firstTimeInError = false;
			std::string errorTitleStr("Unknown error with sounding-view drawing");
			std::string errorStr("Unknown error while drawing sounding-view, there won't be more reports of these.");
            itsSmartMetDocumentInterface->LogAndWarnUser(errorStr, errorTitleStr, CatLog::Severity::Error, CatLog::Category::Visualization, false);
		}
	}
}


void CFmiTempView::OnInitialUpdate()
{
	CView::OnInitialUpdate(); // älä kutsu lähintä emoa!!!

	itsToolBox = new NFmiToolBox(this); // tämä on lähimmässä emossa ja luotava tässä!!
	NFmiRect rect(0.,0.,1.,1.); // 0,0 - 1,1 rect I hope?

	itsView = new NFmiTempView(rect, itsToolBox);
	fViewDirty = true;

	CFmiWin32Helpers::InitializeCPPTooltip(this, m_tooltip, TEMPVIEW_TOOLTIP_ID);
	CRect winRec;
	GetWindowRect(winRec);
	OnSize(SW_RESTORE, winRec.Width(), winRec.Height()); // jostain syystä on pakko tehdä onsize, että tooltip toimii varmasti koko ikkunan alueella?!?!?
}

void CFmiTempView::OnLButtonUp(UINT nFlags, CPoint point)
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

void CFmiTempView::OnRButtonUp(UINT nFlags, CPoint point)
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

BOOL CFmiTempView::OnEraseBkgnd(CDC* pDC)
{
	return FALSE;

//	return CView::OnEraseBkgnd(pDC);
}

void CFmiTempView::OnPrint(CDC* pDC, CPrintInfo* pInfo)
{
	CFmiWin32TemplateHelpers::OnPrintMapView(this, pDC, pInfo);
}

void CFmiTempView::OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo)
{
	// TODO: Add your specialized code here and/or call the base class

	CZoomView::OnBeginPrinting(pDC, pInfo);
}

void CFmiTempView::OnPrepareDC(CDC* pDC, CPrintInfo* pInfo)
{
	CFmiWin32TemplateHelpers::OnPrepareDCMapView(this, pDC, pInfo);

	CView::OnPrepareDC(pDC, pInfo);
}

BOOL CFmiTempView::OnPreparePrinting(CPrintInfo* pInfo)
{
	return CFmiWin32TemplateHelpers::OnPreparePrintingMapView(this, pInfo, true); // true = ei tueta vielä monisivu tulosteita täällä
}

void CFmiTempView::OnEndPrinting(CDC* pDC, CPrintInfo* pInfo)
{
	if(itsSmartMetDocumentInterface)
	{
        itsSmartMetDocumentInterface->Printing(false);
	}

	CView::OnEndPrinting(pDC, pInfo);
}

void CFmiTempView::DoPrint(void)
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

void CFmiTempView::OnSize(UINT nType, int cx, int cy)
{
	fViewDirty = true;
	CZoomView::OnSize(nType, cx, cy);

	CDC *theDC = GetDC();
	CFmiWin32Helpers::SetDescTopGraphicalInfo(GetGraphicalInfo(), theDC, PrintViewSizeInPixels(), itsSmartMetDocumentInterface->DrawObjectScaleFactor(), true); // true pakottaa initialisoinnin
	CRect rect;
	GetClientRect(rect);
	m_tooltip.SetToolRect(this, TEMPVIEW_TOOLTIP_ID, rect);
}

void CFmiTempView::Update(bool fMakeDirty)
{
	fViewDirty = fMakeDirty;
    if(itsSmartMetDocumentInterface && itsSmartMetDocumentInterface->GetMTATempSystem().TempViewOn() == false)
        return ;
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

void CFmiTempView::ResetScales(void)
{
	fViewDirty = true;
	itsView->ResetScales();
}

BOOL CFmiTempView::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	CDC *theDC = GetDC();
	if(!theDC)
		return FALSE;
	itsToolBox->SetDC(theDC);

	// Poiketen "button clik" -metodeista, piste tulee tänne absoluuttisena paikkana
	// ja joudun muuttamaan sen suhteelliseksi tähän ikkunaan ensin.
	CRect screenRect;
	GetWindowRect(screenRect);
	CRect client;
	GetClientRect(client);
	CPoint windowPoint(pt.x - (screenRect.left - client.left), pt.y - (screenRect.top - client.top));

	bool needsUpdate = itsView ? itsView->MouseWheel(itsToolBox->ToViewPoint(windowPoint.x, windowPoint.y)
		,itsToolBox->ConvertCtrlKey(nFlags), zDelta) : false;
	ReleaseDC(theDC);
	if(needsUpdate)
	{
		fViewDirty = true;
		Invalidate(FALSE);
	}
	return TRUE;
}

void CFmiTempView::OnMouseMove(UINT nFlags, CPoint point)
{
	isCurrentMousePoint = point;
	CDC *theDC = GetDC();
	if(!theDC)
		return;
	itsToolBox->SetDC(theDC);

	bool needsUpdate = itsView ? itsView->MouseMove(itsToolBox->ToViewPoint(point.x, point.y)
		,itsToolBox->ConvertCtrlKey(nFlags)) : false;
	ReleaseDC(theDC);
	if(needsUpdate)
	{
		fViewDirty = true;
		Invalidate(FALSE);
	}
	else
		Invalidate(FALSE); // tämä pitää tehdä aina, että SHIFT apuviivat piirtyvät luotausnäyttöön
}

void CFmiTempView::DrawOverBitmapThings(NFmiToolBox *theToolBox)
{
	itsToolBox = theToolBox;
	itsView->DrawOverBitmapThings(itsToolBox, itsToolBox->ToViewPoint(isCurrentMousePoint.x, isCurrentMousePoint.y));
}

void CFmiTempView::ResetSoundingData(void)
{
	itsView->MustResetFirstSoundingData(true);
}

void CFmiTempView::OnLButtonDown(UINT nFlags, CPoint point)
{
	CDC *theDC = GetDC();
	if(!theDC)
		return;
	itsToolBox->SetDC(theDC);

	bool needsUpdate = itsView ? itsView->LeftButtonDown(itsToolBox->ToViewPoint(point.x, point.y)
		,itsToolBox->ConvertCtrlKey(nFlags)) : false;
	ReleaseDC(theDC);
	if(needsUpdate)
	{
		fViewDirty = true;
		Invalidate(FALSE);
	}
}

void CFmiTempView::OnRButtonDown(UINT nFlags, CPoint point)
{
	CDC *theDC = GetDC();
	if(!theDC)
		return;
	itsToolBox->SetDC(theDC);

	bool needsUpdate = itsView ? itsView->RightButtonDown(itsToolBox->ToViewPoint(point.x, point.y)
		,itsToolBox->ConvertCtrlKey(nFlags)) : false;
	ReleaseDC(theDC);
	if(needsUpdate)
	{
		fViewDirty = true;
		Invalidate(FALSE);
	}
}

const std::string& CFmiTempView::SoundingIndexStr(void) const
{
	static std::string dummy("");
	if(itsView)
		return itsView->SoundingIndexStr();
	return dummy;
}

void CFmiTempView::CurrentPrintTime(const NFmiMetTime & /* theTime */ )
{
	// ei toimintaa luotausnäytössä vielä
	//GetGeneralDoc()->CurrentTime(itsMapViewDescTopIndex, theTime);
}

const NFmiRect* CFmiTempView::RelativePrintRect(void) 
{
	return 0; 
//	return &(GetGeneralDoc()->MapViewDescTop(itsMapViewDescTopIndex)->RelativeMapRect()); 
}

void CFmiTempView::OldWayPrintUpdate(void)
{
}

void CFmiTempView::MakePrintViewDirty(bool fViewDirty, bool /* fCacheDirty */ )
{
	fViewDirty = fViewDirty;
}

void CFmiTempView::SetPrintCopyCDC(CDC* /* pDC */ )
{
}

NFmiPoint CFmiTempView::PrintViewSizeInPixels(void)
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

void CFmiTempView::PrintViewSizeInPixels(const NFmiPoint & /* theSize */ )
{
}

void CFmiTempView::RelativePrintRect(const NFmiRect & /* theRect */ )
{
}

CSize CFmiTempView::GetPrintedAreaOnScreenSizeInPixels(void)
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

CtrlViewUtils::GraphicalInfo& CFmiTempView::GetGraphicalInfo(void)
{
	return itsSmartMetDocumentInterface->GetMTATempSystem().GetGraphicalInfo();
}

NFmiMetTime CFmiTempView::CalcPrintingStartTime(void)
{
	return itsSmartMetDocumentInterface->CurrentTime(0); // haetaan pääkarttanäytön alkuaika
}

int CFmiTempView::CalcPrintingPageShiftInMinutes(void)
{
	return itsSmartMetDocumentInterface->GetMTATempSystem().AnimationTimeStepInMinutes();
}

BOOL CFmiTempView::PreTranslateMessage(MSG* pMsg)
{
	if(itsSmartMetDocumentInterface->ShowToolTipTempView())
		m_tooltip.RelayEvent(pMsg);

	return CZoomView::PreTranslateMessage(pMsg);
}

void CFmiTempView::NotifyDisplayTooltip(NMHDR * pNMHDR, LRESULT * result)
{
	*result = 0;
	NM_PPTOOLTIP_DISPLAY * pNotify = (NM_PPTOOLTIP_DISPLAY*)pNMHDR;

	if (pNotify->ti->nIDTool == TEMPVIEW_TOOLTIP_ID)
	{
		CPoint pt = *pNotify->pt;
		ScreenToClient(&pt);

        CString strU_;

		try
		{
			CDC *theDC = GetDC();
			if(!theDC)
				return;
			itsToolBox->SetDC(theDC);
			NFmiPoint relativePoint(itsToolBox->ToViewPoint(pt.x, pt.y));
			ReleaseDC(theDC);
            strU_ = CA2T(itsView->ComposeToolTipText(relativePoint).c_str());
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
