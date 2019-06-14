#include "stdafx.h"
#include "FmiCrossSectionView.h"
#include "NFmiToolBox.h"
#include "NFmiCrossSectionManagerView.h"
#include "SmartMetDocumentInterface.h"
#include "NFmiDrawParam.h"
#include "NFmiCrossSectionSystem.h"
#include "CFmiPopupMenu.h"
#include "CFmiMenu.h"
#include "NFmiMenuItemList.h"
#include "FmiWin32Helpers.h"
#include "FmiWin32TemplateHelpers.h"
#include "CtrlViewKeyboardFunctions.h"
#include "CtrlViewFunctions.h"
#include "ApplicationInterface.h"
#include "CtrlViewWin32Functions.h"
#include "NFmiMacroParamDataCache.h"
#include "SpecialDesctopIndex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CFmiTempView

IMPLEMENT_DYNCREATE(CFmiCrossSectionView, CView)

CFmiCrossSectionView::CFmiCrossSectionView(CWnd* theParent
										, SmartMetDocumentInterface *smartMetDocumentInterface)
:CZoomView(theParent, smartMetDocumentInterface)
,itsView(0)
,itsMemoryBitmap(0)
,fViewDirty(true)
{
}

CFmiCrossSectionView::CFmiCrossSectionView()
{
}

CFmiCrossSectionView::~CFmiCrossSectionView()
{
	delete itsView;
	if(itsMemoryBitmap)
	{
		itsMemoryBitmap->DeleteObject();
		delete itsMemoryBitmap;
	}
}

BEGIN_MESSAGE_MAP(CFmiCrossSectionView, CView)
	//{{AFX_MSG_MAP(CFmiCrossSectionView)
	ON_WM_ERASEBKGND()
	//}}AFX_MSG_MAP
	ON_WM_LBUTTONUP()
	ON_WM_RBUTTONUP()
ON_WM_MOUSEMOVE()
ON_WM_SIZE()
ON_WM_MOUSEWHEEL()
	ON_NOTIFY (UDM_TOOLTIP_DISPLAY, NULL, NotifyDisplayTooltip)
	ON_WM_LBUTTONDOWN()
ON_WM_MBUTTONUP()
ON_WM_LBUTTONDBLCLK()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFmiTempView drawing

void CFmiCrossSectionView::OnDraw(CDC* pDC)
{
	if(itsSmartMetDocumentInterface->Printing())
		return ; // printatessa ei saa mennä OnDraw-metodiin, koska tällöin systeemit kaatuu

	CFmiWin32Helpers::SetDescTopGraphicalInfo(GetGraphicalInfo(), pDC, PrintViewSizeInPixels(), itsSmartMetDocumentInterface->DrawObjectScaleFactor(), true); // true pakottaa initialisoinnin

	CClientDC dc(this);
	CDC dcMem;
	dcMem.CreateCompatibleDC(&dc);
	CRect clientRect;
	GetClientRect(&clientRect);
	CBitmap *oldBitmap = 0;
	if(fViewDirty)
	{
		std::auto_ptr<CWaitCursor> waitCursor = CFmiWin32Helpers::GetWaitCursorIfNeeded(itsSmartMetDocumentInterface->ShowWaitCursorWhileDrawingView());
		if(itsMemoryBitmap)
			itsMemoryBitmap->DeleteObject();
		else
			itsMemoryBitmap = new CBitmap;

		itsMemoryBitmap->CreateCompatibleBitmap(&dc, clientRect.Width(), clientRect.Height());
		ASSERT(itsMemoryBitmap->m_hObject != 0);

		oldBitmap = dcMem.SelectObject(itsMemoryBitmap);
		itsToolBox->SetDC(&dcMem);
		SetToolMastersDC(&dcMem);

		DoDraw();
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

void CFmiCrossSectionView::DoDraw(void)
{
	itsSmartMetDocumentInterface->TransparencyContourDrawView(this); // tämä on osa kikkaa, jolla saadaan piiirettyä läpinäkyviä kenttiä toolmasterilla tai imaginella
	try
	{
		itsView->Draw(itsToolBox);
	}
	catch(std::exception &e)
	{
		std::string errorTitleStr("Error with cross-section-view drawing");
		std::string errorStr("Error while drawing cross-section-view, there won't be more reports of these:\n");
		errorStr += e.what();
        itsSmartMetDocumentInterface->LogAndWarnUser(errorStr, errorTitleStr, CatLog::Severity::Error, CatLog::Category::Visualization, true);
	}
	catch(...)
	{
		std::string errorTitleStr("Unknown error with cross-section-view drawing");
		std::string errorStr("Unknown error while drawing cross-section-view, there won't be more reports of these.");
        itsSmartMetDocumentInterface->LogAndWarnUser(errorStr, errorTitleStr, CatLog::Severity::Error, CatLog::Category::Visualization, true);
	}
    itsSmartMetDocumentInterface->TransparencyContourDrawView(0); // lopuksi pitää nollata läpinäkyvyys-näyttö pointteri
}

// toolmasterin DC:n
void CFmiCrossSectionView::SetToolMastersDC(CDC* theDC)
{
    CtrlView::SetToolMastersDC(theDC, this, itsSmartMetDocumentInterface->IsToolMasterAvailable());
}

void CFmiCrossSectionView::DrawOverBitmapThings(NFmiToolBox *theToolBox)
{
}

/////////////////////////////////////////////////////////////////////////////
// CFmiTempView diagnostics

#ifdef _DEBUG
void CFmiCrossSectionView::AssertValid() const
{
	CView::AssertValid();
}

void CFmiCrossSectionView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CFmiCrossSectionView message handlers

static const int CROSSSECTIOVIEW_TOOLTIP_ID = 1234568;

void CFmiCrossSectionView::OnInitialUpdate()
{
	static NFmiDrawingEnvironment envi;
	CView::OnInitialUpdate(); // älä kutsu lähintä emoa!!!

	itsToolBox = new NFmiToolBox(this); // tämä on lähimmässä emossa ja luotava tässä!!
	CDC *theDC = GetDC();
	if(!theDC)
		return;
	itsToolBox->SetDC(theDC);
	NFmiRect rect(0.,0.,1.,1.); // 0,0 - 1,1 rect I hope?
	itsView = new NFmiCrossSectionManagerView(rect, itsToolBox, &envi);
	ReleaseDC(theDC);
	CFmiWin32Helpers::InitializeCPPTooltip(this, m_tooltip, CROSSSECTIOVIEW_TOOLTIP_ID);
	CRect winRec;
	GetWindowRect(winRec);
	OnSize(SW_RESTORE, winRec.Width(), winRec.Height()); // jostain syystä on pakko tehdä onsize, että tooltip toimii varmasti koko ikkunan alueella?!?!?
}

void CFmiCrossSectionView::Update(bool fMakeDirty)
{
	fViewDirty = fMakeDirty;
    if(itsSmartMetDocumentInterface->CrossSectionSystem()->CrossSectionViewOn() == false)
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

BOOL CFmiCrossSectionView::OnEraseBkgnd(CDC* pDC)
{
	return FALSE;

//	return CView::OnEraseBkgnd(pDC);
}

void CFmiCrossSectionView::OnPrint(CDC* pDC, CPrintInfo* pInfo)
{
	CFmiWin32TemplateHelpers::OnPrintMapView(this, pDC, pInfo);
}

void CFmiCrossSectionView::OnPrepareDC(CDC* pDC, CPrintInfo* pInfo)
{
	CFmiWin32TemplateHelpers::OnPrepareDCMapView(this, pDC, pInfo);

	CView::OnPrepareDC(pDC, pInfo);
}

BOOL CFmiCrossSectionView::OnPreparePrinting(CPrintInfo* pInfo)
{
	return CFmiWin32TemplateHelpers::OnPreparePrintingMapView(this, pInfo, true); // true = ei tueta vielä monisivu tulosteita täällä
}

void CFmiCrossSectionView::OnEndPrinting(CDC* pDC, CPrintInfo* pInfo)
{
    itsSmartMetDocumentInterface->Printing(false);

	CView::OnEndPrinting(pDC, pInfo);
}

void CFmiCrossSectionView::DoPrint(void)
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

void CFmiCrossSectionView::OnLButtonUp(UINT nFlags, CPoint point)
{
	try
	{
		CDC *theDC = GetDC();
		if(!theDC)
			return;
		itsToolBox->SetDC(theDC);
		CtrlView::ReleaseCtrlKeyIfStuck(nFlags); // tämä vapauttaa CTRL-napin, jos se on 'jumiutunut' pohjaan (MFC bugi, ctrl-nappi voi jäädä pohjaan, jos kyseinen näppäin vapautetaan, ennen kuin kartta ruudun piirto on valmis)

		bool needsUpdate = itsView ? itsView->LeftButtonUp(itsToolBox->ToViewPoint(point.x, point.y)
			,itsToolBox->ConvertCtrlKey(nFlags)) : false;
		ReleaseDC(theDC);
		if(needsUpdate)
		{
			fViewDirty = true;
            itsSmartMetDocumentInterface->RefreshApplicationViewsAndDialogs(__FUNCTION__, SmartMetViewId::AllMapViews | SmartMetViewId::CrossSectionView);
			Invalidate(FALSE);
		}
	}
	catch(...)
	{
		ReleaseCapture();
		throw ;
	}
	ReleaseCapture();
}

void CFmiCrossSectionView::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	CDC *theDC = GetDC();
	if(!theDC)
		return;
	itsToolBox->SetDC(theDC);

	CtrlView::ReleaseCtrlKeyIfStuck(nFlags); // tämä vapauttaa CTRL-napin, jos se on 'jumiutunut' pohjaan (MFC bugi, ctrl-nappi voi jäädä pohjaan, jos kyseinen näppäin vapautetaan, ennen kuin kartta ruudun piirto on valmis)

	bool needsUpdate = itsView ? itsView->LeftDoubleClick(itsToolBox->ToViewPoint(point.x, point.y)
		,itsToolBox->ConvertCtrlKey(nFlags)) : false;
	ReleaseDC(theDC);
	if(needsUpdate)
	{
		if(itsSmartMetDocumentInterface->ActivateParamSelectionDlgAfterLeftDoubleClick())
		{
            itsSmartMetDocumentInterface->ActivateParamSelectionDlgAfterLeftDoubleClick(false);
            itsSmartMetDocumentInterface->ActivateViewParamSelectorDlg(itsView->MapViewDescTopIndex());
			return ;
		}
		Invalidate(FALSE);
        itsSmartMetDocumentInterface->RefreshApplicationViewsAndDialogs(__FUNCTION__, SmartMetViewId::AllMapViews | SmartMetViewId::CrossSectionView);
	}
//	CView::OnLButtonDblClk(nFlags, point);
}

void CFmiCrossSectionView::OnMButtonUp(UINT nFlags, CPoint point)
{
	CDC *theDC = GetDC();
	if(!theDC)
		return;
	itsToolBox->SetDC(theDC);
    CtrlView::ReleaseCtrlKeyIfStuck(nFlags); // tämä vapauttaa CTRL-napin, jos se on 'jumiutunut' pohjaan (MFC bugi, ctrl-nappi voi jäädä pohjaan, jos kyseinen näppäin vapautetaan, ennen kuin kartta ruudun piirto on valmis)

	bool needsUpdate = itsView ? itsView->MiddleButtonUp(itsToolBox->ToViewPoint(point.x, point.y)
		,itsToolBox->ConvertCtrlKey(nFlags)) : false;
	ReleaseDC(theDC);
	if(needsUpdate)
	{
		fViewDirty = true;
        itsSmartMetDocumentInterface->RefreshApplicationViewsAndDialogs(__FUNCTION__, SmartMetViewId::AllMapViews | SmartMetViewId::CrossSectionView);
		Invalidate(FALSE);
	}

//	CZoomView::OnMButtonUp(nFlags, point);
}

void CFmiCrossSectionView::OnRButtonUp(UINT nFlags, CPoint point)
{
	CDC *theDC = GetDC();
	if(!theDC)
		return;
	itsToolBox->SetDC(theDC);
    CtrlView::ReleaseCtrlKeyIfStuck(nFlags); // tämä vapauttaa CTRL-napin, jos se on 'jumiutunut' pohjaan (MFC bugi, ctrl-nappi voi jäädä pohjaan, jos kyseinen näppäin vapautetaan, ennen kuin kartta ruudun piirto on valmis)

	bool needsUpdate = itsView ? itsView->RightButtonUp(itsToolBox->ToViewPoint(point.x, point.y)
		,itsToolBox->ConvertCtrlKey(nFlags)) : false;
	ReleaseDC(theDC);
	if(needsUpdate)
	{

		if(itsSmartMetDocumentInterface->OpenPopupMenu())
		{
            CFmiPopupMenu menu;
//            CFmiMenu menu;
			menu.Init(itsSmartMetDocumentInterface->PopupMenu());
			CMenu* pPopup = menu.Popup();

			CRect tempRect;
			GetWindowRect(tempRect);
			pPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x+tempRect.left, point.y+tempRect.top, this);
            itsSmartMetDocumentInterface->OpenPopupMenu(false);
		}
		else
		{
			fViewDirty = true;
			Invalidate(FALSE);
            itsSmartMetDocumentInterface->RefreshApplicationViewsAndDialogs(__FUNCTION__, SmartMetViewId::AllMapViews | SmartMetViewId::CrossSectionView);
		}
	}
}

void CFmiCrossSectionView::OnLButtonDown(UINT nFlags, CPoint point)
{
	CDC *theDC = GetDC();
	if(!theDC)
		return;
	itsToolBox->SetDC(theDC);
    CtrlView::ReleaseCtrlKeyIfStuck(nFlags); // tämä vapauttaa CTRL-napin, jos se on 'jumiutunut' pohjaan (MFC bugi, ctrl-nappi voi jäädä pohjaan, jos kyseinen näppäin vapautetaan, ennen kuin kartta ruudun piirto on valmis)

	bool needsUpdate = itsView ? itsView->LeftButtonDown(itsToolBox->ToViewPoint(point.x, point.y)
		,itsToolBox->ConvertCtrlKey(nFlags)) : false;
	ReleaseDC(theDC);
	if(needsUpdate)
	{
		fViewDirty = true;
		Invalidate(FALSE);
	}
	SetCapture();
}

void CFmiCrossSectionView::OnMouseMove(UINT nFlags, CPoint point)
{
	isCurrentMousePoint = point;

	CDC *theDC = GetDC();
	if(!theDC)
		return;
	itsToolBox->SetDC(theDC);
    CtrlView::ReleaseCtrlKeyIfStuck(nFlags); // tämä vapauttaa CTRL-napin, jos se on 'jumiutunut' pohjaan (MFC bugi, ctrl-nappi voi jäädä pohjaan, jos kyseinen näppäin vapautetaan, ennen kuin kartta ruudun piirto on valmis)

	bool needsUpdate = itsView ? itsView->MouseMove(itsToolBox->ToViewPoint(point.x, point.y)
		,itsToolBox->ConvertCtrlKey(nFlags)) : false;
	ReleaseDC(theDC);
	if(needsUpdate)
	{
		fViewDirty = true;
		Invalidate(FALSE);
	}

}

void CFmiCrossSectionView::OnSize(UINT nType, int cx, int cy)
{
	CZoomView::OnSize(nType, cx, cy);

	CRect rect;
	GetClientRect(rect);
	m_tooltip.SetToolRect(this, CROSSSECTIOVIEW_TOOLTIP_ID, rect);
    itsSmartMetDocumentInterface->CrossSectionViewSizeInPixels(NFmiPoint(rect.Width(), rect.Height()));
	CDC *theDC = GetDC();
	CFmiWin32Helpers::SetDescTopGraphicalInfo(GetGraphicalInfo(), theDC, PrintViewSizeInPixels(), itsSmartMetDocumentInterface->DrawObjectScaleFactor(), true); // true pakottaa initialisoinnin

	Update(true);
}

BOOL CFmiCrossSectionView::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	CDC *theDC = GetDC();
	if(!theDC)
		return FALSE;
	itsToolBox->SetDC(theDC);
    CtrlView::ReleaseCtrlKeyIfStuck(nFlags); // tämä vapauttaa CTRL-napin, jos se on 'jumiutunut' pohjaan (MFC bugi, ctrl-nappi voi jäädä pohjaan, jos kyseinen näppäin vapautetaan, ennen kuin kartta ruudun piirto on valmis)

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
		Update(true);
		Invalidate(FALSE);
		if(itsSmartMetDocumentInterface->CrossSectionSystem()->CrossSectionSystemActive())
		{ 
            // jos karttanäytössä crosssection moodi päällä, päivitetään kartta ja muutkin näytöt
            itsSmartMetDocumentInterface->MacroParamDataCache().clearView(CtrlViewUtils::kFmiCrossSectionView);
            itsSmartMetDocumentInterface->MapViewDirty(CtrlViewUtils::kDoAllMapViewDescTopIndex, false, false, true, false, false, false); // laitetaan viela kaikki ajat likaisiksi cachesta
            itsSmartMetDocumentInterface->RefreshApplicationViewsAndDialogs(__FUNCTION__, SmartMetViewId::AllMapViews | SmartMetViewId::CrossSectionView);
		}
	}
	return TRUE;
}

BOOL CFmiCrossSectionView::OnCommand(WPARAM wParam, LPARAM lParam)
{
    auto popupMenu = itsSmartMetDocumentInterface->PopupMenu();
	if(popupMenu)
	{
		unsigned short nID = short (wParam);
		if (nID >= popupMenu->MinId() && nID <= popupMenu->MaxId())
		{
			if(itsSmartMetDocumentInterface->MakePopUpCommandUsingRowIndex(nID)) // UsingRowIndex-versiota ei vaikuta lopputulokseen
			{
				Update(true);
				Invalidate(FALSE);
			}
		}
	}

	return CZoomView::OnCommand(wParam, lParam);
}


BOOL CFmiCrossSectionView::PreTranslateMessage(MSG* pMsg)
{
    if(itsSmartMetDocumentInterface->CrossSectionSystem()->ShowTooltipOnCrossSectionView())
		m_tooltip.RelayEvent(pMsg);

	return CZoomView::PreTranslateMessage(pMsg);
}

void CFmiCrossSectionView::NotifyDisplayTooltip(NMHDR * pNMHDR, LRESULT * result)
{

	*result = 0;
	NM_PPTOOLTIP_DISPLAY * pNotify = (NM_PPTOOLTIP_DISPLAY*)pNMHDR;

	if (pNotify->ti->nIDTool == CROSSSECTIOVIEW_TOOLTIP_ID)
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

NFmiMetTime CFmiCrossSectionView::CalcPrintingStartTime(void)
{
	return itsSmartMetDocumentInterface->CurrentTime(0); // haetaan pääkarttanäytön alkuaika
}

int CFmiCrossSectionView::CalcPrintingPageShiftInMinutes(void)
{
	return FmiRound(itsSmartMetDocumentInterface->MapViewDescTop(0)->TimeControlTimeStep() * 60.);
}

CtrlViewUtils::GraphicalInfo& CFmiCrossSectionView::GetGraphicalInfo(void)
{
	return itsSmartMetDocumentInterface->CrossSectionSystem()->GetGraphicalInfo();
}

void CFmiCrossSectionView::CurrentPrintTime(const NFmiMetTime & /* theTime */ )
{
	//GetGeneralDoc()->CurrentTime(itsMapViewDescTopIndex, theTime);
}

const NFmiRect* CFmiCrossSectionView::RelativePrintRect(void) 
{
	return 0; 
}

void CFmiCrossSectionView::OldWayPrintUpdate(void)
{
}

void CFmiCrossSectionView::MakePrintViewDirty(bool fViewDirty, bool /* fCacheDirty */ )
{
	fViewDirty = fViewDirty;
}

void CFmiCrossSectionView::SetPrintCopyCDC(CDC* /* pDC */ )
{
}

NFmiPoint CFmiCrossSectionView::PrintViewSizeInPixels(void)
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

void CFmiCrossSectionView::PrintViewSizeInPixels(const NFmiPoint & /* theSize */ )
{
}

void CFmiCrossSectionView::RelativePrintRect(const NFmiRect & /* theRect */ )
{
}

CSize CFmiCrossSectionView::GetPrintedAreaOnScreenSizeInPixels(void)
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

