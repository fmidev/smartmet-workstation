
// SmartMetView.cpp : implementation of the CSmartMetView class
//
#ifdef _MSC_VER
#pragma warning(disable : 4786) // poistaa n kpl VC++ k��nt�j�n varoitusta (liian pitk� nimi >255 merkki� joka johtuu 'puretuista' STL-template nimist�)
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
#include "CtrlViewTimeConsumptionReporter.h"
#include "CombinedMapHandlerInterface.h"
#include "NFmiLedLightStatus.h"
#include "NFmiMouseClickUrlActionData.h"

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
	ON_COMMAND(ID_ACCELERATOR_SWAP_AREA_SECONDARY_KEY, OnAcceleratorSwapAreaSecondaryKey)
	ON_COMMAND(ID_ACCELERATOR_MAKE_SWAP_BASE_AREA, OnAcceleratorMakeSwapBaseArea)
	ON_COMMAND(ID_EDIT_COPY, &CSmartMetView::OnEditCopy)
	ON_COMMAND(ID_EDIT_PASTE, &CSmartMetView::OnEditPaste)
	ON_COMMAND(ID_ACCELERATOR_COPY_ALL_MAP_VIEW_PARAMS, &CSmartMetView::OnAcceleratorCopyAllMapViewParams)
	ON_COMMAND(ID_ACCELERATOR_PASTE_ALL_MAP_VIEW_PARAMS, &CSmartMetView::OnAcceleratorPasteAllMapViewParams)
	ON_COMMAND(ID_ACCELERATOR_TOGGLE_ANIMATION_VIEW, &CSmartMetView::OnAcceleratorToggleAnimationView)
	ON_COMMAND(ID_ACCELERATOR_SET_HOME_TIME, &CSmartMetView::OnAcceleratorSetHomeTime)
	ON_COMMAND(ID_ACCELERATOR_CHANGE_TIME_BY_STEP1_FORWARD, &CSmartMetView::OnAcceleratorChangeTimeByStep1Forward)
	ON_COMMAND(ID_ACCELERATOR_CHANGE_TIME_BY_STEP1_BACKWARD, &CSmartMetView::OnAcceleratorChangeTimeByStep1Backward)
	ON_COMMAND(ID_ACCELERATOR_CHANGE_TIME_BY_STEP2_FORWARD, &CSmartMetView::OnAcceleratorChangeTimeByStep2Forward)
	ON_COMMAND(ID_ACCELERATOR_CHANGE_TIME_BY_STEP2_BACKWARD, &CSmartMetView::OnAcceleratorChangeTimeByStep2Backward)
	ON_COMMAND(ID_ACCELERATOR_CHANGE_TIME_BY_STEP3_FORWARD, &CSmartMetView::OnAcceleratorChangeTimeByStep3Forward)
	ON_COMMAND(ID_ACCELERATOR_CHANGE_TIME_BY_STEP3_BACKWARD, &CSmartMetView::OnAcceleratorChangeTimeByStep3Backward)
	ON_COMMAND(ID_ACCELERATOR_CHANGE_TIME_BY_STEP4_FORWARD, &CSmartMetView::OnAcceleratorChangeTimeByStep4Forward)
	ON_COMMAND(ID_ACCELERATOR_CHANGE_TIME_BY_STEP4_BACKWARD, &CSmartMetView::OnAcceleratorChangeTimeByStep4Backward)
	ON_UPDATE_COMMAND_UI(ID_BUTTON_DATA_TO_DATABASE, &CSmartMetView::OnUpdateButtonDataToDatabase)
    ON_COMMAND(ID_ACCELERATOR_MAP_ZOOM_IN, &CSmartMetView::OnAcceleratorMapZoomIn)
    ON_COMMAND(ID_ACCELERATOR_MAP_ZOOM_OUT, &CSmartMetView::OnAcceleratorMapZoomOut)
    ON_COMMAND(ID_ACCELERATOR_MAP_PAN_DOWN, &CSmartMetView::OnAcceleratorMapPanDown)
    ON_COMMAND(ID_ACCELERATOR_MAP_PAN_LEFT, &CSmartMetView::OnAcceleratorMapPanLeft)
    ON_COMMAND(ID_ACCELERATOR_MAP_PAN_RIGHT, &CSmartMetView::OnAcceleratorMapPanRight)
    ON_COMMAND(ID_ACCELERATOR_MAP_PAN_UP, &CSmartMetView::OnAcceleratorMapPanUp)
    ON_WM_RBUTTONDBLCLK()
	ON_WM_DISPLAYCHANGE()
	ON_COMMAND(ID_ACCELERATOR_CHANGE_ALL_MODEL_DATA_ON_ROW_TO_PREVIOUS_MODEL_RUN, &CSmartMetView::OnAcceleratorChangeAllModelDataOnRowToPreviousModelRun)
	ON_COMMAND(ID_ACCELERATOR_CHANGE_ALL_MODEL_DATA_ON_ROW_TO_NEXT_MODEL_RUN, &CSmartMetView::OnAcceleratorChangeAllModelDataOnRowToNextModelRun)
	ON_COMMAND(ID_ACCELERATOR_MAP_VIEW_RANGE_METER_MODE_TOGGLE, &CSmartMetView::OnAcceleratorMapViewRangeMeterModeToggle)
	ON_COMMAND(ID_ACCELERATOR_MAP_VIEW_RANGE_METER_COLOR_TOGGLE, &CSmartMetView::OnAcceleratorMapViewRangeMeterColorToggle)
	ON_COMMAND(ID_ACCELERATOR_MAP_VIEW_RANGE_METER_LOCK_MODE_TOGGLE, &CSmartMetView::OnAcceleratorMapViewRangeMeterLockModeToggle)
END_MESSAGE_MAP()

// CSmartMetView construction/destruction

CSmartMetView::CSmartMetView()
:itsFinalMapViewImageBitmap(new CBitmap)
,itsMemoryBitmap(new CBitmap)
,itsMapBitmap(new CBitmap)
,itsOverMapBitmap(new CBitmap)
,itsEditMapView(0)
,itsToolBox(0)
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
	// HUOM! Siis resursseissa m��ritelty bitmap IDB_BITMAP_SYNOP_PLOT_TOOLTIP_BASE m��r�� kuinka iso synop-plot
	// bitmapista tulee tooltippiin.
	// SYY miksi joudun luomaan bitmapin resursseista on siin� ett� en saanut win32 ImageList-systeemi� toimimaan muuten.
	// HUOM!2 Perus bitmapin pit�� olla neli�, koska synop-plot piirto perustuu siihen!
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
	delete itsSynopPlotBitmap; // t�h�n ei saa k�ytt�� DestroyBitmap-funktiota
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
	genDoc->TransparencyContourDrawView(this); // t�m� on osa kikkaa, jolla saadaan piiiretty� l�pin�kyvi� kentti� toolmasterilla tai imaginella
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
	genDoc->TransparencyContourDrawView(0); // lopuksi pit�� nollata l�pin�kyvyys-n�ytt� pointteri
}

// t�ll� piirret��n tavara, joka tulee my�s bitmapin p��lle
void CSmartMetView::DrawOverBitmapThings(NFmiToolBox * theGTB)
{
	if(theGTB)
		itsEditMapView->DrawOverBitmapThings(theGTB, false, 0, 0.f, 0);
	else
	{
		CtrlView::DeviceContextHandler<CSmartMetView> deviceContextHandler(this);
		itsEditMapView->DrawOverBitmapThings(itsToolBox, false, 0, 0.f, 0);
	}
}

bool CSmartMetView::GenerateMapBitmap(CBitmap *theUsedBitmap, CDC *theUsedCDC, CDC *theCompatibilityCDC)
{
	try
	{
	NFmiEditMapGeneralDataDoc *data = GetDocument()->GetData();
	return MapDraw::GenerateMapBitmap(&data->GetCtrlViewDocumentInterface(), itsMapViewDescTopIndex, theUsedBitmap, theUsedCDC, theCompatibilityCDC, nullptr);
	}
	catch(std::exception& e)
	{
		CatLog::logMessage(e.what(), CatLog::Severity::Error, CatLog::Category::Configuration, true);
		return false;
	}
}

void CSmartMetView::DoGraphReportOnDraw(const CtrlViewUtils::GraphicalInfo &graphicalInfo, double scaleFactor)
{
    static bool graphInfoReported = false;
    if(graphInfoReported == false)
    { // vain 1. kerran tehd��n lokiin raportti
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
	Invalidate(FALSE); // ruutu pit�� lopuksi p�ivitt��
}

// t�t� kutsutaan yleisess� printtaus funktiossa
void CSmartMetView::OldWayPrintUpdate(void)
{
	GetGeneralDoc()->GetCombinedMapHandler()->getMapViewDescTop(itsMapViewDescTopIndex)->SetBorderDrawDirtyState(CountryBorderDrawDirtyState::Geometry);
	itsEditMapView->Update(); // t�m� pit�� tehd� ett� prionttauksen aikaiset mapAreat ja systeemit tulevat voimaan
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
		data->GetCombinedMapHandler()->getMapViewDescTop(itsMapViewDescTopIndex)->MapView(this);
	}

	if(itsToolBox)
		delete itsToolBox;
	itsToolBox = new NFmiToolBox(this);
	CreateEditMapView();
	DragAcceptFiles(TRUE);
	CFmiWin32Helpers::InitializeCPPTooltip(this, *m_tooltip, MAINMAPVIEW_TOOLTIP_ID, CtrlViewUtils::MaxMapViewTooltipWidthInPixels);
	CRect winRec;
	GetWindowRect(winRec);
	OnSize(SW_RESTORE, winRec.Width(), winRec.Height()); // jostain syyst� on pakko tehd� onsize, ett� tooltip toimii varmasti koko ikkunan alueella?!?!?
}

void CSmartMetView::CreateEditMapView()
{
	CSmartMetDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	NFmiEditMapGeneralDataDoc *data = pDoc->GetData();
	if(!data)
		return;
	delete itsEditMapView;
	itsEditMapView = new NFmiEditMapView(itsMapViewDescTopIndex, itsToolBox, itsDrawParam);
}

void CSmartMetView::OnMButtonUp(UINT nFlags, CPoint point)
{
    try
    {
		CtrlView::DeviceContextHandler<CSmartMetView> deviceContextHandler(this);
		CtrlView::ReleaseCtrlKeyIfStuck(nFlags); // t�m� vapauttaa CTRL-napin, jos se on 'jumiutunut' pohjaan (MFC bugi, ctrl-nappi voi j��d� pohjaan, jos kyseinen n�pp�in vapautetaan, ennen kuin kartta ruudun piirto on valmis)

        bool needsUpdate = itsEditMapView ? itsEditMapView->MiddleButtonUp(itsToolBox->ToViewPoint(point.x, point.y)
            , itsToolBox->ConvertCtrlKey(nFlags)) : false;
        Invalidate(FALSE);
        if(needsUpdate)
        {
            GetDocument()->UpdateAllViewsAndDialogs("Main map view: middle mouse button was released");
        }
    }
    catch(...)
    {
        ReleaseCapture(); // vapautetaan lopuksi hiiren viestit muidenkin ikkunoiden k�ytt��n
        throw; // laitetaan poikkeus eteenp�in
    }
    ReleaseCapture(); // vapautetaan lopuksi hiiren viestit muidenkin ikkunoiden k�ytt��n (OnLButtonDown:issa laitettiin SetCapture p��lle)
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

// Toimenpiteet, mit� tehd��n kun hiiri tulee karttan�yt�n p��lle
void CSmartMetView::MouseEnter(void)
{
	GetDocument()->GetData()->MouseOnMapView(true);
}

// Toimen piteet, mit� tehd��n kun hiiri l�htee karttan�yt�n p��lt�
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
	int cursorRealRowIndex = genData->ToolTipRealRowIndex();
	boost::shared_ptr<NFmiDrawParam> activeDrawParam = genData->GetCombinedMapHandler()->activeDrawParamWithRealRowNumber(itsMapViewDescTopIndex, cursorRealRowIndex);
	if(activeDrawParam)
	{ 
		// jos kursorilla osoitetulta karttarivilt� l�ytyy aktiivinen data, jolla on info, laitetaan n�kyviin my�s
	    // datassa olevan l�himm�n hilapisteen tarkka sijainti
		boost::shared_ptr<NFmiFastQueryInfo> info = genData->InfoOrganizer()->Info(activeDrawParam, false, true);
		if(info)
		{
			NFmiLocation loc(theLatlon);
			if(info->NearestLocation(loc))
			{
				str += " (data-pt: ";
				if(info->Grid())
				{ // laitetaan hila datan tapauksessa viel� hila indeksit n�kyviin
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
	{
		auto textPaneIndex = GetGeneralDoc()->LedLightStatusSystem().MapViewTextStatusbarPaneIndex();
		pStatus->SetPaneText(textPaneIndex, CA2T(theText.c_str()));
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
	auto &mapViewDescTops = genData->GetCombinedMapHandler()->getMapViewDescTops();
	// t�ss� k�yd��n vain 1:st� eteenp�in desctoppeja, koska tuossa aluksi k�ytiin jo l�pi 1..
	// t�m� siksi ett� 1:st� eteenp�in desctopin mapview on aina samaa tyyppi�
	for(unsigned int mapViewDescTopIndex = 1; mapViewDescTopIndex < mapViewDescTops.size(); mapViewDescTopIndex++)
	{
		if(::IsViewForceUpdated(mapViewDescTopIndex, theOriginalCallerDescTopIndex, doOriginalView, doAllOtherMapViews))
		{
			CFmiExtraMapView *view = dynamic_cast<CFmiExtraMapView*>(mapViewDescTops[mapViewDescTopIndex]->MapView());
			if(view)
				view->ForceDrawOverBitmapThingsThisExtraMapView();
		}
	}
}

// t�m� on ik�v� kopio CFmiSynopDataGridViewDlg-luokasta, t�m� pit�isi laittaa
// dokumenttiin, ett� saataisiin yhtenev� funktio molemmissa paikoissa
boost::shared_ptr<NFmiFastQueryInfo> CSmartMetView::GetWantedInfo(int theProducerId)
{
	NFmiEditMapGeneralDataDoc* data = GetDocument()->GetData();
	boost::shared_ptr<NFmiFastQueryInfo> info = data->InfoOrganizer()->FindInfo(NFmiInfoData::kObservations, NFmiProducer(theProducerId), true);
	if(info == 0)
	{ // jos ei l�ytynyt havainnoista synop dataa, tarkastetaan editoitava data, onko se asema dataa
		info = data->EditedSmartInfo();
		if(info)
			if(info->IsGrid())
				info = boost::shared_ptr<NFmiFastQueryInfo>(); // ei haluta hila dataa
	}
	return info;
}

// pakotetaan piirt�m��n bitblitill� bitmap cache karttan�yt�n p��lle
// ja sitten p��lle piirret��n nopeasti DrawOverBitmapThings
void CSmartMetView::ForceDrawOverBitmapThings(unsigned int originalCallerDescTopIndex, bool doOriginalView, bool doAllOtherMapViews)
{
	bool originalCallerIsMainMapview = (originalCallerDescTopIndex == itsMapViewDescTopIndex);
	bool doMainMapView = (originalCallerIsMainMapview && doOriginalView) || (!originalCallerIsMainMapview && doAllOtherMapViews);
	if(doMainMapView)
	{
		GetDocument()->GetData()->GetCombinedMapHandler()->getMapViewDescTop(itsMapViewDescTopIndex)->MapViewBitmapDirty(true);
		Invalidate(FALSE);
	}

	ForceOtherMapViewsDrawOverBitmapThings(originalCallerDescTopIndex, doOriginalView, doAllOtherMapViews);
}

void CSmartMetView::CurrentPrintTime(const NFmiMetTime &theTime)
{
	GetGeneralDoc()->GetCombinedMapHandler()->currentTime(itsMapViewDescTopIndex, theTime, false);
}

const NFmiRect* CSmartMetView::RelativePrintRect(void)
{
	return &(GetGeneralDoc()->GetCombinedMapHandler()->getMapViewDescTop(itsMapViewDescTopIndex)->RelativeMapRect());
}

CSize CSmartMetView::GetPrintedAreaOnScreenSizeInPixels(void)
{
    auto sizeInPixels = GetGeneralDoc()->GetPrintedMapAreaOnScreenSizeInPixels(itsMapViewDescTopIndex);
	return CSize(static_cast<int>(sizeInPixels.X()), static_cast<int>(sizeInPixels.Y()));
}

NFmiPoint CSmartMetView::PrintViewSizeInPixels(void)
{
	return GetGeneralDoc()->GetCombinedMapHandler()->getMapViewDescTop(itsMapViewDescTopIndex)->MapViewSizeInPixels();
}

CtrlViewUtils::GraphicalInfo& CSmartMetView::GetGraphicalInfo(void)
{
	return GetGeneralDoc()->GetCombinedMapHandler()->getMapViewDescTop(itsMapViewDescTopIndex)->GetGraphicalInfo();
}

TrueMapViewSizeInfo& CSmartMetView::GetTrueMapViewSizeInfo()
{
	return GetGeneralDoc()->GetCombinedMapHandler()->getMapViewDescTop(itsMapViewDescTopIndex)->GetTrueMapViewSizeInfo();
}

void CSmartMetView::OnSize(UINT nType, int cx, int cy)
{
    static size_t counter = 0; // N�m� viritykset on tosi �rsytt�vi� mutta jos en laske kuinka mones kerta ollaan t��ll�, tekstin laittaminen statusbar:iin kaataa SmarrtMetin, koska se valmistuu vasta my�hemmin ja en tied� miten sit� voisi tarkastella ett� onko se jo valmis

	CView::OnSize(nType, cx, cy);

	CRect rect;
	GetClientRect(rect);
	m_tooltip->SetToolRect(this, MAINMAPVIEW_TOOLTIP_ID, rect);

	CSmartMetDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	NFmiEditMapGeneralDataDoc *data = pDoc->GetData();
	if(data)
	{
		CtrlView::DeviceContextHandler<CSmartMetView> deviceContextHandler(this);
        data->DoMapViewOnSize(itsMapViewDescTopIndex, NFmiPoint(rect.Width(), rect.Height()), deviceContextHandler.GetDcFromHandler());

        if(counter > 2)
            PutTextInStatusBar(CtrlViewUtils::MakeMapPortionPixelSizeStringForStatusbar(data->GetCombinedMapHandler()->getMapViewDescTop(itsMapViewDescTopIndex)->ActualMapBitmapSizeInPixels(), true));

        // Vain t�m� n�ytt� itse ja zoomaus dialogi pit�� p�ivitt��
		pDoc->UpdateAllViewsAndDialogs("Main map view resized", SmartMetViewId::MainMapView | SmartMetViewId::ZoomDlg);
	}
    counter++;
	Invalidate(FALSE);
}

void CSmartMetView::Update(void)
{
	CtrlView::DeviceContextHandler<CSmartMetView> deviceContextHandler(this);
	itsEditMapView->Update();
    Invalidate(FALSE);
}

void CSmartMetView::RefreshApplicationViewsAndDialogs(const std::string &reasonForUpdate, bool redrawMapView, bool clearMapViewBitmapCacheRows, int theWantedMapViewDescTop)
{
	if(redrawMapView)
	{
		if(theWantedMapViewDescTop == -1)
			GetDocument()->GetData()->GetCombinedMapHandler()->mapViewDirty(itsMapViewDescTopIndex, false, clearMapViewBitmapCacheRows, redrawMapView, false, false, false);
		else
			GetDocument()->GetData()->GetCombinedMapHandler()->mapViewDirty(theWantedMapViewDescTop, false, clearMapViewBitmapCacheRows, redrawMapView, false, false, false);
	}

	GetDocument()->UpdateAllViewsAndDialogs(reasonForUpdate);
}

void CSmartMetView::RefreshApplicationViewsAndDialogs(const std::string &reasonForUpdate, SmartMetViewId updatedViewsFlag, bool redrawMapView, bool clearMapViewBitmapCacheRows, int theWantedMapViewDescTop)
{
    if(redrawMapView)
    {
        if(theWantedMapViewDescTop == -1)
            GetDocument()->GetData()->GetCombinedMapHandler()->mapViewDirty(itsMapViewDescTopIndex, false, clearMapViewBitmapCacheRows, redrawMapView, false, false, false);
        else
            GetDocument()->GetData()->GetCombinedMapHandler()->mapViewDirty(theWantedMapViewDescTop, false, clearMapViewBitmapCacheRows, redrawMapView, false, false, false);
    }

    GetDocument()->UpdateAllViewsAndDialogs(reasonForUpdate, updatedViewsFlag);
}

void CSmartMetView::ActivateFilterDlg(void)
{
	GetDocument()->ActivateFilterDlg();
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
    bool ctrlOrShiftKeyDown = CtrlView::IsKeyboardKeyDown(VK_CONTROL) || CtrlView::IsKeyboardKeyDown(VK_SHIFT); // ctrl/shift avaimet eiv�t saa olla painettuina, jos on, t�ll�in ei nuolin�pp�imi� oteta huomioon, koska ne hoidetaan muualla keyboard-acceleraattoreilla
	// T�m� koodi on 'kopioitu' my�s karttaapuikkunoihin
	// CFmiExtraMapViewDlg::PreTranslateMessage
	switch(nChar)
	{
	case VK_LEFT:
		if(!ctrlOrShiftKeyDown && doc->GetCombinedMapHandler()->setDataToPreviousTime(itsMapViewDescTopIndex, false))
			GetDocument()->UpdateAllViewsAndDialogs("Main map view: left cursor key pressed");
		return;
	case VK_RIGHT:
		if(!ctrlOrShiftKeyDown && doc->GetCombinedMapHandler()->setDataToNextTime(itsMapViewDescTopIndex, false))
			GetDocument()->UpdateAllViewsAndDialogs("Main map view: right cursor key pressed");
		return;
	case VK_UP:
		if(!ctrlOrShiftKeyDown && doc->GetCombinedMapHandler()->scrollViewRow(itsMapViewDescTopIndex, -1))
			GetDocument()->UpdateAllViewsAndDialogs("Main map view: up cursor key pressed");
		return;
	case VK_DOWN:
		if(!ctrlOrShiftKeyDown && doc->GetCombinedMapHandler()->scrollViewRow(itsMapViewDescTopIndex, 1))
			GetDocument()->UpdateAllViewsAndDialogs("Main map view: down cursor key pressed");
		return;
	}

	CView::OnKeyUp(nChar, nRepCnt, nFlags);
}

// asettaa toolmasterin ja toolboxin DC:t
// esim. ennen piirto tai ennen kuin tarkastellaan hiiren klikkausta ruudulta
// HUOM!! printtauksen yhteydess� kutsu ensin DC:n asetus ja sitten printinfon!!!
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
	UINT iFile = 0xFFFFFFFF; // 1. tiputetun tiedoston nime� haetaan
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
	return FALSE;	// n�in ei tyhjennet� taustaa ennen ruudun piirtoa!!
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
	CtrlView::DeviceContextHandler<CSmartMetView> deviceContextHandler(this);
	CtrlView::ReleaseCtrlKeyIfStuck(nFlags); // t�m� vapauttaa CTRL-napin, jos se on 'jumiutunut' pohjaan (MFC bugi, ctrl-nappi voi j��d� pohjaan, jos kyseinen n�pp�in vapautetaan, ennen kuin kartta ruudun piirto on valmis)
	// Jostain syyst� MouseWheel systeemi antaa hiiren kursorin paikan
	// absoluuttisen pikseli sijainnin desctopissa. Nyt pit�� tehd� muunnos
	// kursorin paikka ikkunan omassa sijainnissa.
	CRect zview;
	GetWindowRect(zview);
	CPoint viewPoint(pt - zview.TopLeft());
	bool needsUpdate = itsEditMapView ? itsEditMapView->MouseWheel(itsToolBox->ToViewPoint(viewPoint.x, viewPoint.y)
		, itsToolBox->ConvertCtrlKey(nFlags), zDelta) : false;

	if(needsUpdate)
	{
		GetDocument()->UpdateAllViewsAndDialogs("Main map view: mouse wheel action");
	}
	return TRUE;
	//	return CView::OnMouseWheel(nFlags, zDelta, pt);
}

void CSmartMetView::OnMButtonDown(UINT nFlags, CPoint point)
{
	CtrlView::DeviceContextHandler<CSmartMetView> deviceContextHandler(this);
	bool needsUpdate = itsEditMapView ? itsEditMapView->MiddleButtonDown(itsToolBox->ToViewPoint(point.x, point.y)
		,itsToolBox->ConvertCtrlKey(nFlags)) : false;

	if(needsUpdate)
	{
		GetDocument()->UpdateAllViewsAndDialogs("Main map view: mouse middle button down");
	}
    SetCapture(); // otetaan hiiren liikkeet/viestit talteeen toistaiseksi t�h�n ikkunaan
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
		this->ClientToScreen(&buttRect); // muutetaan paikka absoluuttiseksi screenill�
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
			// t�m� saattaa toimia eri lailla eri Windowseilla (esim. XP vs. Vista)
			CRect buttRect = button->Rect(); 
			this->ClientToScreen(&buttRect); // muutetaan paikka absoluuttiseksi screenill�
			m_SizeWnd->Create(this, buttRect.left, buttRect.top + 1);
		}
	}
#endif // FMI_DISABLE_MFC_FEATURE_PACK
}

void CSmartMetView::UpdateMapView(unsigned int theDescTopIndex)
{
	// T�m�n voisi tehd� paljon hienostuneemminkin!!!
	CSmartMetView *view1 = dynamic_cast<CSmartMetView *>(GetDocument()->GetData()->GetCombinedMapHandler()->getMapViewDescTop(theDescTopIndex)->MapView());
	if(view1)
		view1->Update();
	else
	{
		CFmiExtraMapView *view2 = dynamic_cast<CFmiExtraMapView *>(GetDocument()->GetData()->GetCombinedMapHandler()->getMapViewDescTop(theDescTopIndex)->MapView());
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
		if(pDoc->GetData()->GetCombinedMapHandler()->setMapViewGrid(itsMapViewDescTopIndex, newSize))
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
// L�hes identtiset funktiot  NotifyDisplayTooltip ja DrawSynopPlotImage kahdessa luokassa:
// CSmartMetView ja CFmiExtraMapView. Mieti joska ne voisi yhdist�� k�ytt�m��n samaa koodia.
void CSmartMetView::NotifyDisplayTooltip(NMHDR * pNMHDR, LRESULT * result)
{
    CFmiWin32TemplateHelpers::NotifyDisplayTooltip(this, pNMHDR, result, GetSmartMetDocumentInterface(), MAINMAPVIEW_TOOLTIP_ID);
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
	itsToolBox->SetClientRect(CRect(0, 0, pBitMap.bmWidth, pBitMap.bmHeight)); // pakko tehd� t�ll�inen viritys, koska view:sta luotu dc on eri kokoinen kuin k�ytetty bitmapin koko

	// pit�� tyhjent�� bitmappi piirt�m�ll� valkoinen rect fillill�
	NFmiDrawingEnvironment envi;
	envi.EnableFill();
	envi.EnableFrame();
	envi.SetFillColor(NFmiColor(1,1,1));
	envi.SetFrameColor(NFmiColor(0.8f, 0.8f, 0.8f)); // piirret��n vaalean harmaalla reunus
	NFmiRect bitmapRect(0, 0, 1, 1);
	NFmiRectangle drawRect(bitmapRect, 0, &envi);
	itsToolBox->Convert(&drawRect);

	NFmiRect drawingrect = bitmapRect;

	drawingrect.Inflate(-0.23); // en tied� miksi, mutta t�ll�lailla pit�� piirto laatikkoa pienent��, muuten synop-plotti levi�� piirrett�ess� bitmappiin
	itsEditMapView->DrawSynopPlotOnToolTip(itsToolBox , drawingrect, fDrawSoundingPlot, fDrawMinMaxPlot, fDrawMetarPlot);
	itsSynopPlotBitmap = dcMem.SelectObject(oldBitmap);
	dcMem.DeleteDC();
}

void CSmartMetView::OnAcceleratorSwapArea()
{
	GetDocument()->GetData()->GetCombinedMapHandler()->swapArea(itsMapViewDescTopIndex);
	GetDocument()->UpdateAllViewsAndDialogs("Main map view: used map area swapped (SPACE)");
}

void CSmartMetView::OnAcceleratorSwapAreaSecondaryKey()
{
	GetDocument()->GetData()->GetCombinedMapHandler()->swapArea(itsMapViewDescTopIndex);
	GetDocument()->UpdateAllViewsAndDialogs("Main map view: used map area swapped (CTRL + SHIFT + SPACE -> secondary key)");
}

void CSmartMetView::OnAcceleratorMakeSwapBaseArea()
{
	GetDocument()->GetData()->GetCombinedMapHandler()->makeSwapBaseArea(itsMapViewDescTopIndex);
}

void CSmartMetView::OnEditCopy()
{
	GetDocument()->GetData()->GetCombinedMapHandler()->copyDrawParamsFromMapViewRow(itsMapViewDescTopIndex);
}

void CSmartMetView::OnEditPaste()
{
	GetDocument()->GetData()->GetCombinedMapHandler()->pasteDrawParamsToMapViewRow(itsMapViewDescTopIndex);
}

void CSmartMetView::OnAcceleratorCopyAllMapViewParams()
{
	GetDocument()->GetData()->GetCombinedMapHandler()->copyMapViewDescTopParams(itsMapViewDescTopIndex);
}

void CSmartMetView::OnAcceleratorPasteAllMapViewParams()
{
	GetDocument()->GetData()->GetCombinedMapHandler()->pasteMapViewDescTopParams(itsMapViewDescTopIndex);
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
	GetGeneralDoc()->GetCombinedMapHandler()->getMapViewDescTop(itsMapViewDescTopIndex)->RelativeMapRect(theRect);
}

void CSmartMetView::PrintViewSizeInPixels(const NFmiPoint &theSize)
{
	GetGeneralDoc()->GetCombinedMapHandler()->getMapViewDescTop(itsMapViewDescTopIndex)->MapViewSizeInPixels(theSize, itsToolBox->GetDC(), GetGeneralDoc()->ApplicationWinRegistry().DrawObjectScaleFactor(), true);
}

void CSmartMetView::SetPrintCopyCDC(CDC* pDC)
{
	GetGeneralDoc()->GetCombinedMapHandler()->getMapViewDescTop(itsMapViewDescTopIndex)->CopyCDC(pDC);
}

void CSmartMetView::MakePrintViewDirty(bool fViewDirty, bool fCacheDirty)
{
	GetGeneralDoc()->GetCombinedMapHandler()->getMapViewDescTop(itsMapViewDescTopIndex)->MapViewDirty(fViewDirty, fCacheDirty, true, false);
}

int CSmartMetView::CalcPrintingPageShiftInMinutes(void)
{
	return GetGeneralDoc()->GetCombinedMapHandler()->getMapViewDescTop(itsMapViewDescTopIndex)->CalcPrintingPageShiftInMinutes();
}

NFmiMetTime CSmartMetView::CalcPrintingStartTime(void)
{
	return GetGeneralDoc()->GetCombinedMapHandler()->currentTime(itsMapViewDescTopIndex);
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

void CSmartMetView::OnAcceleratorChangeTimeByStep1Forward()
{
	GetGeneralDoc()->GetCombinedMapHandler()->changeTime(1, kForward, itsMapViewDescTopIndex, 1);
}

void CSmartMetView::OnAcceleratorChangeTimeByStep1Backward()
{
	GetGeneralDoc()->GetCombinedMapHandler()->changeTime(1, kBackward, itsMapViewDescTopIndex, 1);
}

void CSmartMetView::OnAcceleratorChangeTimeByStep2Forward()
{
	GetGeneralDoc()->GetCombinedMapHandler()->changeTime(2, kForward, itsMapViewDescTopIndex, 1);
}

void CSmartMetView::OnAcceleratorChangeTimeByStep2Backward()
{
	GetGeneralDoc()->GetCombinedMapHandler()->changeTime(2, kBackward, itsMapViewDescTopIndex, 1);
}

void CSmartMetView::OnAcceleratorChangeTimeByStep3Forward()
{
	GetGeneralDoc()->GetCombinedMapHandler()->changeTime(3, kForward, itsMapViewDescTopIndex, 1);
}

void CSmartMetView::OnAcceleratorChangeTimeByStep3Backward()
{
	GetGeneralDoc()->GetCombinedMapHandler()->changeTime(3, kBackward, itsMapViewDescTopIndex, 1);
}

void CSmartMetView::OnAcceleratorChangeTimeByStep4Forward()
{
	GetGeneralDoc()->GetCombinedMapHandler()->changeTime(4, kForward, itsMapViewDescTopIndex, 1);
}

void CSmartMetView::OnAcceleratorChangeTimeByStep4Backward()
{
	GetGeneralDoc()->GetCombinedMapHandler()->changeTime(4, kBackward, itsMapViewDescTopIndex, 1);
}

void CSmartMetView::OnAcceleratorChangeAllModelDataOnRowToPreviousModelRun()
{
	GetGeneralDoc()->GetCombinedMapHandler()->setModelRunOffsetForAllModelDataOnActiveRow(itsMapViewDescTopIndex, kBackward);
}

void CSmartMetView::OnAcceleratorChangeAllModelDataOnRowToNextModelRun()
{
	GetGeneralDoc()->GetCombinedMapHandler()->setModelRunOffsetForAllModelDataOnActiveRow(itsMapViewDescTopIndex, kForward);
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

void CSmartMetView::OnDisplayChange(UINT, int, int)
{
	GetGeneralDoc()->OnButtonRefresh("Display settings have changed somehow, doing full update on all views");
}

void CSmartMetView::OnAcceleratorMapViewRangeMeterModeToggle()
{
	auto& rangeMeter = GetGeneralDoc()->ApplicationWinRegistry().ConfigurationRelatedWinRegistry().MapViewRangeMeter();
	rangeMeter.ModeOnToggle();
	// Kun rangeMeter laitetaan toimintaan, kannattaa karttojen tooltipin aukeamisaikaa laittaa pidemm�ksi, 
	// muuten tooltip kuplaa pit�� koko ajan klikata hiirell� ennen kuin p��see tekem��n mouse left-click-drag:ia
	ApplicationInterface::GetApplicationInterfaceImplementation()->SetAllMapViewTooltipDelays(!rangeMeter.ModeOn(), NFmiMapViewRangeMeterWinRegistry::TooltipDelayInMS);
	ApplicationInterface::GetApplicationInterfaceImplementation()->ForceDrawOverBitmapThings(itsMapViewDescTopIndex, true, true);
}

void CSmartMetView::OnAcceleratorMapViewRangeMeterColorToggle()
{
	auto& mapViewRangeMeter = GetGeneralDoc()->ApplicationWinRegistry().ConfigurationRelatedWinRegistry().MapViewRangeMeter();
	if(mapViewRangeMeter.ModeOn())
	{
		mapViewRangeMeter.ToggleColor();
		ApplicationInterface::GetApplicationInterfaceImplementation()->ForceDrawOverBitmapThings(itsMapViewDescTopIndex, true, true);
	}
}

void CSmartMetView::OnAcceleratorMapViewRangeMeterLockModeToggle()
{
	auto& mapViewRangeMeter = GetGeneralDoc()->ApplicationWinRegistry().ConfigurationRelatedWinRegistry().MapViewRangeMeter();
	if(mapViewRangeMeter.ModeOn())
	{
		mapViewRangeMeter.LockModeOnToggle();
		ApplicationInterface::GetApplicationInterfaceImplementation()->ForceDrawOverBitmapThings(itsMapViewDescTopIndex, true, true);
	}
}

// **********************************************************************************
// Eri hiiren napin k�sittelyn perus-implementaatio funktiot
// **********************************************************************************

void CSmartMetView::OnMouseMove_Implementation(UINT nFlags, CPoint point)
{
	NFmiEditMapGeneralDataDoc* genData = GetDocument()->GetData();
	if(genData->Printing())
		return;
	CtrlView::DeviceContextHandler<CSmartMetView> deviceContextHandler(this);
	CtrlView::ReleaseCtrlKeyIfStuck(nFlags); // t�m� vapauttaa CTRL-napin, jos se on 'jumiutunut' pohjaan (MFC bugi, ctrl-nappi voi j��d� pohjaan, jos kyseinen n�pp�in vapautetaan, ennen kuin kartta ruudun piirto on valmis)

	// laitetaan my�s karttabitmap valmiiksi osittaisia p�ivityksi� varten
	CClientDC dc(this);
	CDC dcMem;
	dcMem.CreateCompatibleDC(&dc);
	CBitmap* oldBitmap2 = dcMem.SelectObject(itsMapBitmap);
	auto* mapViewDescTop = genData->GetCombinedMapHandler()->getMapViewDescTop(itsMapViewDescTopIndex);
	mapViewDescTop->MapBlitDC(&dcMem);

	NFmiPoint viewPoint(itsToolBox->ToViewPoint(point.x, point.y));

	// HUOM! jos pensselill� muokataan l�pin�kyvyydell� piirrett�v�� dataa, pit�� l�pin�kyvyys n�ytt� asettaa k�ytt��n
	genData->TransparencyContourDrawView(this); // t�m� on osa kikkaa, jolla saadaan piiiretty� l�pin�kyvi� kentti� toolmasterilla tai imaginella
	bool needsUpdate = false;
	try
	{
		needsUpdate = itsEditMapView ? itsEditMapView->MouseMove(viewPoint, itsToolBox->ConvertCtrlKey(nFlags)) : false;
	}
	catch(...)
	{
	}
	genData->TransparencyContourDrawView(0); // lopuksi se asetetaan taas 0-pointteriksi...

	mapViewDescTop->MapBlitDC(0);
	dcMem.SelectObject(oldBitmap2);
	dcMem.DeleteDC();

	bool drawOverBitmapAnyway = genData->EditedPointsSelectionChanged(false); // pyydet��n vanha arvo drawOverBitmapAnyway -muuttujaan ja asetetaan samalla false:ksi
	if(itsEditMapView->MapRect().IsInside(viewPoint))
	{
		NFmiPoint cursorLatlon(genData->ToolTipLatLonPoint());
		std::string str = CtrlViewUtils::GetTotalMapViewStatusBarStr(&genData->GetCtrlViewDocumentInterface(), cursorLatlon);
		// Laitetaan kartta osion pikseli koko t�h�n per��n sulkuihin
		str += CtrlViewUtils::MakeMapPortionPixelSizeStringForStatusbar(mapViewDescTop->ActualMapBitmapSizeInPixels(), false);

		// Jos CTRL-nappi on pohjassa, lis�t��n viel� osoitetun karttaruudun aktiivisen datan joko hilapiste indeksit tai havaintoaseman location id.
		if(CtrlView::IsKeyboardKeyDown(VK_CONTROL))
			str += MakeActiveDataLocationIndexString(cursorLatlon);

		PutTextInStatusBar(str);

		// P�ivitet��n my�s synop data teksti muodossa taulukossa ikkunaa, eli
		// l�hin asema scrollataan n�kyviin ja tehd��n se valituksi
		if(genData->SynopDataGridViewOn())
		{
			if(nFlags & MK_CONTROL && genData->MouseCaptured() == false)
			{// tehd��n l�himpien havainto asemien seuranta vain CTRL pohjassa ja kun hiiri ei ole kaapattu
				if(itsEditMapView->MapRect().IsInside(viewPoint))
				{ // ja vain jos ollaan karttan�yt�n p��ll�, ei kun ollaan aikakontrolli ikkunan p��ll�
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
	else if(mapViewDescTop->MapViewBitmapDirty())
		Invalidate(FALSE);
	else if(!(genData->MiddleMouseButtonDown() && genData->MouseCaptured())) // muuten ForceDrawOverBitmapThings, paitsi jos ollaan vet�m�ss� kartan p��lle zoomi laatikkoa, koska se peittyisi aina ForceDrawOverBitmapThings:n alle
	{
		bool doIncreaseCurrentUpdateId = false;
		bool rangeMeterModeOn = genData->ApplicationWinRegistry().ConfigurationRelatedWinRegistry().MapViewRangeMeter().ModeOn();

		if(genData->ShowMouseHelpCursorsOnMap() || drawOverBitmapAnyway || rangeMeterModeOn)
		{
			// T��lt� ei haluta doIncreaseCurrentUpdateId asetusta, koska ei kiinnosta, jos kartalla p�ivitet��n apukursoreita
			ForceDrawOverBitmapThings(itsMapViewDescTopIndex, true, true); // hiiren apukursorit pit�� joka tapauksessa piirt�� aina
		}
		if(genData->MustDrawTempView())
		{
			if(genData->GetMTATempSystem().TempViewOn())
				doIncreaseCurrentUpdateId = true;
			GetDocument()->UpdateTempView();
			// T�m� nollataan vasta piirron j�lkeen, koska piirrossa kiinnostaa ett� menn��nk� sinne mouse-move operaatiosta vai mist�.
			genData->MustDrawTempView(false);
		}
		if(genData->MustDrawTimeSerialView())
		{
			if(genData->TimeSerialDataViewOn())
				doIncreaseCurrentUpdateId = true;
			genData->MustDrawTimeSerialView(false);
			GetDocument()->UpdateTimeSerialView();
		}
		if(genData->MustDrawCrossSectionView())
		{
			if(genData->CrossSectionSystem()->CrossSectionViewOn())
				doIncreaseCurrentUpdateId = true;
			genData->MustDrawCrossSectionView(false);
			GetDocument()->UpdateCrossSectionView();
			ForceDrawOverBitmapThings(itsMapViewDescTopIndex, true, false);
		}

		// Kasvatetaan ruutujenp�ivityslaskuria, jos mouse-move operaatio on aiheuttanut muiden kuin karttan�yt�n p�ivityksi� (tosin j�lkij�tt�isesti)
		if(doIncreaseCurrentUpdateId)
			CtrlViewUtils::CtrlViewTimeConsumptionReporter::increaseCurrentUpdateId();
	}
}

void CSmartMetView::OnLButtonDown_Implementation(UINT nFlags, CPoint point)
{
	CtrlView::DeviceContextHandler<CSmartMetView> deviceContextHandler(this);
	bool needsUpdate = itsEditMapView ? itsEditMapView->LeftButtonDown(itsToolBox->ToViewPoint(point.x, point.y)
		, itsToolBox->ConvertCtrlKey(nFlags)) : false;

	if(needsUpdate)
	{
		GetDocument()->UpdateAllViewsAndDialogs("Main map view: left mouse button was pressed down");
	}
	SetCapture(); // otetaan hiiren liikkeet/viestit talteeen toistaiseksi t�h�n ikkunaan
}

void CSmartMetView::OnLButtonUp_Implementation(UINT nFlags, CPoint point)
{
	try // try-catch -blokilla varmistetaan ett� lopuksi ReleaseCapture todellakin kutsutaan
	{
		CtrlView::DeviceContextHandler<CSmartMetView> deviceContextHandler(this);
		CtrlView::ReleaseCtrlKeyIfStuck(nFlags); // t�m� vapauttaa CTRL-napin, jos se on 'jumiutunut' pohjaan (MFC bugi, ctrl-nappi voi j��d� pohjaan, jos kyseinen n�pp�in vapautetaan, ennen kuin kartta ruudun piirto on valmis)

		bool needsUpdate = itsEditMapView ? itsEditMapView->LeftButtonUp(itsToolBox->ToViewPoint(point.x, point.y)
			, itsToolBox->ConvertCtrlKey(nFlags)) : false;	// M.K. 29.4.99 Lis�sin "parametrivalintalaatikon" piirt�mist� varten.
		Invalidate(FALSE);
		if(needsUpdate)
		{
			GetDocument()->UpdateAllViewsAndDialogs("Main map view: left mouse button was released");
			ForceDrawOverBitmapThings(itsMapViewDescTopIndex, false, true);
		}
	}
	catch(...)
	{
		ReleaseCapture(); // vapautetaan lopuksi hiiren viestit muidenkin ikkunoiden k�ytt��n
		throw; // laitetaan poikkeus eteenp�in
	}
	ReleaseCapture(); // vapautetaan lopuksi hiiren viestit muidenkin ikkunoiden k�ytt��n (OnLButtonDown:issa laitettiin SetCapture p��lle)

}

void CSmartMetView::OnLButtonDblClk_Implementation(UINT nFlags, CPoint point)
{
	CtrlView::DeviceContextHandler<CSmartMetView> deviceContextHandler(this);
	CtrlView::ReleaseCtrlKeyIfStuck(nFlags); // t�m� vapauttaa CTRL-napin, jos se on 'jumiutunut' pohjaan (MFC bugi, ctrl-nappi voi j��d� pohjaan, jos kyseinen n�pp�in vapautetaan, ennen kuin kartta ruudun piirto on valmis)

	bool needsUpdate = itsEditMapView ? itsEditMapView->LeftDoubleClick(itsToolBox->ToViewPoint(point.x, point.y)
		, itsToolBox->ConvertCtrlKey(nFlags)) : false;
	if(needsUpdate)
	{
		CSmartMetDoc* pDoc = GetDocument();
		if(pDoc)
		{
			if(pDoc->GetData()->ActivateParamSelectionDlgAfterLeftDoubleClick())
			{
				pDoc->GetData()->ActivateParamSelectionDlgAfterLeftDoubleClick(false);
				pDoc->ActivateParameterSelectionDlg();
				return;
			}
		}
		Invalidate(FALSE);
		GetDocument()->UpdateAllViewsAndDialogs("Main map view: left mouse button double click");
	}
	//	CView::OnLButtonDblClk(nFlags, point);
}

// **********************************************************************************
// Eri hiiren napin k�sittelyn varsinaiset funktiot
// **********************************************************************************

void CSmartMetView::OnLButtonDown(UINT nFlags, CPoint point)
{
	itsCurrentOpenUrlAction = CtrlView::GetOpenUrlKeyPressedState();
	if(!CtrlView::HandleUrlMouseActions(itsCurrentOpenUrlAction))
	{
		// Tehd��n mouse-down vain jos ei olla tekem�ss� url-action juttuja, t�ll�in ei 
		// haluta tehd� mit��n mouse-drag juttuja, joiden setup tehd��n mouse-down k�sittelyiss�.
		OnLButtonDown_Implementation(nFlags, point);
	}
}

void CSmartMetView::OnLButtonUp(UINT nFlags, CPoint point)
{
	// Normi mouse-up jutut tehd��n my�s silloin kun on tarkoitus tehd� 
	// url-action juttuja, koska haluamme valita kartalta normi k�ytt��n
	// saman pisteen kuin t�lle url-actionillekin.
	// 1. Ensin valitaan klikattu piste.
	OnLButtonUp_Implementation(nFlags, point);
	if(CtrlView::HandleUrlMouseActions(itsCurrentOpenUrlAction))
	{
		// 2. Sitten mahdollisesti hanskataan se klikatun pisteen url-action
		CtrlView::OpenWantedUrlInBrowser(itsCurrentOpenUrlAction, itsMapViewDescTopIndex);
	}
	// 3. Lopuksi aina nollataan menossa oleva url-action asetus
	itsCurrentOpenUrlAction = SmartMetOpenUrlAction::None;
}

void CSmartMetView::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	// Ei tehd�k��n mit��n url-action juttuja jos tupla-klikkaus, koska
	// se saattoi olla vahinko ja turha avata paria tabia sekunnin sis��n 
	// selaimeen vahinkolaukauksesta.
	OnLButtonDblClk_Implementation(nFlags, point);
}

void CSmartMetView::OnRButtonDown(UINT nFlags, CPoint point)
{
	CtrlView::DeviceContextHandler<CSmartMetView> deviceContextHandler(this);
	bool needsUpdate = itsEditMapView ? itsEditMapView->RightButtonDown(itsToolBox->ToViewPoint(point.x, point.y)
		, itsToolBox->ConvertCtrlKey(nFlags)) : false;

	if(needsUpdate)
	{
		GetDocument()->UpdateAllViewsAndDialogs("Main map view: right mouse button was pressed down");
	}
}

void CSmartMetView::OnRButtonUp(UINT nFlags, CPoint point)
{
	CtrlView::DeviceContextHandler<CSmartMetView> deviceContextHandler(this);
	CtrlView::ReleaseCtrlKeyIfStuck(nFlags); // t�m� vapauttaa CTRL-napin, jos se on 'jumiutunut' pohjaan (MFC bugi, ctrl-nappi voi j��d� pohjaan, jos kyseinen n�pp�in vapautetaan, ennen kuin kartta ruudun piirto on valmis)

	bool needsUpdate = itsEditMapView ? itsEditMapView->RightButtonUp(itsToolBox->ToViewPoint(point.x, point.y)
		, itsToolBox->ConvertCtrlKey(nFlags)) : false;

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
		pPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x + tempRect.left, point.y + tempRect.top, this);
		genData->OpenPopupMenu(false);
	}
	else if(needsUpdate)
	{
		GetDocument()->UpdateAllViewsAndDialogs("Main map view: right mouse button was released");
		ForceDrawOverBitmapThings(itsMapViewDescTopIndex, false, true);
	}
}

void CSmartMetView::OnRButtonDblClk(UINT nFlags, CPoint point)
{
	CtrlView::DeviceContextHandler<CSmartMetView> deviceContextHandler(this);
	CtrlView::ReleaseCtrlKeyIfStuck(nFlags); // t�m� vapauttaa CTRL-napin, jos se on 'jumiutunut' pohjaan (MFC bugi, ctrl-nappi voi j��d� pohjaan, jos kyseinen n�pp�in vapautetaan, ennen kuin kartta ruudun piirto on valmis)

	bool needsUpdate = itsEditMapView ? itsEditMapView->RightDoubleClick(itsToolBox->ToViewPoint(point.x, point.y)
		, itsToolBox->ConvertCtrlKey(nFlags)) : false;
	if(needsUpdate)
	{
		Invalidate(FALSE);
		GetDocument()->UpdateAllViewsAndDialogs("Main map view: right mouse button double click");
	}
	//    CView::OnRButtonDblClk(nFlags, point);
}

void CSmartMetView::OnMouseMove(UINT nFlags, CPoint point)
{
	// Mouse-move juttuja hanskataan vain jos ei ole menossa url-action juttuja.
	// Ei haluta mouse drag ja muita vastaavia k�sittelyj�, jos k�ytt�j� haluaa
	// vain avata button-up k�sittelyssa valitun pisteen url-actionin.
	if(!CtrlView::HandleUrlMouseActions(itsCurrentOpenUrlAction))
	{
		OnMouseMove_Implementation(nFlags, point);
	}
}
