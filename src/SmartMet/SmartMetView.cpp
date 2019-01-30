
// SmartMetView.cpp : implementation of the CSmartMetView class
//
#ifdef _MSC_VER
#pragma warning(disable : 4786) // poistaa n kpl VC++ kääntäjän varoitusta (liian pitkä nimi >255 merkkiä joka johtuu 'puretuista' STL-template nimistä)
#endif

#include "stdafx.h"
#include "SmartMet.h"
#include "SmartMetDoc.h"
#include "SmartMetView.h"
#include "MainFrm.h"
#include "winuser.h"

#include "NFmiEditMapView.h"
#include "NFmiToolBox.h"
#include "NFmiEditmapGeneralDataDoc.h"
#include "NFmiQueryData.h"
#include "NFmiStationView.h"
#include "NFmiDrawParam.h"
#include "NFmiFastQueryInfo.h"
#include "NFmiGrid.h"
#include "TableWnd.h"
#include "PPToolTip.h"

#include "CFmiPopupMenu.h"
#include "NFmiMenuItemList.h"
#include "NFmiValueString.h"
#include "NFmiInfoOrganizer.h"
#include "NFmiMilliSecondTimer.h"
#include "FmiSynopDataGridViewDlg.h"
#include "NFmiMetEditorOptionsData.h"
#include "NFmiMapViewDescTop.h"
#include "NFmiRectangle.h"

#include "FmiExtraMapView.h"
#include "FmiWin32Helpers.h"
#include "FmiWin32TemplateHelpers.h"
#include "NFmiStationViewHandler.h"
#include "CtrlViewFunctions.h"
#include "CtrlViewKeyboardFunctions.h"
#include "MapDrawFunctions.h"
#include "SmartMetDocumentInterface.h"
#include "ApplicationInterface.h"
#include "CtrlViewWin32Functions.h"
#include "NFmiGdiPlusImageMapHandler.h"

#include <Winspool.h>

extern CSmartMetApp theApp;

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CSmartMetView

IMPLEMENT_DYNCREATE(CSmartMetView, CView)

BEGIN_MESSAGE_MAP(CSmartMetView, CView)
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, &CSmartMetView::OnFilePrintPreview)

	ON_WM_LBUTTONUP()
	ON_WM_RBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDOWN()
	ON_WM_SIZE()
	ON_WM_RBUTTONDOWN()
	ON_WM_SETCURSOR()
	ON_WM_KEYUP()
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, OnFilePrintPreview)
	ON_WM_DROPFILES()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_ERASEBKGND()
	ON_WM_MBUTTONUP()
	ON_WM_MOUSEWHEEL()
	ON_WM_MBUTTONDOWN()
	ON_COMMAND(ID_MENUITEM_VIEW_GRID_SELECTION_DLG, OnMenuitemViewGridSelectionDlg)
	ON_NOTIFY (UDM_TOOLTIP_DISPLAY, NULL, NotifyDisplayTooltip)
	ON_COMMAND(ID_ACCELERATOR_SWAP_AREA, OnAcceleratorSwapArea)
	ON_COMMAND(ID_ACCELERATOR_MAKE_SWAP_BASE_AREA, OnAcceleratorMakeSwapBaseArea)
	ON_COMMAND(ID_EDIT_COPY, &CSmartMetView::OnEditCopy)
	ON_COMMAND(ID_EDIT_PASTE, &CSmartMetView::OnEditPaste)
	ON_COMMAND(ID_ACCELERATOR_COPY_ALL_MAP_VIEW_PARAMS, &CSmartMetView::OnAcceleratorCopyAllMapViewParams)
	ON_COMMAND(ID_ACCELERATOR_PASTE_ALL_MAP_VIEW_PARAMS, &CSmartMetView::OnAcceleratorPasteAllMapViewParams)
	ON_COMMAND(ID_ACCELERATOR_TOGGLE_ANIMATION_VIEW, &CSmartMetView::OnAcceleratorToggleAnimationView)
	ON_COMMAND(ID_ACCELERATOR_SET_HOME_TIME, &CSmartMetView::OnAcceleratorSetHomeTime)
	ON_COMMAND(ID_ACCELERATOR_CHANGE_TIME_BY_DAY_FORWARD, &CSmartMetView::OnAcceleratorChangeTimeByDayForward)
	ON_COMMAND(ID_ACCELERATOR_CHANGE_TIME_BY_DAY_BACKWARD, &CSmartMetView::OnAcceleratorChangeTimeByDayBackward)
	ON_COMMAND(ID_ACCELERATOR_CHANGE_TIME_BY_WEEK_FORWARD, &CSmartMetView::OnAcceleratorChangeTimeByWeekForward)
	ON_COMMAND(ID_ACCELERATOR_CHANGE_TIME_BY_WEEK_BACKWARD, &CSmartMetView::OnAcceleratorChangeTimeByWeekBackward)
	ON_COMMAND(ID_ACCELERATOR_CHANGE_TIME_BY_MONTH_FORWARD, &CSmartMetView::OnAcceleratorChangeTimeByMonthForward)
	ON_COMMAND(ID_ACCELERATOR_CHANGE_TIME_BY_MONTH_BACKWARD, &CSmartMetView::OnAcceleratorChangeTimeByMonthBackward)
	ON_COMMAND(ID_ACCELERATOR_CHANGE_TIME_BY_YEAR_FORWARD, &CSmartMetView::OnAcceleratorChangeTimeByYearForward)
	ON_COMMAND(ID_ACCELERATOR_CHANGE_TIME_BY_YEAR_BACKWARD, &CSmartMetView::OnAcceleratorChangeTimeByYearBackward)
	ON_UPDATE_COMMAND_UI(ID_BUTTON_DATA_TO_DATABASE, &CSmartMetView::OnUpdateButtonDataToDatabase)
    ON_COMMAND(ID_ACCELERATOR_MAP_ZOOM_IN, &CSmartMetView::OnAcceleratorMapZoomIn)
    ON_COMMAND(ID_ACCELERATOR_MAP_ZOOM_OUT, &CSmartMetView::OnAcceleratorMapZoomOut)
    ON_COMMAND(ID_ACCELERATOR_MAP_PAN_DOWN, &CSmartMetView::OnAcceleratorMapPanDown)
    ON_COMMAND(ID_ACCELERATOR_MAP_PAN_LEFT, &CSmartMetView::OnAcceleratorMapPanLeft)
    ON_COMMAND(ID_ACCELERATOR_MAP_PAN_RIGHT, &CSmartMetView::OnAcceleratorMapPanRight)
    ON_COMMAND(ID_ACCELERATOR_MAP_PAN_UP, &CSmartMetView::OnAcceleratorMapPanUp)
    ON_WM_RBUTTONDBLCLK()
END_MESSAGE_MAP()

// CSmartMetView construction/destruction

CSmartMetView::CSmartMetView()
:itsFinalMapViewImageBitmap(new CBitmap)
,itsMemoryBitmap(new CBitmap)
,itsMapBitmap(new CBitmap)
,itsOverMapBitmap(new CBitmap)
,itsEditMapView(0)
,itsToolBox(0)
,itsDrawingEnvironment()
,itsDrawParam(new NFmiDrawParam())
,itsMapViewDescTopIndex(0)
,fMouseCursorOnWnd(false)
,fDoSpecialPrinting(false)
,itsSynopPlotBitmap(0)
,itsSynopPlotBitmapHandle(0)
,m_SizeWnd(new CTableWnd(CtrlViewUtils::MaxViewGridXSize, CtrlViewUtils::MaxViewGridYSize, CtrlViewUtils::MaxViewGridXSize, CtrlViewUtils::MaxViewGridYSize))
,m_tooltip(new CPPToolTip())
{
	EnableActiveAccessibility();
	// TODO: add construction code here
	// HUOM! Siis resursseissa määritelty bitmap IDB_BITMAP_SYNOP_PLOT_TOOLTIP_BASE määrää kuinka iso synop-plot
	// bitmapista tulee tooltippiin.
	// SYY miksi joudun luomaan bitmapin resursseista on siinä että en saanut win32 ImageList-systeemiä toimimaan muuten.
	// HUOM!2 Perus bitmapin pitää olla neliö, koska synop-plot piirto perustuu siihen!
	itsSynopPlotBitmapHandle = CFmiWin32Helpers::GetBitmapFromResources(IDB_BITMAP_SYNOP_PLOT_TOOLTIP_BASE);
	itsSynopPlotBitmap = new CBitmap;
	itsSynopPlotBitmap->Attach(itsSynopPlotBitmapHandle);

}

CSmartMetView::~CSmartMetView()
{
	delete itsEditMapView;
	delete itsToolBox;
    CtrlView::DestroyBitmap(&itsFinalMapViewImageBitmap);
    CtrlView::DestroyBitmap(&itsMemoryBitmap);
    CtrlView::DestroyBitmap(&itsMapBitmap);
    CtrlView::DestroyBitmap(&itsOverMapBitmap);
	delete itsSynopPlotBitmap; // tähän ei saa käyttää DestroyBitmap-funktiota
	delete m_SizeWnd;
	delete m_tooltip;
}

BOOL CSmartMetView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CView::PreCreateWindow(cs);
}

// CSmartMetView drawing

void CSmartMetView::OnDraw(CDC* pDC)
{
    CFmiWin32TemplateHelpers::MapViewOnDraw(this, pDC, GetSmartMetDocumentInterface());
}

void CSmartMetView::DoDraw()
{
	static bool firstTimeInError = true;
	NFmiEditMapGeneralDataDoc *genDoc = GetDocument()->GetData();
	genDoc->TransparencyContourDrawView(this); // tämä on osa kikkaa, jolla saadaan piiirettyä läpinäkyviä kenttiä toolmasterilla tai imaginella
	try
	{
		itsEditMapView->Draw(itsToolBox);
	}
	catch(std::exception &e)
	{
		if(firstTimeInError)
		{
			firstTimeInError = false;
			std::string errorTitleStr("Error with main-map-view drawing");
			std::string errorStr("Error while drawing main-map-view, there won't be more reports of these:\n");
			errorStr += e.what();
			genDoc->LogAndWarnUser(errorStr, errorTitleStr, CatLog::Severity::Error, CatLog::Category::Visualization, false);
		}
	}
	catch(...)
	{
		if(firstTimeInError)
		{
			firstTimeInError = false;
			std::string errorTitleStr("Unknown error with main-map-view drawing");
			std::string errorStr("Unknown error while drawing main-map-view, there won't be more reports of these.");
			genDoc->LogAndWarnUser(errorStr, errorTitleStr, CatLog::Severity::Error, CatLog::Category::Visualization, false);
		}
	}
	genDoc->TransparencyContourDrawView(0); // lopuksi pitää nollata läpinäkyvyys-näyttö pointteri
}

// tällä piirretään tavara, joka tulee myös bitmapin päälle
void CSmartMetView::DrawOverBitmapThings(NFmiToolBox * theGTB)
{
	if(theGTB)
		itsEditMapView->DrawOverBitmapThings(theGTB, false, 0, 0.f, 0);
	else
	{
		CDC *theDC = GetDC();
		if(!theDC)
			return;
		SetToolsDCs(theDC) ;
		itsEditMapView->DrawOverBitmapThings(itsToolBox, false, 0, 0.f, 0);

		ReleaseDC(theDC);
	}
}

bool CSmartMetView::GenerateMapBitmap(CBitmap *theUsedBitmap, CDC *theUsedCDC, CDC *theCompatibilityCDC)
{
	NFmiEditMapGeneralDataDoc *data = GetDocument()->GetData();
	return MapDraw::GenerateMapBitmap(&data->GetCtrlViewDocumentInterface(), itsMapViewDescTopIndex, theUsedBitmap, theUsedCDC, theCompatibilityCDC);
}

void CSmartMetView::DoGraphReportOnDraw(const CtrlViewUtils::GraphicalInfo &graphicalInfo, double scaleFactor)
{
    static bool graphInfoReported = false;
    if(graphInfoReported == false)
    { // vain 1. kerran tehdään lokiin raportti
        graphInfoReported = true;
        std::stringstream sstream;
        sstream << "\nViewWidthInMM: " << graphicalInfo.itsViewWidthInMM << std::endl;
        sstream << "ViewHeightInMM: " << graphicalInfo.itsViewHeightInMM << std::endl;
        sstream << "ScreenWidthInMM: " << graphicalInfo.itsScreenWidthInMM << std::endl;
        sstream << "ScreenHeightInMM: " << graphicalInfo.itsScreenHeightInMM << std::endl;
        sstream << "ScreenWidthInPixels: " << graphicalInfo.itsScreenWidthInPixels << std::endl;
        sstream << "ScreenHeightInPixels: " << graphicalInfo.itsScreenHeightInPixels << std::endl;
        sstream << "DPI x: " << graphicalInfo.itsDpiX << std::endl;
        sstream << "DPI y: " << graphicalInfo.itsDpiY << std::endl;
        sstream << "DrawObjectScaleFactor (editor.conf - MetEditor::DrawObjectScaleFactor ): " << scaleFactor << std::endl;
        if(scaleFactor == 0)
            sstream << "Using ScreenWidthInMM and ScreenHeightInMM to calculate pixels-per-mm values" << std::endl;
        else
            sstream << "Using DPI x and y and DrawObjectScaleFactor to calculate pixels-per-mm values" << std::endl;
        sstream << "PixelsPerMM_x: " << graphicalInfo.itsPixelsPerMM_x << std::endl;
        sstream << "PixelsPerMM_y: " << graphicalInfo.itsPixelsPerMM_y << std::endl;
        CatLog::logMessage(sstream.str(), CatLog::Severity::Debug, CatLog::Category::Visualization);
    }
}

// CSmartMetView printing


void CSmartMetView::OnFilePrintPreview()
{
#ifdef FMI_DISABLE_MFC_FEATURE_PACK
	// TODO: Add your command handler code here
	CView::OnFilePrintPreview();
#else
	AFXPrintPreview(this);
#endif // FMI_DISABLE_MFC_FEATURE_PACK
}

void CSmartMetView::OnPrint(CDC* pDC, CPrintInfo* pInfo)
{
	CFmiWin32TemplateHelpers::OnPrintMapView(this, pDC, pInfo);
}

BOOL CSmartMetView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// default preparation
//	return DoPreparePrinting(pInfo);
	return CFmiWin32TemplateHelpers::OnPreparePrintingMapView(this, pInfo, false);
}

void CSmartMetView::OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo)
{
	CView::OnBeginPrinting(pDC, pInfo);
}

void CSmartMetView::OnEndPrinting(CDC* pDC, CPrintInfo* pInfo)
{
	GetGeneralDoc()->Printing(false);

	CView::OnEndPrinting(pDC, pInfo);
}

void CSmartMetView::OnFilePrint()
{
	CView::OnFilePrint();
	Invalidate(FALSE); // ruutu pitää lopuksi päivittää
}

// tätä kutsutaan yleisessä printtaus funktiossa
void CSmartMetView::OldWayPrintUpdate(void)
{
	GetGeneralDoc()->MapViewDescTop(itsMapViewDescTopIndex)->BorderDrawDirty(true);
	itsEditMapView->Update(); // tämä pitää tehdä että prionttauksen aikaiset mapAreat ja systeemit tulevat voimaan
}

NFmiEditMapGeneralDataDoc* CSmartMetView::GetGeneralDoc(void)
{
	return GetDocument()->GetData();
}

SmartMetDocumentInterface* CSmartMetView::GetSmartMetDocumentInterface()
{
    return SmartMetDocumentInterface::GetSmartMetDocumentInterfaceImplementation();
}

/*
void CSmartMetView::OnRButtonUp(UINT nFlags, CPoint point)
{
	ClientToScreen(&point);
	OnContextMenu(this, point);
}
*/

#ifndef FMI_DISABLE_MFC_FEATURE_PACK
void CSmartMetView::OnContextMenu(CWnd* pWnd, CPoint point)
{
	theApp.GetContextMenuManager()->ShowPopupMenu(IDR_POPUP_EDIT, point.x, point.y, this, TRUE);
}
#endif // FMI_DISABLE_MFC_FEATURE_PACK


// CSmartMetView diagnostics

#ifdef _DEBUG
void CSmartMetView::AssertValid() const
{
	CView::AssertValid();
}

void CSmartMetView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CSmartMetDoc* CSmartMetView::GetDocument() const // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CSmartMetDoc)));
	return (CSmartMetDoc*)m_pDocument;
}
#endif //_DEBUG


// CSmartMetView message handlers
static const int MAINMAPVIEW_TOOLTIP_ID = 1234566;

void CSmartMetView::OnInitialUpdate()
{
	CView::OnInitialUpdate();

    // Must initialize SmartMetView pointers here, at the end of this method they are used by calls from OnSize method.
    ApplicationInterface::SetSmartMetView(this);
    ApplicationInterface::SetSmartMetViewAsCView(this);

	CSmartMetDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	NFmiEditMapGeneralDataDoc *data = pDoc->GetData();
	if(data)
	{
		data->MapViewDescTop(itsMapViewDescTopIndex)->MapView(this);
	}

	if(itsToolBox)
		delete itsToolBox;
	itsToolBox = new NFmiToolBox(this);
	CreateEditMapView();
	DragAcceptFiles(TRUE);
	CFmiWin32Helpers::InitializeCPPTooltip(this, *m_tooltip, MAINMAPVIEW_TOOLTIP_ID);
	CRect winRec;
	GetWindowRect(winRec);
	OnSize(SW_RESTORE, winRec.Width(), winRec.Height()); // jostain syystä on pakko tehdä onsize, että tooltip toimii varmasti koko ikkunan alueella?!?!?
}

void CSmartMetView::CreateEditMapView()
{
	CSmartMetDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	NFmiEditMapGeneralDataDoc *data = pDoc->GetData();
	if(!data)
		return;
	delete itsEditMapView;
	itsEditMapView = new NFmiEditMapView(itsMapViewDescTopIndex, itsToolBox, &itsDrawingEnvironment, itsDrawParam);
}

void CSmartMetView::OnLButtonUp(UINT nFlags, CPoint point)
{
	try // try-catch -blokilla varmistetaan että lopuksi ReleaseCapture todellakin kutsutaan
	{
		CDC *theDC = GetDC();
		if(!theDC)
			return;
		SetToolsDCs(theDC);

		CtrlView::ReleaseCtrlKeyIfStuck(nFlags); // tämä vapauttaa CTRL-napin, jos se on 'jumiutunut' pohjaan (MFC bugi, ctrl-nappi voi jäädä pohjaan, jos kyseinen näppäin vapautetaan, ennen kuin kartta ruudun piirto on valmis)

		bool needsUpdate = itsEditMapView ? itsEditMapView->LeftButtonUp(itsToolBox->ToViewPoint(point.x, point.y)
			,itsToolBox->ConvertCtrlKey(nFlags)) : false;	// M.K. 29.4.99 Lisäsin "parametrivalintalaatikon" piirtämistä varten.
		ReleaseDC(theDC);
		Invalidate(FALSE);
		if(needsUpdate)
		{
			GetDocument()->UpdateAllViewsAndDialogs("Main map view: left mouse button was released");
            ForceDrawOverBitmapThings(itsMapViewDescTopIndex, false, true);
		}
	}
	catch(...)
	{
		ReleaseCapture(); // vapautetaan lopuksi hiiren viestit muidenkin ikkunoiden käyttöön
		throw ; // laitetaan poikkeus eteenpäin
	}
	ReleaseCapture(); // vapautetaan lopuksi hiiren viestit muidenkin ikkunoiden käyttöön (OnLButtonDown:issa laitettiin SetCapture päälle)

}

void CSmartMetView::OnMButtonUp(UINT nFlags, CPoint point)
{
	CDC *theDC = GetDC();
	if(!theDC)
		return;
	SetToolsDCs(theDC);

	CtrlView::ReleaseCtrlKeyIfStuck(nFlags); // tämä vapauttaa CTRL-napin, jos se on 'jumiutunut' pohjaan (MFC bugi, ctrl-nappi voi jäädä pohjaan, jos kyseinen näppäin vapautetaan, ennen kuin kartta ruudun piirto on valmis)

	bool needsUpdate = itsEditMapView ? itsEditMapView->MiddleButtonUp(itsToolBox->ToViewPoint(point.x, point.y)
		,itsToolBox->ConvertCtrlKey(nFlags)) : false;
	ReleaseDC(theDC);
	Invalidate(FALSE);
	if(needsUpdate)
	{
		GetDocument()->UpdateAllViewsAndDialogs("Main map view: middle mouse button was released");
	}
}

void CSmartMetView::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	CDC *theDC = GetDC();
	if(!theDC)
		return;
	SetToolsDCs(theDC);

    CtrlView::ReleaseCtrlKeyIfStuck(nFlags); // tämä vapauttaa CTRL-napin, jos se on 'jumiutunut' pohjaan (MFC bugi, ctrl-nappi voi jäädä pohjaan, jos kyseinen näppäin vapautetaan, ennen kuin kartta ruudun piirto on valmis)

	bool needsUpdate = itsEditMapView ? itsEditMapView->LeftDoubleClick(itsToolBox->ToViewPoint(point.x, point.y)
		,itsToolBox->ConvertCtrlKey(nFlags)) : false;
	ReleaseDC(theDC);
	if(needsUpdate)
	{
		CSmartMetDoc* pDoc = GetDocument();
		if(pDoc)
		{
			if(pDoc->GetData()->ActivateParamSelectionDlgAfterLeftDoubleClick())
			{
				pDoc->GetData()->ActivateParamSelectionDlgAfterLeftDoubleClick(false);
				pDoc->ActivateParameterSelectionDlg();
				return ;
			}
		}
		Invalidate(FALSE);
		GetDocument()->UpdateAllViewsAndDialogs("Main map view: left mouse button double click");
	}
//	CView::OnLButtonDblClk(nFlags, point);
}

void CSmartMetView::OnRButtonDblClk(UINT nFlags, CPoint point)
{
	CDC *theDC = GetDC();
	if(!theDC)
		return;
	SetToolsDCs(theDC);

    CtrlView::ReleaseCtrlKeyIfStuck(nFlags); // tämä vapauttaa CTRL-napin, jos se on 'jumiutunut' pohjaan (MFC bugi, ctrl-nappi voi jäädä pohjaan, jos kyseinen näppäin vapautetaan, ennen kuin kartta ruudun piirto on valmis)

	bool needsUpdate = itsEditMapView ? itsEditMapView->RightDoubleClick(itsToolBox->ToViewPoint(point.x, point.y)
		,itsToolBox->ConvertCtrlKey(nFlags)) : false;
	ReleaseDC(theDC);
	if(needsUpdate)
	{
		Invalidate(FALSE);
		GetDocument()->UpdateAllViewsAndDialogs("Main map view: right mouse button double click");
	}
//    CView::OnRButtonDblClk(nFlags, point);
}

void CSmartMetView::OnRButtonUp(UINT nFlags, CPoint point)
{
/*
	ClientToScreen(&point);
	OnContextMenu(this, point);
*/

	CDC *theDC = GetDC();
	if(!theDC)
		return;
	SetToolsDCs(theDC);

    CtrlView::ReleaseCtrlKeyIfStuck(nFlags); // tämä vapauttaa CTRL-napin, jos se on 'jumiutunut' pohjaan (MFC bugi, ctrl-nappi voi jäädä pohjaan, jos kyseinen näppäin vapautetaan, ennen kuin kartta ruudun piirto on valmis)

	bool needsUpdate = itsEditMapView ? itsEditMapView->RightButtonUp(itsToolBox->ToViewPoint(point.x, point.y)
		,itsToolBox->ConvertCtrlKey(nFlags)) : false;
	ReleaseDC(theDC);

	NFmiEditMapGeneralDataDoc* genData = GetDocument()->GetData();
	Invalidate(FALSE);
	if(genData && genData->OpenPopupMenu())
	{
        CFmiPopupMenu menu;
//        CFmiMenu menu;
		menu.Init(genData->PopupMenu());
		CMenu* pPopup = menu.Popup();

		CRect tempRect;
		GetWindowRect(tempRect);
		pPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x+tempRect.left, point.y+tempRect.top, this);
		genData->OpenPopupMenu(false);
	}
	else if(needsUpdate)
	{
		GetDocument()->UpdateAllViewsAndDialogs("Main map view: right mouse button was released");
		ForceDrawOverBitmapThings(itsMapViewDescTopIndex, false, true);
	}
}

void CSmartMetView::OnUpdate(CView* /* pSender */ , LPARAM /* lHint */ , CObject* /* pHint */ )
{
	CSmartMetDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	if(itsEditMapView)
		itsEditMapView->Update();
	Invalidate(FALSE);

//	CView::OnUpdate(pSender, lHint, pHint);
}

BOOL CSmartMetView::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
	return CView::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}

BOOL CSmartMetView::OnCommand(WPARAM wParam, LPARAM lParam)
{
	NFmiEditMapGeneralDataDoc* genData = GetDocument()->GetData();
	if(genData && genData->PopupMenu())
	{
		NFmiMenuItemList* menuItemList = genData->PopupMenu();
		unsigned short nID = short (wParam);
		if (nID >= menuItemList->MinId() && nID <= menuItemList->MaxId())
		{
			if(genData->MakePopUpCommandUsingRowIndex(nID))
			{
				GetDocument()->UpdateAllViewsAndDialogs("Main map view: making popup command");
				return TRUE;
			}
		}
	}

	return CView::OnCommand(wParam, lParam);
}

void CSmartMetView::OnLButtonDown(UINT nFlags, CPoint point)
{
	CDC *theDC = GetDC();
	if(!theDC)
		return;
	SetToolsDCs(theDC);

	bool needsUpdate = itsEditMapView ? itsEditMapView->LeftButtonDown(itsToolBox->ToViewPoint(point.x, point.y)
		,itsToolBox->ConvertCtrlKey(nFlags)) : false;
	ReleaseDC(theDC);

	if(needsUpdate)
	{
		GetDocument()->UpdateAllViewsAndDialogs("Main map view: left mouse button was pressed down");
	}
	SetCapture(); // otetaan hiiren liikkeet/viestit talteeen toistaiseksi tähän ikkunaan
}

// Toimenpiteet, mitä tehdään kun hiiri tulee karttanäytön päälle
void CSmartMetView::MouseEnter(void)
{
	GetDocument()->GetData()->MouseOnMapView(true);
}

// Toimen piteet, mitä tehdään kun hiiri lähtee karttanäytön päältä
void CSmartMetView::MouseLeave(void)
{
	GetDocument()->GetData()->MouseOnMapView(false);
}

static bool NearestShipLocation(NFmiFastQueryInfo &theInfo, const NFmiLocation &theLocation)
{
	double minDist = 999999999.;
	unsigned long minLocInd = static_cast<unsigned long>(-1);
	for(theInfo.ResetLocation(); theInfo.NextLocation(); )
	{
		NFmiPoint p(theInfo.GetLatlonFromData());
		if(p.X() == kFloatMissing || p.Y() == kFloatMissing)
			continue;
		double currentDist = theLocation.Distance(p);
		if(currentDist < minDist)
		{
			minDist = currentDist;
			minLocInd = theInfo.LocationIndex();
		}
	}
	return theInfo.LocationIndex(minLocInd);
}

std::string CSmartMetView::MakeActiveDataLocationIndexString(const NFmiPoint &theLatlon)
{
	std::string str;
	NFmiEditMapGeneralDataDoc* genData = GetDocument()->GetData();
	int cursorRowIndex = genData->ToolTipRowIndex();
	boost::shared_ptr<NFmiDrawParam> activeDrawParam = genData->ActiveDrawParam(itsMapViewDescTopIndex, cursorRowIndex);
	if(activeDrawParam)
	{ // jos kursorilla osoitetulta karttariviltä löytyy aktiivinen data, jolla on info, laitetaan näkyviin myös
	  // datassa olevan lähimmän hilapisteen tarkka sijainti
		boost::shared_ptr<NFmiFastQueryInfo> info = genData->InfoOrganizer()->Info(activeDrawParam, false, true);
		if(info)
		{
			NFmiLocation loc(theLatlon);
			if(info->NearestLocation(loc))
			{
				str += " (data-pt: ";
/*
				NFmiPoint activeDataPointLatlon = info->LatLon();
				str += "  (Nearest act. data pt.: ";
				str += "Lon: ";
				NFmiValueString lonStr2((float)activeDataPointLatlon.X(), "%0.4f");
				str += lonStr2;
				str += "   Lat: ";
				NFmiValueString latStr2((float)activeDataPointLatlon.Y(), "%0.4f");
				str += latStr2;
*/
				if(info->Grid())
				{ // laitetaan hila datan tapauksessa vielä hila indeksit näkyviin
					NFmiPoint activeGridPoint = info->Grid()->GridPoint(info->LocationIndex());
					str += " X: ";
					NFmiValueString xStr((int)activeGridPoint.X(), "%d");
					str += xStr;
					str += " Y: ";
					NFmiValueString yStr((int)activeGridPoint.Y(), "%d");
					str += yStr;
				}
				str += " locIndex: ";
				NFmiValueString locIndStr((int)info->LocationIndex(), "%d");
				str += locIndStr;
				str += ")";
			}
		}
	}
	return str;
}

void CSmartMetView::PutTextInStatusBar(const std::string &theText)
{
    CMainFrame *pFrame = (CMainFrame *)AfxGetApp()->m_pMainWnd;
#ifdef FMI_DISABLE_MFC_FEATURE_PACK
    CStatusBar* pStatus = &pFrame->m_wndStatusBar;
#else
    CMFCStatusBar* pStatus = &pFrame->m_wndStatusBar;
#endif // FMI_DISABLE_MFC_FEATURE_PACK
    if(pStatus && pStatus->GetSafeHwnd())
        pStatus->SetPaneText(0, CA2T(theText.c_str()));
}

void CSmartMetView::OnMouseMove(UINT nFlags, CPoint point)
{
	CDC *theDC = GetDC();
	if(!theDC)
		return;
	SetToolsDCs(theDC);

    CtrlView::ReleaseCtrlKeyIfStuck(nFlags); // tämä vapauttaa CTRL-napin, jos se on 'jumiutunut' pohjaan (MFC bugi, ctrl-nappi voi jäädä pohjaan, jos kyseinen näppäin vapautetaan, ennen kuin kartta ruudun piirto on valmis)

// laitetaan myös karttabitmap valmiiksi osittaisia päivityksiä varten
	CClientDC dc(this);
	CDC dcMem;
	dcMem.CreateCompatibleDC(&dc);
	CBitmap* oldBitmap2 = dcMem.SelectObject(itsMapBitmap);
	NFmiEditMapGeneralDataDoc* genData = GetDocument()->GetData();
	genData->MapViewDescTop(itsMapViewDescTopIndex)->MapBlitDC(&dcMem);

	NFmiPoint viewPoint(itsToolBox->ToViewPoint(point.x, point.y));

	// HUOM! jos pensselillä muokataan läpinäkyvyydellä piirrettävää dataa, pitää läpinäkyvyys näyttö asettaa käyttöön
	genData->TransparencyContourDrawView(this); // tämä on osa kikkaa, jolla saadaan piiirettyä läpinäkyviä kenttiä toolmasterilla tai imaginella
	bool needsUpdate = false;
	try
	{
		needsUpdate = itsEditMapView ? itsEditMapView->MouseMove(viewPoint, itsToolBox->ConvertCtrlKey(nFlags)) : false;
	}
	catch(...)
	{
	}
	genData->TransparencyContourDrawView(0); // lopuksi se asetetaan taas 0-pointteriksi...

	ReleaseDC(theDC);
	genData->MapViewDescTop(itsMapViewDescTopIndex)->MapBlitDC(0);
	dcMem.SelectObject(oldBitmap2);
	dcMem.DeleteDC();

	bool drawOverBitmapAnyway = genData->EditedPointsSelectionChanged(false); // pyydetään vanha arvo drawOverBitmapAnyway -muuttujaan ja asetetaan samalla false:ksi
	if(itsEditMapView->MapRect().IsInside(viewPoint))
	{
		NFmiPoint cursorLatlon(genData->ToolTipLatLonPoint());
		std::string str = CtrlViewUtils::GetTotalMapViewStatusBarStr(&genData->GetCtrlViewDocumentInterface(), cursorLatlon);
        // Laitetaan kartta osion pikseli koko tähän perään sulkuihin
        str += CtrlViewUtils::MakeMapPortionPixelSizeStringForStatusbar(genData->MapViewDescTop(itsMapViewDescTopIndex)->ActualMapBitmapSizeInPixels(), false);

		// Jos CTRL-nappi on pohjassa, lisätään vielä osoitetun karttaruudun aktiivisen datan joko hilapiste indeksit tai havaintoaseman location id.
        if(CtrlView::IsKeyboardKeyDown(VK_CONTROL))
    		str += MakeActiveDataLocationIndexString(cursorLatlon);

        PutTextInStatusBar(str);

		// Päivitetään myös synop data teksti muodossa taulukossa ikkunaa, eli
		// lähin asema scrollataan näkyviin ja tehdään se valituksi
		if(genData->SynopDataGridViewOn())
		{
			if(nFlags & MK_CONTROL && genData->MouseCaptured() == false)
			{// tehdään lähimpien havainto asemien seuranta vain CTRL pohjassa ja kun hiiri ei ole kaapattu
				if(itsEditMapView->MapRect().IsInside(viewPoint))
				{ // ja vain jos ollaan karttanäytön päällä, ei kun ollaan aikakontrolli ikkunan päällä
					boost::shared_ptr<NFmiFastQueryInfo> obsInfo = genData->GetNearestSynopStationInfo(NFmiLocation(cursorLatlon), NFmiMetTime(), true, 0);
					if(obsInfo)
					{
						if(GetDocument()->SynopDataGridViewDlg())
						{
							genData->LastSelectedSynopWmoId(obsInfo->Location()->GetIdent());
							GetDocument()->SynopDataGridViewDlg()->EnsureVisibleStationRow();
							drawOverBitmapAnyway = true;
						}
					}
				}
			}
		}
	}

	if(needsUpdate)
	{
		GetDocument()->UpdateAllViewsAndDialogs("Main map view: mouse cursor moved while dragging it");
	}
	else if(genData && genData->MapViewDescTop(itsMapViewDescTopIndex)->MapViewBitmapDirty())
		Invalidate(FALSE);
	else if(!(genData->MiddleMouseButtonDown() && genData->MouseCaptured())) // muuten ForceDrawOverBitmapThings, paitsi jos ollaan vetämässä kartan päälle zoomi laatikkoa, koska se peittyisi aina ForceDrawOverBitmapThings:n alle
	{
		if(genData->ShowMouseHelpCursorsOnMap() || drawOverBitmapAnyway)
		{
			ForceDrawOverBitmapThings(itsMapViewDescTopIndex, true, true); // hiiren apukursorit pitää joka tapauksessa piirtää aina
		}
		if(genData->MustDrawTempView())
		{
			genData->MustDrawTempView(false);
			GetDocument()->UpdateTempView();
		}
		if(genData->MustDrawTimeSerialView())
		{
			genData->MustDrawTimeSerialView(false);
			GetDocument()->UpdateTimeSerialView();
		}
		if(genData->MustDrawCrossSectionView())
		{
			genData->MustDrawCrossSectionView(false);
			GetDocument()->UpdateCrossSectionView();
			ForceDrawOverBitmapThings(itsMapViewDescTopIndex, true, false);
		}
	}
}

static bool IsViewForceUpdated(unsigned int viewIndexToBeUpdated, unsigned int theOriginalCallerDescTopIndex, bool doOriginalView, bool doAllOtherMapViews)
{
    bool originalIsCurrentViewIndex = (viewIndexToBeUpdated == theOriginalCallerDescTopIndex);
    if(originalIsCurrentViewIndex && doOriginalView)
        return true;
    if(!originalIsCurrentViewIndex && doAllOtherMapViews)
        return true;

    return false;
}

void CSmartMetView::ForceOtherMapViewsDrawOverBitmapThings(unsigned int theOriginalCallerDescTopIndex, bool doOriginalView, bool doAllOtherMapViews)
{
	NFmiEditMapGeneralDataDoc* genData = GetDocument()->GetData();
	std::vector<NFmiMapViewDescTop *> &mapViewDescTopList = genData->MapViewDescTopList();
	// tässä käydään vain 1:stä eteenpäin desctoppeja, koska tuossa aluksi käytiin jo läpi 1..
	// tämä siksi että 1:stä eteenpäin desctopin mapview on aina samaa tyyppiä
	for(size_t i = 1; i < mapViewDescTopList.size(); i++)
	{
		if(::IsViewForceUpdated(static_cast<unsigned int>(i), theOriginalCallerDescTopIndex, doOriginalView, doAllOtherMapViews))
		{
			CFmiExtraMapView *view = dynamic_cast<CFmiExtraMapView*>(mapViewDescTopList[i]->MapView());
			if(view)
				view->ForceDrawOverBitmapThingsThisExtraMapView();
		}
	}
}

// tämä on ikävä kopio CFmiSynopDataGridViewDlg-luokasta, tämä pitäisi laittaa
// dokumenttiin, että saataisiin yhtenevä funktio molemmissa paikoissa
boost::shared_ptr<NFmiFastQueryInfo> CSmartMetView::GetWantedInfo(int theProducerId)
{
	NFmiEditMapGeneralDataDoc* data = GetDocument()->GetData();
	boost::shared_ptr<NFmiFastQueryInfo> info = data->InfoOrganizer()->FindInfo(NFmiInfoData::kObservations, NFmiProducer(theProducerId), true);
	if(info == 0)
	{ // jos ei löytynyt havainnoista synop dataa, tarkastetaan editoitava data, onko se asema dataa
		info = data->EditedSmartInfo();
		if(info)
			if(info->IsGrid())
				info = boost::shared_ptr<NFmiFastQueryInfo>(); // ei haluta hila dataa
	}
	return info;
}

// pakotetaan piirtämään bitblitillä bitmap cache karttanäytön päälle
// ja sitten päälle piirretään nopeasti DrawOverBitmapThings
void CSmartMetView::ForceDrawOverBitmapThings(unsigned int originalCallerDescTopIndex, bool doOriginalView, bool doAllOtherMapViews)
{
    bool originalCallerIsMainMapview = (originalCallerDescTopIndex == itsMapViewDescTopIndex);
    bool doMainMapView = (originalCallerIsMainMapview && doOriginalView) || (!originalCallerIsMainMapview && doAllOtherMapViews);
    if(doMainMapView)
    {
        NFmiEditMapGeneralDataDoc *data = GetDocument()->GetData();
        if(data)
        {
            data->MapViewDescTop(itsMapViewDescTopIndex)->MapViewBitmapDirty(true);
            Invalidate(FALSE);
        }
    }

    ForceOtherMapViewsDrawOverBitmapThings(originalCallerDescTopIndex, doOriginalView, doAllOtherMapViews);
}

void CSmartMetView::CurrentPrintTime(const NFmiMetTime &theTime)
{
	GetGeneralDoc()->CurrentTime(itsMapViewDescTopIndex, theTime);
}

const NFmiRect* CSmartMetView::RelativePrintRect(void)
{
	return &(GetGeneralDoc()->MapViewDescTop(itsMapViewDescTopIndex)->RelativeMapRect());
}

CSize CSmartMetView::GetPrintedAreaOnScreenSizeInPixels(void)
{
    auto sizeInPixels = GetGeneralDoc()->GetPrintedMapAreaOnScreenSizeInPixels(itsMapViewDescTopIndex);
	return CSize(static_cast<int>(sizeInPixels.X()), static_cast<int>(sizeInPixels.Y()));
}

NFmiPoint CSmartMetView::PrintViewSizeInPixels(void)
{
	return GetGeneralDoc()->MapViewDescTop(itsMapViewDescTopIndex)->MapViewSizeInPixels();
}

CtrlViewUtils::GraphicalInfo& CSmartMetView::GetGraphicalInfo(void)
{
	return GetGeneralDoc()->MapViewDescTop(itsMapViewDescTopIndex)->GetGraphicalInfo();
}

void CSmartMetView::OnSize(UINT nType, int cx, int cy)
{
    static size_t counter = 0; // Nämä viritykset on tosi ärsyttäviä mutta jos en laske kuinka mones kerta ollaan täällä, tekstin laittaminen statusbar:iin kaataa SmarrtMetin, koska se valmistuu vasta myöhemmin ja en tiedä miten sitä voisi tarkastella että onko se jo valmis

	CView::OnSize(nType, cx, cy);

	CRect rect;
	GetClientRect(rect);
	m_tooltip->SetToolRect(this, MAINMAPVIEW_TOOLTIP_ID, rect);

	CSmartMetDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	NFmiEditMapGeneralDataDoc *data = pDoc->GetData();
	if(data)
	{
        data->DoMapViewOnSize(itsMapViewDescTopIndex, NFmiPoint(cx, cy), NFmiPoint(rect.Width(), rect.Height()));
		CDC *pDC = GetDC();
		CFmiWin32Helpers::SetDescTopGraphicalInfo(GetGraphicalInfo(), pDC, PrintViewSizeInPixels(), data->DrawObjectScaleFactor(), true); // true pakottaa initialisoinnin

        if(counter > 2)
            PutTextInStatusBar(CtrlViewUtils::MakeMapPortionPixelSizeStringForStatusbar(data->MapViewDescTop(itsMapViewDescTopIndex)->ActualMapBitmapSizeInPixels(), true));

        // Vain tämä näyttö itse ja zoomaus dialogi pitää päivittää
		pDoc->UpdateAllViewsAndDialogs("Main map view resized", SmartMetViewId::MainMapView | SmartMetViewId::ZoomDlg);
	}
    counter++;
	Invalidate(FALSE);
}

void CSmartMetView::Update(void)
{
	CDC *theDC = GetDC();
	if(!theDC)
		return;
	SetToolsDCs(theDC);
	itsEditMapView->Update();
	ReleaseDC(theDC);
    Invalidate(FALSE);
}

void CSmartMetView::RefreshApplicationViewsAndDialogs(const std::string &reasonForUpdate, bool redrawMapView, bool clearMapViewBitmapCacheRows, int theWantedMapViewDescTop)
{
	if(redrawMapView)
	{
		if(theWantedMapViewDescTop == -1)
			GetDocument()->GetData()->MapViewDirty(itsMapViewDescTopIndex, false, clearMapViewBitmapCacheRows, redrawMapView, false, false, false);
		else
			GetDocument()->GetData()->MapViewDirty(theWantedMapViewDescTop, false, clearMapViewBitmapCacheRows, redrawMapView, false, false, false);
	}

	GetDocument()->UpdateAllViewsAndDialogs(reasonForUpdate);
}

void CSmartMetView::RefreshApplicationViewsAndDialogs(const std::string &reasonForUpdate, SmartMetViewId updatedViewsFlag, bool redrawMapView, bool clearMapViewBitmapCacheRows, int theWantedMapViewDescTop)
{
    if(redrawMapView)
    {
        if(theWantedMapViewDescTop == -1)
            GetDocument()->GetData()->MapViewDirty(itsMapViewDescTopIndex, false, clearMapViewBitmapCacheRows, redrawMapView, false, false, false);
        else
            GetDocument()->GetData()->MapViewDirty(theWantedMapViewDescTop, false, clearMapViewBitmapCacheRows, redrawMapView, false, false, false);
    }

    GetDocument()->UpdateAllViewsAndDialogs(reasonForUpdate, updatedViewsFlag);
}

void CSmartMetView::ActivateFilterDlg(void)
{
	GetDocument()->ActivateFilterDlg();
}

void CSmartMetView::OnRButtonDown(UINT nFlags, CPoint point)
{
	CDC *theDC = GetDC();
	if(!theDC)
		return;
	SetToolsDCs(theDC);

	bool needsUpdate = itsEditMapView ? itsEditMapView->RightButtonDown(itsToolBox->ToViewPoint(point.x, point.y)
		,itsToolBox->ConvertCtrlKey(nFlags)) : false;
	ReleaseDC(theDC);

	if(needsUpdate)
	{
		GetDocument()->UpdateAllViewsAndDialogs("Main map view: right mouse button was pressed down");
	}
}

BOOL CSmartMetView::OnSetCursor(CWnd* /* pWnd */ , UINT /* nHitTest */ , UINT /* message */ )
{
	NFmiEditMapGeneralDataDoc* genData = GetDocument()->GetData();
	if(genData->ModifyToolMode() == CtrlViewUtils::kFmiEditorModifyToolModeBrush)
	{
		SetCursor(AfxGetApp()->LoadStandardCursor(IDC_CROSS));
	}
	else
	{
		SetCursor(AfxGetApp()->LoadStandardCursor(IDC_ARROW));
	}
	return TRUE;

//	return CView::OnSetCursor(pWnd, nHitTest, message);
}

void CSmartMetView::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	NFmiEditMapGeneralDataDoc* doc = GetDocument()->GetData();
    bool ctrlOrShiftKeyDown = CtrlView::IsKeyboardKeyDown(VK_CONTROL) || CtrlView::IsKeyboardKeyDown(VK_SHIFT); // ctrl/shift avaimet eivät saa olla painettuina, jos on, tällöin ei nuolinäppäimiä oteta huomioon, koska ne hoidetaan muualla keyboard-acceleraattoreilla
	// Tämä koodi on 'kopioitu' myös karttaapuikkunoihin
	// CFmiExtraMapViewDlg::PreTranslateMessage
	switch(nChar)
	{
	case VK_LEFT:
		if(!ctrlOrShiftKeyDown && doc->SetDataToPreviousTime(itsMapViewDescTopIndex))
			GetDocument()->UpdateAllViewsAndDialogs("Main map view: left cursor key pressed");
		return;
	case VK_RIGHT:
		if(!ctrlOrShiftKeyDown && doc->SetDataToNextTime(itsMapViewDescTopIndex))
			GetDocument()->UpdateAllViewsAndDialogs("Main map view: right cursor key pressed");
		return;
	case VK_UP:
		if(!ctrlOrShiftKeyDown && doc->ScrollViewRow(itsMapViewDescTopIndex, -1))
			GetDocument()->UpdateAllViewsAndDialogs("Main map view: up cursor key pressed");
		return;
	case VK_DOWN:
		if(!ctrlOrShiftKeyDown && doc->ScrollViewRow(itsMapViewDescTopIndex, 1))
			GetDocument()->UpdateAllViewsAndDialogs("Main map view: down cursor key pressed");
		return;
	}

	CView::OnKeyUp(nChar, nRepCnt, nFlags);
}

// asettaa toolmasterin ja toolboxin DC:t
// esim. ennen piirto tai ennen kuin tarkastellaan hiiren klikkausta ruudulta
// HUOM!! printtauksen yhteydessä kutsu ensin DC:n asetus ja sitten printinfon!!!
void CSmartMetView::SetToolsDCs(CDC* theDC)
{
	SetToolBoxsDC(theDC);
	SetToolMastersDC(theDC);
}

// toolboxin DC:n
void CSmartMetView::SetToolBoxsDC(CDC* theDC)
{
	if(itsToolBox)
		itsToolBox->SetDC(theDC);
}
// toolmasterin DC:n
void CSmartMetView::SetToolMastersDC(CDC* theDC)
{
    CtrlView::SetToolMastersDC(theDC, this, GetGeneralDoc()->IsToolMasterAvailable());
}

void CSmartMetView::OnDropFiles(HDROP hDropInfo)
{
	CWaitCursor cursor;

	POINT p;
	DragQueryPoint(hDropInfo, &p);

	CSmartMetDoc* pDoc = GetDocument();
	const UINT cch = 512;
	TCHAR fileName[512] = _TEXT("");
	UINT iFile = 0xFFFFFFFF; // 1. tiputetun tiedoston nimeä haetaan
	UINT numOfFiles = ::DragQueryFile(hDropInfo, iFile, fileName, cch);
	std::vector<std::string> files;
	for(unsigned int i=0 ; i<numOfFiles; i++)
	{
		::DragQueryFile(hDropInfo, i, fileName, cch);
		files.push_back(std::string(CT2A(fileName)));
	}
	pDoc->LoadDataFromFilesAndAdd(files);

//	CView::OnDropFiles(hDropInfo);
}

BOOL CSmartMetView::OnEraseBkgnd(CDC* /* pDC */ )
{
	return FALSE;	// näin ei tyhjennetä taustaa ennen ruudun piirtoa!!
//	return CView::OnEraseBkgnd(pDC);
}

void CSmartMetView::StoreViewMacroWindowsSettings(NFmiViewSettingMacro &theViewMacro)
{
	CSmartMetDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if(pDoc)
		pDoc->StoreViewMacroWindowsSettings(theViewMacro);
}

void CSmartMetView::LoadViewMacroWindowsSettings(NFmiViewSettingMacro &theViewMacro)
{
	CSmartMetDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if(pDoc)
		pDoc->LoadViewMacroWindowsSettings(theViewMacro);
}

BOOL CSmartMetView::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	CDC *theDC = GetDC();
	if(theDC)
	{
        CtrlView::ReleaseCtrlKeyIfStuck(nFlags); // tämä vapauttaa CTRL-napin, jos se on 'jumiutunut' pohjaan (MFC bugi, ctrl-nappi voi jäädä pohjaan, jos kyseinen näppäin vapautetaan, ennen kuin kartta ruudun piirto on valmis)
		SetToolsDCs(theDC);
		// Jostain syystä MouseWheel systeemi antaa hiiren kursorin paikan
		// absoluuttisen pikseli sijainnin desctopissa. Nyt pitää tehdä muunnos
		// kursorin paikka ikkunan omassa sijainnissa.
		CRect zview;
		GetWindowRect(zview);
		CPoint viewPoint(pt - zview.TopLeft());
		bool needsUpdate = itsEditMapView ? itsEditMapView->MouseWheel(itsToolBox->ToViewPoint(viewPoint.x, viewPoint.y)
			,itsToolBox->ConvertCtrlKey(nFlags), zDelta) : false;
		ReleaseDC(theDC);

		if(needsUpdate)
		{
			GetDocument()->UpdateAllViewsAndDialogs("Main map view: mouse wheel action");
		}
	}
	return TRUE;
//	return CView::OnMouseWheel(nFlags, zDelta, pt);
}

void CSmartMetView::OnMButtonDown(UINT nFlags, CPoint point)
{
	CDC *theDC = GetDC();
	if(!theDC)
		return;
	SetToolsDCs(theDC);

	bool needsUpdate = itsEditMapView ? itsEditMapView->MiddleButtonDown(itsToolBox->ToViewPoint(point.x, point.y)
		,itsToolBox->ConvertCtrlKey(nFlags)) : false;
	ReleaseDC(theDC);

	if(needsUpdate)
	{
		GetDocument()->UpdateAllViewsAndDialogs("Main map view: mouse middle button down");
	}
}

void CSmartMetView::OnMenuitemViewGridSelectionDlg()
{
#ifdef FMI_DISABLE_MFC_FEATURE_PACK
	CToolBar *toolbar = static_cast<CToolBar*>(&(static_cast<CMainFrame*>(AfxGetMainWnd())->m_wndToolBar));
	if(toolbar)
	{
		int buttonIndex = toolbar->CommandToIndex(ID_MENUITEM_VIEW_GRID_SELECTION_DLG);
		CRect buttRect; 
		toolbar->GetItemRect(buttonIndex, &buttRect);
		this->ClientToScreen(&buttRect); // muutetaan paikka absoluuttiseksi screenillä
		m_SizeWnd->Create(this, buttRect.left, buttRect.top + 1);
	}
#else
	CMFCToolBar *toolbar = static_cast<CMFCToolBar*>(&(static_cast<CMainFrame*>(AfxGetMainWnd())->m_wndToolBar));
	if(toolbar)
	{
		int buttonIndex = toolbar->CommandToIndex(ID_MENUITEM_VIEW_GRID_SELECTION_DLG);
		CMFCToolBarButton *button = toolbar->GetButton(buttonIndex);
		if(button)
		{
			// tämä saattaa toimia eri lailla eri Windowseilla (esim. XP vs. Vista)
			CRect buttRect = button->Rect(); 
			this->ClientToScreen(&buttRect); // muutetaan paikka absoluuttiseksi screenillä
			m_SizeWnd->Create(this, buttRect.left, buttRect.top + 1);
		}
	}
#endif // FMI_DISABLE_MFC_FEATURE_PACK
}

void CSmartMetView::UpdateMapView(unsigned int theDescTopIndex)
{
	CSmartMetView *view1 = dynamic_cast<CSmartMetView *>(GetDocument()->GetData()->MapViewDescTop(theDescTopIndex)->MapView());
	if(view1)
		view1->Update();
	else
	{
		CFmiExtraMapView *view2 = dynamic_cast<CFmiExtraMapView *>(GetDocument()->GetData()->MapViewDescTop(theDescTopIndex)->MapView());
		if(view2)
		{
			view2->UpdateMap();
			view2->Invalidate(FALSE);
		}
	}
}

void CSmartMetView::SetMapViewGridSize(const NFmiPoint &newSize)
{
	CSmartMetDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if(pDoc)
	{
		if(pDoc->GetData()->SetMapViewGrid(itsMapViewDescTopIndex, newSize))
			pDoc->UpdateAllViewsAndDialogs("Main map view: view's grid size changed");
	}
}


// Malli SizeDemo-projektista CSizeDemoDlg::DefWindowProc
LRESULT CSmartMetView::DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	if(message == TW_SETPARAMS)
	{
		m_cx.cx = LOWORD(wParam);
		m_cx.cy = HIWORD(wParam);
		SetMapViewGridSize(NFmiPoint(m_cx.cx, m_cx.cy));
		return (LRESULT)1;
	}

	return CView::DefWindowProc(message, wParam, lParam);
}

void CSmartMetView::ActivateZoomDialog(int theWantedDescTopIndex)
{
	GetDocument()->ActivateZoomDialog(theWantedDescTopIndex);
}

void CSmartMetView::OnPrepareDC(CDC* pDC, CPrintInfo* pInfo)
{
	CFmiWin32TemplateHelpers::OnPrepareDCMapView(this, pDC, pInfo);

	CView::OnPrepareDC(pDC, pInfo);
}

BOOL CSmartMetView::PreTranslateMessage(MSG* pMsg)
{
	NFmiEditMapGeneralDataDoc *doc = GetDocument()->GetData();
	if(doc && doc->MetEditorOptionsData().ShowToolTipsOnMapView())
		m_tooltip->RelayEvent(pMsg);

	return CView::PreTranslateMessage(pMsg);
}

// HUOM!!! duplikaatti koodia!!
// Lähes identtiset funktiot  NotifyDisplayTooltip ja DrawSynopPlotImage kahdessa luokassa:
// CSmartMetView ja CFmiExtraMapView. Mieti joska ne voisi yhdistää käyttämään samaa koodia.
void CSmartMetView::NotifyDisplayTooltip(NMHDR * pNMHDR, LRESULT * result)
{
    CFmiWin32TemplateHelpers::NotifyDisplayTooltip(this, pNMHDR, result, GetGeneralDoc()->Printing(), MAINMAPVIEW_TOOLTIP_ID);
}

void CSmartMetView::DrawSynopPlotImage(bool fDrawSoundingPlot, bool fDrawMinMaxPlot, bool fDrawMetarPlot)
{
	CClientDC dc(this);
	CDC dcMem;
	dcMem.CreateCompatibleDC(&dc);
	CBitmap *oldBitmap = dcMem.SelectObject(itsSynopPlotBitmap);
	SetToolBoxsDC(&dcMem);
	BITMAP pBitMap;
	itsSynopPlotBitmap->GetBitmap(&pBitMap);
	itsToolBox->SetClientRect(CRect(0, 0, pBitMap.bmWidth, pBitMap.bmHeight)); // pakko tehdä tälläinen viritys, koska view:sta luotu dc on eri kokoinen kuin käytetty bitmapin koko

	// pitää tyhjentää bitmappi piirtämällä valkoinen rect fillillä
	NFmiDrawingEnvironment envi;
	envi.EnableFill();
	envi.EnableFrame();
	envi.SetFillColor(NFmiColor(1,1,1));
	envi.SetFrameColor(NFmiColor(0.8f, 0.8f, 0.8f)); // piirretään vaalean harmaalla reunus
	NFmiRect bitmapRect(0, 0, 1, 1);
	NFmiRectangle drawRect(bitmapRect, 0, &envi);
	itsToolBox->Convert(&drawRect);

	NFmiRect drawingrect = bitmapRect;

	drawingrect.Inflate(-0.23); // en tiedä miksi, mutta tällälailla pitää piirto laatikkoa pienentää, muuten synop-plotti leviää piirrettäessä bitmappiin
	itsEditMapView->DrawSynopPlotOnToolTip(itsToolBox , drawingrect, fDrawSoundingPlot, fDrawMinMaxPlot, fDrawMetarPlot);
	itsSynopPlotBitmap = dcMem.SelectObject(oldBitmap);
	dcMem.DeleteDC();
}

void CSmartMetView::OnAcceleratorSwapArea()
{
	GetDocument()->GetData()->SwapArea(itsMapViewDescTopIndex);
	GetDocument()->UpdateAllViewsAndDialogs("Main map view: used map area swapped");
}

void CSmartMetView::OnAcceleratorMakeSwapBaseArea()
{
	GetDocument()->GetData()->MakeSwapBaseArea(itsMapViewDescTopIndex);
}

void CSmartMetView::OnEditCopy()
{
	GetDocument()->GetData()->CopyDrawParamsFromMapViewRow(itsMapViewDescTopIndex);
}

void CSmartMetView::OnEditPaste()
{
	GetDocument()->GetData()->PasteDrawParamsToMapViewRow(itsMapViewDescTopIndex);
}

void CSmartMetView::OnAcceleratorCopyAllMapViewParams()
{
	GetDocument()->GetData()->CopyMapViewDescTopParams(itsMapViewDescTopIndex);
}

void CSmartMetView::OnAcceleratorPasteAllMapViewParams()
{
	GetDocument()->GetData()->PasteMapViewDescTopParams(itsMapViewDescTopIndex);
}

void CSmartMetView::OnAcceleratorToggleAnimationView()
{
	GetDocument()->GetData()->ToggleTimeControlAnimationView(itsMapViewDescTopIndex);
}

void CSmartMetView::OnAcceleratorSetHomeTime()
{
    GetGeneralDoc()->OnAcceleratorSetHomeTime(itsMapViewDescTopIndex);
}

void CSmartMetView::RelativePrintRect(const NFmiRect &theRect)
{
	GetGeneralDoc()->MapViewDescTop(itsMapViewDescTopIndex)->RelativeMapRect(theRect);
}

void CSmartMetView::PrintViewSizeInPixels(const NFmiPoint &theSize)
{
	GetGeneralDoc()->MapViewDescTop(itsMapViewDescTopIndex)->MapViewSizeInPixels(theSize, true);
}

void CSmartMetView::SetPrintCopyCDC(CDC* pDC)
{
	GetGeneralDoc()->MapViewDescTop(itsMapViewDescTopIndex)->CopyCDC(pDC);
}

void CSmartMetView::MakePrintViewDirty(bool fViewDirty, bool fCacheDirty)
{
	GetGeneralDoc()->MapViewDescTop(itsMapViewDescTopIndex)->MapViewDirty(fViewDirty, fCacheDirty, true, false);
}

int CSmartMetView::CalcPrintingPageShiftInMinutes(void)
{
	return GetGeneralDoc()->MapViewDescTop(itsMapViewDescTopIndex)->CalcPrintingPageShiftInMinutes();
}

NFmiMetTime CSmartMetView::CalcPrintingStartTime(void)
{
	return GetGeneralDoc()->CurrentTime(itsMapViewDescTopIndex);
}

void CSmartMetView::SetNotificationMessage(const std::string &theNotificationMsgStr, const std::string &theNotificationTitle, int theStyle, int theTimeout, bool fNoSound)
{
	CMainFrame *pFrame = (CMainFrame *) AfxGetApp()->m_pMainWnd;
	if(pFrame)
		pFrame->SetNotificationMessage(theNotificationMsgStr, theNotificationTitle, theStyle, theTimeout, fNoSound);
}

void CSmartMetView::SetMacroErrorText(const std::string &theErrorStr)
{
	GetDocument()->SetMacroErrorText(theErrorStr);
}

void CSmartMetView::OnAcceleratorChangeTimeByDayForward()
{
	GetGeneralDoc()->ChangeTime(1, kForward, 1, itsMapViewDescTopIndex, 1);
}

void CSmartMetView::OnAcceleratorChangeTimeByDayBackward()
{
	GetGeneralDoc()->ChangeTime(1, kBackward, 1, itsMapViewDescTopIndex, 1);
}

void CSmartMetView::OnAcceleratorChangeTimeByWeekForward()
{
	GetGeneralDoc()->ChangeTime(2, kForward, 1, itsMapViewDescTopIndex, 1);
}

void CSmartMetView::OnAcceleratorChangeTimeByWeekBackward()
{
	GetGeneralDoc()->ChangeTime(2, kBackward, 1, itsMapViewDescTopIndex, 1);
}

void CSmartMetView::OnAcceleratorChangeTimeByMonthForward()
{
	GetGeneralDoc()->ChangeTime(3, kForward, 1, itsMapViewDescTopIndex, 1);
}

void CSmartMetView::OnAcceleratorChangeTimeByMonthBackward()
{
	GetGeneralDoc()->ChangeTime(3, kBackward, 1, itsMapViewDescTopIndex, 1);
}

void CSmartMetView::OnAcceleratorChangeTimeByYearForward()
{
	GetGeneralDoc()->ChangeTime(4, kForward, 1, itsMapViewDescTopIndex, 1);
}

void CSmartMetView::OnAcceleratorChangeTimeByYearBackward()
{
	GetGeneralDoc()->ChangeTime(4, kBackward, 1, itsMapViewDescTopIndex, 1);
}

void CSmartMetView::PutWarningFlagTimerOn(void)
{
	CMainFrame *pFrame = (CMainFrame *) AfxGetApp()->m_pMainWnd;
	if(pFrame)
		pFrame->PutWarningFlagTimerOn();
}


void CSmartMetView::OnUpdateButtonDataToDatabase(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(GetGeneralDoc()->CaseStudyModeOn() == false);
}

void CSmartMetView::OnAcceleratorMapZoomIn()
{
    CFmiWin32TemplateHelpers::ArrowKeyMapZoom(itsEditMapView, SmartMetDocumentInterface::GetSmartMetDocumentInterfaceImplementation(), 0.95);
}

void CSmartMetView::OnAcceleratorMapZoomOut()
{
    CFmiWin32TemplateHelpers::ArrowKeyMapZoom(itsEditMapView, SmartMetDocumentInterface::GetSmartMetDocumentInterfaceImplementation(), 1.05);
}

void CSmartMetView::OnAcceleratorMapPanDown()
{
    CFmiWin32TemplateHelpers::ArrowKeyMapPan(itsEditMapView, SmartMetDocumentInterface::GetSmartMetDocumentInterfaceImplementation(), NFmiPoint(0, 0.05));
}


void CSmartMetView::OnAcceleratorMapPanLeft()
{
    CFmiWin32TemplateHelpers::ArrowKeyMapPan(itsEditMapView, SmartMetDocumentInterface::GetSmartMetDocumentInterfaceImplementation(), NFmiPoint(-0.05, 0));
}


void CSmartMetView::OnAcceleratorMapPanRight()
{
    CFmiWin32TemplateHelpers::ArrowKeyMapPan(itsEditMapView, SmartMetDocumentInterface::GetSmartMetDocumentInterfaceImplementation(), NFmiPoint(0.05, 0));
}


void CSmartMetView::OnAcceleratorMapPanUp()
{
    CFmiWin32TemplateHelpers::ArrowKeyMapPan(itsEditMapView, SmartMetDocumentInterface::GetSmartMetDocumentInterfaceImplementation(), NFmiPoint(0, -0.05));
}

