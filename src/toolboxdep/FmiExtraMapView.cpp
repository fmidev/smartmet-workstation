#include "stdafx.h"
#include "FmiExtraMapView.h"
#include "NFmiDrawParam.h"
#include "NFmiToolBox.h"
#include "NFmiEditMapView.h"
#include "CtrlViewDocumentInterface.h"
#include "SmartMetDocumentInterface.h"
#include "NFmiMapViewDescTop.h"
#include "NFmiGdiPlusImageMapHandler.h"

#include "NFmiMetEditorOptionsData.h"
#include "DialogEx.h"
#include "CFmiMenu.h"
#include "NFmiMenuItemList.h"
#include "FmiWin32Helpers.h"
#include "NFmiRectangle.h"
#include "FmiWin32TemplateHelpers.h"
#include "CtrlViewFunctions.h"
#include "CtrlViewKeyboardFunctions.h"
#include "MapDrawFunctions.h"
#include "ApplicationInterface.h"

// CFmiExtraMapView

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNCREATE(CFmiExtraMapView, CView)

CFmiExtraMapView::CFmiExtraMapView()
:CView()
,itsSmartMetDocumentInterface(nullptr)
,itsFinalMapViewImageBitmap(new CBitmap, BitmapDeleter(true))
,itsMemoryBitmap(new CBitmap, BitmapDeleter(true))
,itsMapBitmap(new CBitmap, BitmapDeleter(true))
,itsSynopPlotBitmap(std::make_unique<CBitmap>())
,itsSynopPlotBitmapHandle(0)
,itsEditMapView(nullptr)
,itsToolBox(nullptr)
,itsDrawingEnvironment()
,itsDrawParam()
,itsMapViewDescTopIndex(0)
,fMouseCursorOnWnd(false)
,fPrintingOnDontSetDcs(false)
{
}

CFmiExtraMapView::~CFmiExtraMapView()
{
	delete itsEditMapView;
	delete itsToolBox;
}

CFmiExtraMapView::CFmiExtraMapView(SmartMetDocumentInterface *smartMetDocumentInterface, unsigned int theMapViewDescTopIndex)
:CView()
,itsSmartMetDocumentInterface(smartMetDocumentInterface)
,itsFinalMapViewImageBitmap(new CBitmap, BitmapDeleter(true))
,itsMemoryBitmap(new CBitmap, BitmapDeleter(true))
,itsMapBitmap(new CBitmap, BitmapDeleter(true))
,itsSynopPlotBitmap(std::make_unique<CBitmap>())
,itsSynopPlotBitmapHandle(0)
,itsEditMapView(nullptr)
,itsToolBox(nullptr)
,itsDrawingEnvironment()
,itsDrawParam(new NFmiDrawParam())
,itsMapViewDescTopIndex(theMapViewDescTopIndex)
,fMouseCursorOnWnd(false)
,fPrintingOnDontSetDcs(false)
{
	// HUOM! Siis resursseissa määritelty bitmap IDB_BITMAP_SYNOP_PLOT_TOOLTIP_BASE määrää kuinka iso synop-plot
	// bitmapista tulee tooltippiin.
	// SYY miksi joudun luomaan bitmapin resursseista on siinä että en saanut win32 ImageList-systeemiä toimimaan muuten.
	// HUOM!2 Perus bitmapin pitää olla neliö, koska synop-plot piirto perustuu siihen!
	itsSynopPlotBitmapHandle = CFmiWin32Helpers::GetBitmapFromResources(IDB_BITMAP_SYNOP_PLOT_TOOLTIP_BASE_EXTRA_MAP);
	itsSynopPlotBitmap->Attach(itsSynopPlotBitmapHandle);
}

BEGIN_MESSAGE_MAP(CFmiExtraMapView, CView)
	ON_WM_SIZE()
	ON_WM_ERASEBKGND()
	ON_WM_MOUSEWHEEL()
	ON_WM_MBUTTONDOWN()
	ON_WM_MBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_RBUTTONUP()
	ON_WM_LBUTTONUP()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_RBUTTONDOWN()
	ON_NOTIFY (UDM_TOOLTIP_DISPLAY, NULL, NotifyDisplayTooltip)
    ON_WM_RBUTTONDBLCLK()
END_MESSAGE_MAP()

// CFmiExtraMapView drawing

void CFmiExtraMapView::OnDraw(CDC* pDC)
{
    CFmiWin32TemplateHelpers::MapViewOnDraw(this, pDC, itsSmartMetDocumentInterface);
}

// CFmiExtraMapView diagnostics

#ifdef _DEBUG
void CFmiExtraMapView::AssertValid() const
{
	CView::AssertValid();
}

void CFmiExtraMapView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}
#endif //_DEBUG


// CFmiExtraMapView message handlers

static const int EXTRAMAPVIEW_TOOLTIP_ID = 1234567;

void CFmiExtraMapView::OnInitialUpdate()
{
	CView::OnInitialUpdate();

	if(itsToolBox)
		delete itsToolBox;
	itsToolBox = new NFmiToolBox(this);
	itsToolBox->UpdateClientRect(); // ilman tätä toolboxilla ei ole kykyä laskea esim. SX, SY metodeja oikein

	CreateEditMapView();
	itsSmartMetDocumentInterface->MapViewDescTop(itsMapViewDescTopIndex)->MapView(this);

	CFmiWin32Helpers::InitializeCPPTooltip(this, m_tooltip, EXTRAMAPVIEW_TOOLTIP_ID);
	CRect winRec;
	GetWindowRect(winRec);
	OnSize(SW_RESTORE, winRec.Width(), winRec.Height()); // jostain syystä on pakko tehdä onsize, että tooltip toimii varmasti koko ikkunan alueella?!?!?
}

void CFmiExtraMapView::CreateEditMapView()
{
	delete itsEditMapView;
	itsEditMapView = new NFmiEditMapView(itsMapViewDescTopIndex, itsToolBox, &itsDrawingEnvironment, itsDrawParam);
}

void CFmiExtraMapView::CurrentPrintTime(const NFmiMetTime &theTime)
{
    itsSmartMetDocumentInterface->CurrentTime(itsMapViewDescTopIndex, theTime);
}

const NFmiRect* CFmiExtraMapView::RelativePrintRect(void) 
{
	return &(itsSmartMetDocumentInterface->MapViewDescTop(itsMapViewDescTopIndex)->RelativeMapRect());
}

CSize CFmiExtraMapView::GetPrintedAreaOnScreenSizeInPixels(void)
{
    auto pixelSize = itsSmartMetDocumentInterface->GetPrintedMapAreaOnScreenSizeInPixels(itsMapViewDescTopIndex);
    return CPoint(static_cast<int>(pixelSize.X()), static_cast<int>(pixelSize.Y()));
}

NFmiPoint CFmiExtraMapView::PrintViewSizeInPixels(void)
{
	return itsSmartMetDocumentInterface->MapViewDescTop(itsMapViewDescTopIndex)->MapViewSizeInPixels();
}

void CFmiExtraMapView::RelativePrintRect(const NFmiRect &theRect) 
{
    itsSmartMetDocumentInterface->MapViewDescTop(itsMapViewDescTopIndex)->RelativeMapRect(theRect);
}

void CFmiExtraMapView::PrintViewSizeInPixels(const NFmiPoint &theSize)
{
    itsSmartMetDocumentInterface->MapViewDescTop(itsMapViewDescTopIndex)->MapViewSizeInPixels(theSize, itsToolBox->GetDC(), true);
}

void CFmiExtraMapView::SetPrintCopyCDC(CDC* pDC)
{
    itsSmartMetDocumentInterface->MapViewDescTop(itsMapViewDescTopIndex)->CopyCDC(pDC);
}

void CFmiExtraMapView::MakePrintViewDirty(bool fViewDirty, bool fCacheDirty)
{
    itsSmartMetDocumentInterface->MapViewDescTop(itsMapViewDescTopIndex)->MapViewDirty(fViewDirty, fCacheDirty, true, false);
}

void CFmiExtraMapView::OnSize(UINT nType, int cx, int cy)
{
    CView::OnSize(nType, cx, cy);

    CRect rect;
    GetClientRect(rect);
    m_tooltip.SetToolRect(this, EXTRAMAPVIEW_TOOLTIP_ID, rect);

	CtrlView::DeviceContextHandler<CFmiExtraMapView> deviceContextHandler(this);
    itsSmartMetDocumentInterface->DoMapViewOnSize(itsMapViewDescTopIndex, NFmiPoint(rect.Width(), rect.Height()), deviceContextHandler.GetDcFromHandler());
    PutTextInStatusBar(CtrlViewUtils::MakeMapPortionPixelSizeStringForStatusbar(itsSmartMetDocumentInterface->MapViewDescTop(itsMapViewDescTopIndex)->ActualMapBitmapSizeInPixels(), true));

    Invalidate(FALSE);
}

CtrlViewUtils::GraphicalInfo& CFmiExtraMapView::GetGraphicalInfo(void)
{
	return itsSmartMetDocumentInterface->MapViewDescTop(itsMapViewDescTopIndex)->GetGraphicalInfo();
}

TrueMapViewSizeInfo& CFmiExtraMapView::GetTrueMapViewSizeInfo()
{
	return itsSmartMetDocumentInterface->MapViewDescTop(itsMapViewDescTopIndex)->GetTrueMapViewSizeInfo();
}

int CFmiExtraMapView::CalcPrintingPageShiftInMinutes(void)
{
	return itsSmartMetDocumentInterface->MapViewDescTop(itsMapViewDescTopIndex)->CalcPrintingPageShiftInMinutes();
}

void CFmiExtraMapView::DoDraw(void)
{
	static bool firstTimeInError = true;
    itsSmartMetDocumentInterface->TransparencyContourDrawView(this); // tämä on osa kikkaa, jolla saadaan piiirettyä läpinäkyviä kenttiä toolmasterilla tai imaginella
	try
	{
		itsEditMapView->Draw(itsToolBox);
	}
	catch(std::exception &e)
	{
		if(firstTimeInError)
		{
			firstTimeInError = false;
			std::string errorTitleStr("Error with extra-map-view drawing");
			std::string errorStr("Error while drawing extra-map-view, there won't be more reports of these:\n");
			errorStr += e.what();
            itsSmartMetDocumentInterface->LogAndWarnUser(errorStr, errorTitleStr, CatLog::Severity::Error, CatLog::Category::Visualization, false);
		}
	}
	catch(...)
	{
		if(firstTimeInError)
		{
			firstTimeInError = false;
			std::string errorTitleStr("Unknown error with extra-map-view drawing");
			std::string errorStr("Unknown error while drawing extra-map-view, there won't be more reports of these.");
            itsSmartMetDocumentInterface->LogAndWarnUser(errorStr, errorTitleStr, CatLog::Severity::Error, CatLog::Category::Visualization, false);
		}
	}
    itsSmartMetDocumentInterface->TransparencyContourDrawView(0); // lopuksi pitää nollata läpinäkyvyys-näyttö pointteri
}

bool CFmiExtraMapView::GenerateMapBitmap(CBitmap *theUsedBitmap, CDC *theUsedCDC, CDC *theCompatibilityCDC)
{
	return MapDraw::GenerateMapBitmap(CtrlViewDocumentInterface::GetCtrlViewDocumentInterfaceImplementation(), itsMapViewDescTopIndex, theUsedBitmap, theUsedCDC, theCompatibilityCDC);
}

// asettaa toolmasterin ja toolboxin DC:t
// esim. ennen piirto tai ennen kuin tarkastellaan hiiren klikkausta ruudulta
// HUOM!! printtauksen yhteydessä kutsu ensin DC:n asetus ja sitten printinfon!!!
void CFmiExtraMapView::SetToolsDCs(CDC* theDC)
{
	SetToolBoxsDC(theDC);
	SetToolMastersDC(theDC);
}

void CFmiExtraMapView::SetToolBoxsDC(CDC* theDC)
{
	if(itsToolBox)
		itsToolBox->SetDC(theDC);
}

// toolmasterin DC:n
void CFmiExtraMapView::SetToolMastersDC(CDC* theDC)
{
    CtrlView::SetToolMastersDC(theDC, this, itsSmartMetDocumentInterface->IsToolMasterAvailable());
}

// tällä piirretään tavara, joka tulee myös bitmapin päälle
void CFmiExtraMapView::DrawOverBitmapThings(NFmiToolBox * theGTB)
{
	if(itsEditMapView)
	{
		if(theGTB)
			itsEditMapView->DrawOverBitmapThings(theGTB, false, 0, 0.f, 0);
		else
		{
			CtrlView::DeviceContextHandler<CFmiExtraMapView> deviceContextHandler(this);
			itsEditMapView->DrawOverBitmapThings(itsToolBox, false, 0, 0.f, 0);
		}
	}
}

BOOL CFmiExtraMapView::OnEraseBkgnd(CDC* /* pDC */ )
{
	return FALSE;	// näin ei tyhjennetä taustaa ennen ruudun piirtoa!!

//	return CView::OnEraseBkgnd(pDC);
}

void CFmiExtraMapView::UpdateMap(void)
{
	if(fPrintingOnDontSetDcs)
		return ; // pitää estää erikseen apukarrtanäytössä kaikenlaiset CDC-asetuksen kesken printtauksen!
	if(itsEditMapView)
	{
		CtrlView::DeviceContextHandler<CFmiExtraMapView> deviceContextHandler(this);
		itsEditMapView->Update();
	}
}

BOOL CFmiExtraMapView::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	if(fPrintingOnDontSetDcs)
		return FALSE; // pitää estää erikseen apukarrtanäytössä kaikenlaiset CDC-asetuksen kesken printtauksen!
	CtrlView::DeviceContextHandler<CFmiExtraMapView> deviceContextHandler(this);
	CtrlView::ReleaseCtrlKeyIfStuck(nFlags); // tämä vapauttaa CTRL-napin, jos se on 'jumiutunut' pohjaan (MFC bugi, ctrl-nappi voi jäädä pohjaan, jos kyseinen näppäin vapautetaan, ennen kuin kartta ruudun piirto on valmis)
		// Jostain syystä MouseWheel systeemi antaa hiiren kursorin paikan
		// absoluuttisen pikseli sijainnin desctopissa. Nyt pitää tehdä muunnos
		// kursorin paikka ikkunan omassa sijainnissa.
	ScreenToClient(&pt);
	bool needsUpdate = itsEditMapView ? itsEditMapView->MouseWheel(itsToolBox->ToViewPoint(pt.x, pt.y)
		, itsToolBox->ConvertCtrlKey(nFlags), zDelta) : false;

	if(needsUpdate)
	{
		itsSmartMetDocumentInterface->RefreshApplicationViewsAndDialogs("Map view 2/3: Mouse wheel action");
	}
	return TRUE;

	//	return CView::OnMouseWheel(nFlags, zDelta, pt);
}

void CFmiExtraMapView::OnMButtonDown(UINT nFlags, CPoint point)
{
	if(fPrintingOnDontSetDcs)
		return ; // pitää estää erikseen apukarrtanäytössä kaikenlaiset CDC-asetuksen kesken printtauksen!
	CtrlView::DeviceContextHandler<CFmiExtraMapView> deviceContextHandler(this);
	bool needsUpdate = itsEditMapView ? itsEditMapView->MiddleButtonDown(itsToolBox->ToViewPoint(point.x, point.y)
		,itsToolBox->ConvertCtrlKey(nFlags)) : false;

	if(needsUpdate)
	{
        itsSmartMetDocumentInterface->RefreshApplicationViewsAndDialogs("Map view 2/3: Mouse middle button down");
	}
    SetCapture(); // otetaan hiiren liikkeet/viestit talteeen toistaiseksi tähän ikkunaan
}

void CFmiExtraMapView::OnMButtonUp(UINT nFlags, CPoint point)
{
    try
    {
        if(fPrintingOnDontSetDcs)
            return; // pitää estää erikseen apukarrtanäytössä kaikenlaiset CDC-asetuksen kesken printtauksen!
		CtrlView::DeviceContextHandler<CFmiExtraMapView> deviceContextHandler(this);
		CtrlView::ReleaseCtrlKeyIfStuck(nFlags); // tämä vapauttaa CTRL-napin, jos se on 'jumiutunut' pohjaan (MFC bugi, ctrl-nappi voi jäädä pohjaan, jos kyseinen näppäin vapautetaan, ennen kuin kartta ruudun piirto on valmis)

        bool needsUpdate = itsEditMapView ? itsEditMapView->MiddleButtonUp(itsToolBox->ToViewPoint(point.x, point.y)
            , itsToolBox->ConvertCtrlKey(nFlags)) : false;
        Invalidate(FALSE);
        if(needsUpdate)
        {
            itsSmartMetDocumentInterface->RefreshApplicationViewsAndDialogs("Map view 2/3: Mouse middle button up");
        }
    }
    catch(...)
    {
        ReleaseCapture(); // vapautetaan lopuksi hiiren viestit muidenkin ikkunoiden käyttöön
        throw; // laitetaan poikkeus eteenpäin
    }
    ReleaseCapture(); // vapautetaan lopuksi hiiren viestit muidenkin ikkunoiden käyttöön (OnLButtonDown:issa laitettiin SetCapture päälle)
}


void CFmiExtraMapView::PutTextInStatusBar(const std::string &theText)
{
    CStatusBar* pStatus = ((CDialogEx2*)GetParent())->GetStatusBar();
    if(pStatus && pStatus->GetSafeHwnd())
        pStatus->SetPaneText(0, CA2T(theText.c_str()));
}

void CFmiExtraMapView::OnMouseMove(UINT nFlags, CPoint point)
{
	if(fPrintingOnDontSetDcs)
		return ; // pitää estää erikseen apukarrtanäytössä kaikenlaiset CDC-asetuksen kesken printtauksen!
	if(itsEditMapView == 0)
		return ;

	CtrlView::DeviceContextHandler<CFmiExtraMapView> deviceContextHandler(this);
	CtrlView::ReleaseCtrlKeyIfStuck(nFlags); // tämä vapauttaa CTRL-napin, jos se on 'jumiutunut' pohjaan (MFC bugi, ctrl-nappi voi jäädä pohjaan, jos kyseinen näppäin vapautetaan, ennen kuin kartta ruudun piirto on valmis)

// laitetaan myös karttabitmap valmiiksi osittaisia päivityksiä varten
	CClientDC dc(this);
	CDC dcMem;
	dcMem.CreateCompatibleDC(&dc);
	CBitmap* oldBitmap2 = dcMem.SelectObject(itsMapBitmap.get());
    auto mapViewDescTop = itsSmartMetDocumentInterface->MapViewDescTop(itsMapViewDescTopIndex);
    mapViewDescTop->MapBlitDC(&dcMem);

	NFmiPoint viewPoint(itsToolBox->ToViewPoint(point.x, point.y));

	bool needsUpdate = itsEditMapView ? itsEditMapView->MouseMove(viewPoint, itsToolBox->ConvertCtrlKey(nFlags)) : false;
    mapViewDescTop->MapBlitDC(0);
	dcMem.SelectObject(oldBitmap2);
	dcMem.DeleteDC();

	bool drawOverBitmapAnyway = itsSmartMetDocumentInterface->EditedPointsSelectionChanged(false); // pyydetään vanha arvo drawOverBitmapAnyway -muuttujaan ja asetetaan samalla false:ksi
	if(itsEditMapView->MapRect().IsInside(viewPoint))
	{
		NFmiPoint cursorLatlon = itsSmartMetDocumentInterface->ToolTipLatLonPoint();
		std::string str = CtrlViewUtils::GetTotalMapViewStatusBarStr(CtrlViewDocumentInterface::GetCtrlViewDocumentInterfaceImplementation(), cursorLatlon);
        // Laitetaan kartta osion pikseli koko tähän perään sulkuihin
        str += CtrlViewUtils::MakeMapPortionPixelSizeStringForStatusbar(mapViewDescTop->ActualMapBitmapSizeInPixels(), false);
        PutTextInStatusBar(str);
	}

	if(needsUpdate)
	{
        itsSmartMetDocumentInterface->RefreshApplicationViewsAndDialogs("Map view 2/3: Mouse move action");
	}
	else if(mapViewDescTop->MapViewBitmapDirty())
		Invalidate(FALSE);
	else if(!(itsSmartMetDocumentInterface->MiddleMouseButtonDown() && itsSmartMetDocumentInterface->MouseCaptured())) // muuten ForceDrawOverBitmapThings, paitsi jos ollaan vetämässä kartan päälle zoomi laatikkoa, koska se peittyisi aina ForceDrawOverBitmapThings:n alle
	{
		if(itsSmartMetDocumentInterface->ShowMouseHelpCursorsOnMap() || drawOverBitmapAnyway)
		{
            itsSmartMetDocumentInterface->ForceDrawOverBitmapThings(itsMapViewDescTopIndex, true, true); // hiiren apukursorit pitää joka tapauksessa piirtää aina ja joka karttanäyttöön
		}
		if(itsSmartMetDocumentInterface->MustDrawTempView())
		{
            itsSmartMetDocumentInterface->MustDrawTempView(false);
            itsSmartMetDocumentInterface->UpdateTempView();
		}
		if(itsSmartMetDocumentInterface->MustDrawCrossSectionView())
		{
            itsSmartMetDocumentInterface->MustDrawCrossSectionView(false);
            itsSmartMetDocumentInterface->UpdateCrossSectionView();
            itsSmartMetDocumentInterface->ForceDrawOverBitmapThings(0, true, false); // Päivitetään vain pääikkunaa, koska sinne piirretään poikkileikkaus jutut
        }
	}

//	CView::OnMouseMove(nFlags, point);
}

// pakotetaan piirtämään bitblitillä bitmap cache karttanäytön päälle
// ja sitten päälle piirretään nopeasti DrawOverBitmapThings
void CFmiExtraMapView::ForceDrawOverBitmapThingsThisExtraMapView(void)
{
    itsSmartMetDocumentInterface->MapViewDescTop(itsMapViewDescTopIndex)->MapViewBitmapDirty(true);
    Invalidate(FALSE);
}

void CFmiExtraMapView::OnRButtonUp(UINT nFlags, CPoint point)
{
	if(fPrintingOnDontSetDcs)
		return ; // pitää estää erikseen apukarrtanäytössä kaikenlaiset CDC-asetuksen kesken printtauksen!
	CtrlView::DeviceContextHandler<CFmiExtraMapView> deviceContextHandler(this);
	CtrlView::ReleaseCtrlKeyIfStuck(nFlags); // tämä vapauttaa CTRL-napin, jos se on 'jumiutunut' pohjaan (MFC bugi, ctrl-nappi voi jäädä pohjaan, jos kyseinen näppäin vapautetaan, ennen kuin kartta ruudun piirto on valmis)

	bool needsUpdate = itsEditMapView ? itsEditMapView->RightButtonUp(itsToolBox->ToViewPoint(point.x, point.y)
		,itsToolBox->ConvertCtrlKey(nFlags)) : false;

	Invalidate(FALSE);
	if(itsSmartMetDocumentInterface->OpenPopupMenu())
	{
		CFmiMenu menu;
		menu.Init(itsSmartMetDocumentInterface->PopupMenu());
		CMenu* pPopup = menu.Popup();

		CRect tempRect;
		GetWindowRect(tempRect);
		pPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x+tempRect.left, point.y+tempRect.top, this);
        itsSmartMetDocumentInterface->OpenPopupMenu(false);
	}
	else if(needsUpdate)
	{
        itsSmartMetDocumentInterface->RefreshApplicationViewsAndDialogs("Map view 2/3: Mouse right button up");
        itsSmartMetDocumentInterface->ForceDrawOverBitmapThings(itsMapViewDescTopIndex, false, true);
	}

//	CView::OnRButtonUp(nFlags, point);
}

BOOL CFmiExtraMapView::OnCommand(WPARAM wParam, LPARAM lParam)
{
	if(itsSmartMetDocumentInterface->PopupMenu())
	{
		NFmiMenuItemList* menuItemList = itsSmartMetDocumentInterface->PopupMenu();
		unsigned short nID = short (wParam);
		if (nID >= menuItemList->MinId() && nID <= menuItemList->MaxId())
		{
			if(itsSmartMetDocumentInterface->MakePopUpCommandUsingRowIndex(nID))
			{
                itsSmartMetDocumentInterface->RefreshApplicationViewsAndDialogs("Map view 2/3: Making popup command");
				return TRUE;
			}
		}
	}

	return CView::OnCommand(wParam, lParam);
}

void CFmiExtraMapView::OnLButtonUp(UINT nFlags, CPoint point)
{
	if(fPrintingOnDontSetDcs)
		return ; // pitää estää erikseen apukarrtanäytössä kaikenlaiset CDC-asetuksen kesken printtauksen!
	try
	{
		CtrlView::DeviceContextHandler<CFmiExtraMapView> deviceContextHandler(this);
		CtrlView::ReleaseCtrlKeyIfStuck(nFlags); // tämä vapauttaa CTRL-napin, jos se on 'jumiutunut' pohjaan (MFC bugi, ctrl-nappi voi jäädä pohjaan, jos kyseinen näppäin vapautetaan, ennen kuin kartta ruudun piirto on valmis)

		bool needsUpdate = itsEditMapView ? itsEditMapView->LeftButtonUp(itsToolBox->ToViewPoint(point.x, point.y)
			,itsToolBox->ConvertCtrlKey(nFlags)) : false;	// M.K. 29.4.99 Lisäsin "parametrivalintalaatikon" piirtämistä varten.
		Invalidate(FALSE);
		if(needsUpdate)
		{
            itsSmartMetDocumentInterface->RefreshApplicationViewsAndDialogs("Map view 2/3: Mouse left button up");
            itsSmartMetDocumentInterface->ForceDrawOverBitmapThings(itsMapViewDescTopIndex, false, true);
		}

	//	CView::OnLButtonUp(nFlags, point);
	}
	catch(...)
	{
		ReleaseCapture(); // vapautetaan lopuksi hiiren viestit muidenkin ikkunoiden käyttöön
		throw ; // laitetaan poikkeus eteenpäin
	}
	ReleaseCapture(); // vapautetaan lopuksi hiiren viestit muidenkin ikkunoiden käyttöön (OnLButtonDown:issa laitettiin SetCapture päälle)
}

void CFmiExtraMapView::OnLButtonDown(UINT nFlags, CPoint point)
{
	if(fPrintingOnDontSetDcs)
		return ; // pitää estää erikseen apukarrtanäytössä kaikenlaiset CDC-asetuksen kesken printtauksen!
	CtrlView::DeviceContextHandler<CFmiExtraMapView> deviceContextHandler(this);
	bool needsUpdate = itsEditMapView ? itsEditMapView->LeftButtonDown(itsToolBox->ToViewPoint(point.x, point.y)
		,itsToolBox->ConvertCtrlKey(nFlags)) : false;

	if(needsUpdate)
	{
        itsSmartMetDocumentInterface->RefreshApplicationViewsAndDialogs("Map view 2/3: Mouse left button down");
	}
	SetCapture(); // otetaan hiiren liikkeet/viestit talteeen toistaiseksi tähän ikkunaan
}

void CFmiExtraMapView::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	if(fPrintingOnDontSetDcs)
		return ; // pitää estää erikseen apukarrtanäytössä kaikenlaiset CDC-asetuksen kesken printtauksen!
	CtrlView::DeviceContextHandler<CFmiExtraMapView> deviceContextHandler(this);
	CtrlView::ReleaseCtrlKeyIfStuck(nFlags); // tämä vapauttaa CTRL-napin, jos se on 'jumiutunut' pohjaan (MFC bugi, ctrl-nappi voi jäädä pohjaan, jos kyseinen näppäin vapautetaan, ennen kuin kartta ruudun piirto on valmis)

	bool needsUpdate = itsEditMapView ? itsEditMapView->LeftDoubleClick(itsToolBox->ToViewPoint(point.x, point.y)
		,itsToolBox->ConvertCtrlKey(nFlags)) : false;
	if(needsUpdate)
	{
		if(itsSmartMetDocumentInterface->ActivateParamSelectionDlgAfterLeftDoubleClick())
		{
            itsSmartMetDocumentInterface->ActivateParamSelectionDlgAfterLeftDoubleClick(false);
            itsSmartMetDocumentInterface->ActivateParameterSelectionDlg(itsMapViewDescTopIndex);
			return ;
		}
		Invalidate(FALSE);
        itsSmartMetDocumentInterface->RefreshApplicationViewsAndDialogs("Map view 2/3: Mouse left button double click");
	}
//	CView::OnLButtonDblClk(nFlags, point);
}

void CFmiExtraMapView::OnRButtonDblClk(UINT nFlags, CPoint point)
{
	if(fPrintingOnDontSetDcs)
		return ; // pitää estää erikseen apukarrtanäytössä kaikenlaiset CDC-asetuksen kesken printtauksen!
	CtrlView::DeviceContextHandler<CFmiExtraMapView> deviceContextHandler(this);
	CtrlView::ReleaseCtrlKeyIfStuck(nFlags); // tämä vapauttaa CTRL-napin, jos se on 'jumiutunut' pohjaan (MFC bugi, ctrl-nappi voi jäädä pohjaan, jos kyseinen näppäin vapautetaan, ennen kuin kartta ruudun piirto on valmis)

	bool needsUpdate = itsEditMapView ? itsEditMapView->RightDoubleClick(itsToolBox->ToViewPoint(point.x, point.y)
		,itsToolBox->ConvertCtrlKey(nFlags)) : false;
	if(needsUpdate)
	{
		Invalidate(FALSE);
        itsSmartMetDocumentInterface->RefreshApplicationViewsAndDialogs("Map view 2/3: Mouse right button double click");
	}
//    CView::OnRButtonDblClk(nFlags, point);
}

void CFmiExtraMapView::OnRButtonDown(UINT nFlags, CPoint point)
{
	if(fPrintingOnDontSetDcs)
		return ; // pitää estää erikseen apukarrtanäytössä kaikenlaiset CDC-asetuksen kesken printtauksen!
	CtrlView::DeviceContextHandler<CFmiExtraMapView> deviceContextHandler(this);
	bool needsUpdate = itsEditMapView ? itsEditMapView->RightButtonDown(itsToolBox->ToViewPoint(point.x, point.y)
		,itsToolBox->ConvertCtrlKey(nFlags)) : false;

	if(needsUpdate)
	{
        itsSmartMetDocumentInterface->RefreshApplicationViewsAndDialogs("Map view 2/3: Mouse right button down");
	}
}

void CFmiExtraMapView::DoPrint(void)
{
	// TÄMÄ on armoton kikka vitonen!!!!
	// m_pDocument -pointteriin pitää laittaa dokumentti pointteri osoittamaan printtauksen ajaksi
	// koska muuten moni sivuisten printtausten tulostus ei onnistu (CView::OnFilePrint vaatii tässä dokumenttia käyttöön)
	// Yritin tehdä ensin omaa printtaus funktiota kopioimalla originaali osion mutta niin että dokumenttia ei tarvittaisi.
	// Tämä onnistui mutta vain yhdelle paperille kerrallaan. Range print vaatii alkuperäisen CView::OnFilePrint -metodin 
	// käyttöä ja niin että m_pDocument -pointteri osoittaa applikaation dokumentti olioon.
	m_pDocument = ApplicationInterface::GetApplicationInterfaceImplementation()->GetDocumentAsCDocument();
	fPrintingOnDontSetDcs = true;

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
	fPrintingOnDontSetDcs = false;

}

void CFmiExtraMapView::OnPrepareDC(CDC* pDC, CPrintInfo* pInfo)
{
	CFmiWin32TemplateHelpers::OnPrepareDCMapView(this, pDC, pInfo);

	CView::OnPrepareDC(pDC, pInfo);
}

void CFmiExtraMapView::OnPrint(CDC* pDC, CPrintInfo* pInfo)
{
	CFmiWin32TemplateHelpers::OnPrintMapView(this, pDC, pInfo);
}

BOOL CFmiExtraMapView::OnPreparePrinting(CPrintInfo* pInfo)
{
	return CFmiWin32TemplateHelpers::OnPreparePrintingMapView(this, pInfo, false);
}

void CFmiExtraMapView::OnEndPrinting(CDC* pDC, CPrintInfo* pInfo)
{
    itsSmartMetDocumentInterface->Printing(false);

	CView::OnEndPrinting(pDC, pInfo);
}

BOOL CFmiExtraMapView::PreTranslateMessage(MSG* pMsg)
{
	if(itsSmartMetDocumentInterface->MetEditorOptionsData().ShowToolTipsOnMapView())
		m_tooltip.RelayEvent(pMsg);

	return CView::PreTranslateMessage(pMsg);
}

std::string::size_type CFmiExtraMapView::FindPlotImagePosition(const std::string &theTootipStr, std::string &theFoundPlotStr, bool &drawSoundingPlot, bool &drawMinMaxPlot, bool &drawMetarPlot)
{
	std::string tmpStr = theTootipStr;
	std::string sought = "#synop-plot-image#"; // etsitään josko löytyy synop-plot place holderia tooltip stringistä
	std::string::size_type pos = tmpStr.find(sought);
	if(pos == std::string::npos)
	{ // katsotaan jos löytyy sitten sounding-plot
		std::string sought2 = "#sounding-plot-image#"; // etsitään josko löytyy sounding-plot place holderia tooltip stringistä
		std::string::size_type pos2 = tmpStr.find(sought2);
		if(pos2 != std::string::npos)
		{
			sought = sought2;
			pos = pos2;
			drawSoundingPlot = true;
		}
		else
		{
			// katsotaan jos löytyy sitten minmax-plot
			std::string sought3 = "#minmax-plot-image#"; // etsitään josko löytyy minmax-plot place holderia tooltip stringistä
			std::string::size_type pos3 = tmpStr.find(sought3);
			if(pos3 != std::string::npos)
			{
				sought = sought3;
				pos = pos3;
				drawMinMaxPlot = true;
			}
			else
			{
				// katsotaan jos löytyy sitten metar-plot
				std::string sought4 = "#metar-plot-image#"; // etsitään josko löytyy metar-plot place holderia tooltip stringistä
				std::string::size_type pos4 = tmpStr.find(sought4);
				if(pos4 != std::string::npos)
				{
					sought = sought4;
					pos = pos4;
					drawMetarPlot = true;
				}
			}
		}
	}
	theFoundPlotStr = sought;
	return pos;
}


void CFmiExtraMapView::NotifyDisplayTooltip(NMHDR * pNMHDR, LRESULT * result)
{
    CFmiWin32TemplateHelpers::NotifyDisplayTooltip(this, pNMHDR, result, GetSmartMetDocumentInterface(), EXTRAMAPVIEW_TOOLTIP_ID);
}

// tätä kutsutaan yleisessä printtaus funktiossa
void CFmiExtraMapView::OldWayPrintUpdate(void)
{
    itsSmartMetDocumentInterface->MapViewDescTop(itsMapViewDescTopIndex)->SetBorderDrawDirtyState(CountryBorderDrawDirtyState::Geometry);
	itsEditMapView->Update(); // tämä pitää tehdä että prionttauksen aikaiset mapAreat ja systeemit tulevat voimaan
}

NFmiMetTime CFmiExtraMapView::CalcPrintingStartTime(void)
{
	return itsSmartMetDocumentInterface->CurrentTime(itsMapViewDescTopIndex);
}

NFmiStationViewHandler* CFmiExtraMapView::GetMapViewHandler(int theRowIndex, int theIndex)
{
    if(itsEditMapView)
        return itsEditMapView->GetMapViewHandler(theRowIndex, theIndex);
    else
        return 0;
}
