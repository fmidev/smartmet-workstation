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
#include "WaitCursorHelper.h"

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
		return ; // printatessa ei saa menn‰ OnDraw-metodiin, koska t‰llˆin systeemit kaatuu

	CFmiWin32Helpers::SetDescTopGraphicalInfo(IsMapView(), GetGraphicalInfo(), pDC, PrintViewSizeInPixels(), itsSmartMetDocumentInterface->DrawObjectScaleFactor(), true); // true pakottaa initialisoinnin

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
	itsSmartMetDocumentInterface->TransparencyContourDrawView(this); // t‰m‰ on osa kikkaa, jolla saadaan piiiretty‰ l‰pin‰kyvi‰ kentti‰ toolmasterilla tai imaginella
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
    itsSmartMetDocumentInterface->TransparencyContourDrawView(0); // lopuksi pit‰‰ nollata l‰pin‰kyvyys-n‰yttˆ pointteri
}

// toolmasterin DC:n
void CFmiCrossSectionView::SetToolMastersDC(CDC* theDC)
{
    CtrlView::SetToolMastersDC(theDC, this, itsSmartMetDocumentInterface->IsToolMasterAvailable());
}

void CFmiCrossSectionView::DrawOverBitmapThings(NFmiToolBox *theToolBox)
{
}

int CFmiCrossSectionView::MapViewDescTopIndex(void)
{
    return CtrlViewUtils::kFmiCrossSectionView;
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
	CView::OnInitialUpdate(); // ‰l‰ kutsu l‰hint‰ emoa!!!

	itsToolBox = new NFmiToolBox(this); // t‰m‰ on l‰himm‰ss‰ emossa ja luotava t‰ss‰!!
	CtrlView::DeviceContextHandler<CFmiCrossSectionView> deviceContextHandler(this);
	NFmiRect rect(0.,0.,1.,1.); // 0,0 - 1,1 rect I hope?
	itsView = new NFmiCrossSectionManagerView(rect, itsToolBox);
	CFmiWin32Helpers::InitializeCPPTooltip(this, m_tooltip, CROSSSECTIOVIEW_TOOLTIP_ID, CtrlViewUtils::MaxMapViewTooltipWidthInPixels);
	CRect winRec;
	GetWindowRect(winRec);
	OnSize(SW_RESTORE, winRec.Width(), winRec.Height()); // jostain syyst‰ on pakko tehd‰ onsize, ett‰ tooltip toimii varmasti koko ikkunan alueella?!?!?
}

void CFmiCrossSectionView::SetToolsDCs(CDC* theDC)
{
	if(itsToolBox)	
		itsToolBox->SetDC(theDC);
}

void CFmiCrossSectionView::Update(bool fMakeDirty)
{
	fViewDirty = fMakeDirty;
    if(itsSmartMetDocumentInterface->CrossSectionSystem()->CrossSectionViewOn() == false)
        return ;
	if(itsView)
	{
		CtrlView::DeviceContextHandler<CFmiCrossSectionView> deviceContextHandler(this);
		itsView->Update();
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
	return CFmiWin32TemplateHelpers::OnPreparePrintingMapView(this, pInfo, true); // true = ei tueta viel‰ monisivu tulosteita t‰‰ll‰
}

void CFmiCrossSectionView::OnEndPrinting(CDC* pDC, CPrintInfo* pInfo)
{
    itsSmartMetDocumentInterface->Printing(false);

	CView::OnEndPrinting(pDC, pInfo);
}

void CFmiCrossSectionView::DoPrint(void)
{
	// TƒMƒ on armoton kikka vitonen!!!!
	// m_pDocument -pointteriin pit‰‰ laittaa dokumentti pointteri osoittamaan printtauksen ajaksi
	// koska muuten moni sivuisten printtausten tulostus ei onnistu (CView::OnFilePrint vaatii t‰ss‰ dokumenttia k‰yttˆˆn)
	// Yritin tehd‰ ensin omaa printtaus funktiota kopioimalla originaali osion mutta niin ett‰ dokumenttia ei tarvittaisi.
	// T‰m‰ onnistui mutta vain yhdelle paperille kerrallaan. Range print vaatii alkuper‰isen CView::OnFilePrint -metodin 
	// k‰yttˆ‰ ja niin ett‰ m_pDocument -pointteri osoittaa applikaation dokumentti olioon.
	m_pDocument = ApplicationInterface::GetApplicationInterfaceImplementation()->GetDocumentAsCDocument();

	try
	{
		CView::OnFilePrint();
		Invalidate(FALSE); // ruutu pit‰‰ lopuksi p‰ivitt‰‰
	}
	catch(...)
	{
	}
	// Lopuksi pointteri on aina nollattava, muuten luokan destruktori yritt‰‰ tuhota dokumenttia ja se ei k‰y.
	m_pDocument = 0;

}

void CFmiCrossSectionView::OnLButtonUp(UINT nFlags, CPoint point)
{
	try
	{
		CtrlView::DeviceContextHandler<CFmiCrossSectionView> deviceContextHandler(this);
		CtrlView::ReleaseCtrlKeyIfStuck(nFlags); // t‰m‰ vapauttaa CTRL-napin, jos se on 'jumiutunut' pohjaan (MFC bugi, ctrl-nappi voi j‰‰d‰ pohjaan, jos kyseinen n‰pp‰in vapautetaan, ennen kuin kartta ruudun piirto on valmis)

		bool needsUpdate = itsView ? itsView->LeftButtonUp(itsToolBox->ToViewPoint(point.x, point.y)
			,itsToolBox->ConvertCtrlKey(nFlags)) : false;
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
	CtrlView::DeviceContextHandler<CFmiCrossSectionView> deviceContextHandler(this);
	CtrlView::ReleaseCtrlKeyIfStuck(nFlags); // t‰m‰ vapauttaa CTRL-napin, jos se on 'jumiutunut' pohjaan (MFC bugi, ctrl-nappi voi j‰‰d‰ pohjaan, jos kyseinen n‰pp‰in vapautetaan, ennen kuin kartta ruudun piirto on valmis)

	bool needsUpdate = itsView ? itsView->LeftDoubleClick(itsToolBox->ToViewPoint(point.x, point.y)
		,itsToolBox->ConvertCtrlKey(nFlags)) : false;
	if(needsUpdate)
	{
		if(itsSmartMetDocumentInterface->ActivateParamSelectionDlgAfterLeftDoubleClick())
		{
            itsSmartMetDocumentInterface->ActivateParamSelectionDlgAfterLeftDoubleClick(false);
			itsSmartMetDocumentInterface->ActivateParameterSelectionDlg(itsView->MapViewDescTopIndex());
			return ;
		}
		Invalidate(FALSE);
        itsSmartMetDocumentInterface->RefreshApplicationViewsAndDialogs(__FUNCTION__, SmartMetViewId::AllMapViews | SmartMetViewId::CrossSectionView);
	}
//	CView::OnLButtonDblClk(nFlags, point);
}

void CFmiCrossSectionView::OnMButtonUp(UINT nFlags, CPoint point)
{
	CtrlView::DeviceContextHandler<CFmiCrossSectionView> deviceContextHandler(this);
	CtrlView::ReleaseCtrlKeyIfStuck(nFlags); // t‰m‰ vapauttaa CTRL-napin, jos se on 'jumiutunut' pohjaan (MFC bugi, ctrl-nappi voi j‰‰d‰ pohjaan, jos kyseinen n‰pp‰in vapautetaan, ennen kuin kartta ruudun piirto on valmis)

	bool needsUpdate = itsView ? itsView->MiddleButtonUp(itsToolBox->ToViewPoint(point.x, point.y)
		,itsToolBox->ConvertCtrlKey(nFlags)) : false;
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
	CtrlView::DeviceContextHandler<CFmiCrossSectionView> deviceContextHandler(this);
	CtrlView::ReleaseCtrlKeyIfStuck(nFlags); // t‰m‰ vapauttaa CTRL-napin, jos se on 'jumiutunut' pohjaan (MFC bugi, ctrl-nappi voi j‰‰d‰ pohjaan, jos kyseinen n‰pp‰in vapautetaan, ennen kuin kartta ruudun piirto on valmis)

	bool needsUpdate = itsView ? itsView->RightButtonUp(itsToolBox->ToViewPoint(point.x, point.y)
		,itsToolBox->ConvertCtrlKey(nFlags)) : false;
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
	CtrlView::DeviceContextHandler<CFmiCrossSectionView> deviceContextHandler(this);
	CtrlView::ReleaseCtrlKeyIfStuck(nFlags); // t‰m‰ vapauttaa CTRL-napin, jos se on 'jumiutunut' pohjaan (MFC bugi, ctrl-nappi voi j‰‰d‰ pohjaan, jos kyseinen n‰pp‰in vapautetaan, ennen kuin kartta ruudun piirto on valmis)

	bool needsUpdate = itsView ? itsView->LeftButtonDown(itsToolBox->ToViewPoint(point.x, point.y)
		,itsToolBox->ConvertCtrlKey(nFlags)) : false;
	if(needsUpdate)
	{
		fViewDirty = true;
		Invalidate(FALSE);
	}
	SetCapture();
}

void CFmiCrossSectionView::OnMouseMove(UINT nFlags, CPoint point)
{
    if(itsSmartMetDocumentInterface->Printing())
        return;
	isCurrentMousePoint = point;

	CtrlView::DeviceContextHandler<CFmiCrossSectionView> deviceContextHandler(this);
	CtrlView::ReleaseCtrlKeyIfStuck(nFlags); // t‰m‰ vapauttaa CTRL-napin, jos se on 'jumiutunut' pohjaan (MFC bugi, ctrl-nappi voi j‰‰d‰ pohjaan, jos kyseinen n‰pp‰in vapautetaan, ennen kuin kartta ruudun piirto on valmis)

	bool needsUpdate = itsView ? itsView->MouseMove(itsToolBox->ToViewPoint(point.x, point.y)
		,itsToolBox->ConvertCtrlKey(nFlags)) : false;
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
	NFmiPoint clientSizeInPixels(rect.Width(), rect.Height());
    itsSmartMetDocumentInterface->CrossSectionViewSizeInPixels(clientSizeInPixels);
	CtrlView::DeviceContextHandler<CFmiCrossSectionView> deviceContextHandler(this);
	itsSmartMetDocumentInterface->DoMapViewOnSize(MapViewDescTopIndex(), clientSizeInPixels, deviceContextHandler.GetDcFromHandler());

	Update(true);
}

BOOL CFmiCrossSectionView::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	CtrlView::DeviceContextHandler<CFmiCrossSectionView> deviceContextHandler(this);
	CtrlView::ReleaseCtrlKeyIfStuck(nFlags); // t‰m‰ vapauttaa CTRL-napin, jos se on 'jumiutunut' pohjaan (MFC bugi, ctrl-nappi voi j‰‰d‰ pohjaan, jos kyseinen n‰pp‰in vapautetaan, ennen kuin kartta ruudun piirto on valmis)

	// Poiketen "button clik" -metodeista, piste tulee t‰nne absoluuttisena paikkana
	// ja joudun muuttamaan sen suhteelliseksi t‰h‰n ikkunaan ensin.
	CRect screenRect;
	GetWindowRect(screenRect);
	CRect client;
	GetClientRect(client);
	CPoint windowPoint(pt.x - (screenRect.left - client.left), pt.y - (screenRect.top - client.top));

	bool needsUpdate = itsView ? itsView->MouseWheel(itsToolBox->ToViewPoint(windowPoint.x, windowPoint.y)
		,itsToolBox->ConvertCtrlKey(nFlags), zDelta) : false;
	if(needsUpdate)
	{
		Update(true);
		Invalidate(FALSE);
		if(itsSmartMetDocumentInterface->CrossSectionSystem()->CrossSectionSystemActive())
		{ 
            // jos karttan‰ytˆss‰ crosssection moodi p‰‰ll‰, p‰ivitet‰‰n kartta ja muutkin n‰ytˆt
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
    if(!CFmiWin32TemplateHelpers::AllowTooltipDisplay(itsSmartMetDocumentInterface))
        return;

	*result = 0;
	NM_PPTOOLTIP_DISPLAY * pNotify = (NM_PPTOOLTIP_DISPLAY*)pNMHDR;

	if (pNotify->ti->nIDTool == CROSSSECTIOVIEW_TOOLTIP_ID)
	{
		CPoint pt = *pNotify->pt;
		ScreenToClient(&pt);

        CString strU_;

		try
		{
			CtrlView::DeviceContextHandler<CFmiCrossSectionView> deviceContextHandler(this);
			NFmiPoint relativePoint(itsToolBox->ToViewPoint(pt.x, pt.y));
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
	return itsSmartMetDocumentInterface->CurrentTime(0); // haetaan p‰‰karttan‰ytˆn alkuaika
}

int CFmiCrossSectionView::CalcPrintingPageShiftInMinutes(void)
{
	return FmiRound(itsSmartMetDocumentInterface->MapViewDescTop(0)->TimeControlTimeStep() * 60.);
}

CtrlViewUtils::GraphicalInfo& CFmiCrossSectionView::GetGraphicalInfo()
{
	return itsSmartMetDocumentInterface->CrossSectionSystem()->GetGraphicalInfo();
}

TrueMapViewSizeInfo& CFmiCrossSectionView::GetTrueMapViewSizeInfo()
{
	return itsSmartMetDocumentInterface->CrossSectionSystem()->GetTrueMapViewSizeInfo();
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
	return GetTrueMapViewSizeInfo().clientAreaSizeInPixels();
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

void CFmiCrossSectionView::AutoAdjustValueScales(bool /* fJustActive */, bool /* redrawOnSuccess */)
{
	// T‰m‰ ei tee mit‰‰n poikkileikkausn‰ytˆn yhteydess‰.
}
