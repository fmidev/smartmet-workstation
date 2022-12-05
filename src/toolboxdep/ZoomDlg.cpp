// ZoomDlg.cpp : implementation file
//
#ifdef _MSC_VER
#pragma warning(disable : 4786) // poistaa n kpl VC++ kääntäjän varoitusta (liian pitkä nimi >255 merkkiä joka johtuu 'puretuista' STL-template nimistä)
#endif

#include "stdafx.h"
#include "ZoomDlg.h"
#include "ZoomView.h"
#include "SmartMetDocumentInterface.h"
#include "NFmiGdiPlusImageMapHandler.h"

#include "NFmiDictionaryFunction.h"
#include "FmiWin32TemplateHelpers.h"
#include "NFmiMapViewDescTop.h"
#include "FmiWin32Helpers.h"
#include "NFmiApplicationWinRegistry.h"
#include "persist2.h"
#include "NFmiArea.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

const unsigned int gNonMapViewWasSelectedIndex = 99999;

/////////////////////////////////////////////////////////////////////////////
// CZoomDlg dialog
const NFmiViewPosRegistryInfo CZoomDlg::s_ViewPosRegistryInfo(CRect(400, 100, 800, 600), "\\ZoomView");

CZoomDlg::CZoomDlg(SmartMetDocumentInterface *smartMetDocumentInterface, CWnd* pParent)
: CDialog(CZoomDlg::IDD, pParent)
,itsClientView(0)
,itsSmartMetDocumentInterface(smartMetDocumentInterface)
, fMapView1Enabled(TRUE)
, fMapView2Enabled(FALSE)
, fMapView3Enabled(FALSE)
, fAllowMixedMapAreaZoom(FALSE)
{
//	itsMapHandler->SaveState();
	//{{AFX_DATA_INIT(CZoomDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

void CZoomDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CZoomDlg)
	// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
	DDX_Check(pDX, IDC_CHECK_MAP_VIEW1_ENABLED, fMapView1Enabled);
	DDX_Check(pDX, IDC_CHECK_MAP_VIEW2_ENABLED, fMapView2Enabled);
	DDX_Check(pDX, IDC_CHECK_MAP_VIEW3_ENABLED, fMapView3Enabled);
	DDX_Check(pDX, IDC_CHECK_ALLOW_MIXED_MAP_AREA_ZOOMING, fAllowMixedMapAreaZoom);
}


BEGIN_MESSAGE_MAP(CZoomDlg, CDialog)
	//{{AFX_MSG_MAP(CZoomDlg)
	ON_WM_SIZE()
	ON_BN_CLICKED(ID_SHOW_ZOOMED_AREA, OnShowZoomedArea)
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_CHECK_MAP_VIEW1_ENABLED, &CZoomDlg::OnBnClickedCheckMapView1Enabled)
	ON_BN_CLICKED(IDC_CHECK_MAP_VIEW2_ENABLED, &CZoomDlg::OnBnClickedCheckMapView2Enabled)
	ON_BN_CLICKED(IDC_CHECK_MAP_VIEW3_ENABLED, &CZoomDlg::OnBnClickedCheckMapView3Enabled)
	ON_BN_CLICKED(IDC_CHECK_ALLOW_MIXED_MAP_AREA_ZOOMING, &CZoomDlg::OnBnClickedCheckAllowMixedMapAreaZooming)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CZoomDlg message handlers

BOOL CZoomDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	// TODO: Add extra initialization here

	std::string errorBaseStr("Error in CZoomDlg::OnInitDialog while reading dialog size and position values");
    CFmiWin32TemplateHelpers::DoWindowSizeSettingsFromWinRegistry(itsSmartMetDocumentInterface->ApplicationWinRegistry(), this, false, errorBaseStr, 0);

// tämä asettaa FMI tunnuksen (IL -pallukka joka on talletettu IDR_MAINFRAME2 -nimiseksi ikoniksi resursseihim)
//	HICON hIcon = LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDR_MAINFRAME2));
//	this->SetIcon(hIcon, FALSE);

	InitDialogTexts();
	MoveMapArea();
	CRect mapRect(CalcMapArea());
	mapRect.InflateRect(-1,-1,-1,-1);
	itsClientView = new CZoomView(this, itsSmartMetDocumentInterface);
	itsClientView->Create(NULL,NULL, WS_VISIBLE | WS_CHILD, mapRect, this, NULL);
	itsClientView->OnInitialUpdate(); // pitää kutsua erikseen, koska formvieta ei ole sidottu dokumenttiin

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CZoomDlg::OnSize(UINT nType, int cx, int cy) 
{
	static int firstTime = TRUE;
	CDialog::OnSize(nType, cx, cy);
	
	// TODO: Add your message handler code here

// laitoin tämän toistaiseksi OnSize()-metodiin
	if(!firstTime) // ei suoriteta tätä osaa ennen OnInitialDialog-Metodia
	{
		MoveMapArea();
	}
	else
		firstTime = FALSE;
}

void CZoomDlg::OnOK() 
{
	Zoom();
	CDialog::OnOK();
}

void CZoomDlg::MoveMapArea()
{
	CWnd *win = GetDlgItem(IDC_STATIC_CLIENT_AREA);
	CRect rect(CalcMapArea());
	if(win)
		win->MoveWindow(rect);
	rect.InflateRect(-1,-1,-1,-1);
	if(itsClientView)
		itsClientView->MoveWindow(rect);
}

unsigned int CZoomDlg::GetFirstSelectedMapViewIndex()
{
	if(fMapView1Enabled)
		return 0;
	if(fMapView2Enabled)
		return 1;
	if(fMapView3Enabled)
		return 2;
	return gNonMapViewWasSelectedIndex;
}

int CZoomDlg::CalcClientRectTopYPos()
{
	auto ctrlWin = GetDlgItem(IDC_CHECK_ALLOW_MIXED_MAP_AREA_ZOOMING);
	if(ctrlWin)
	{
		WINDOWPLACEMENT wplace;
		ctrlWin->GetWindowPlacement(&wplace);
		return wplace.rcNormalPosition.bottom + 1;
	}
	const int pixelRoomForButtons = 35; // room for all constrols at top of view
	return pixelRoomForButtons;
}

CRect CZoomDlg::CalcMapArea()
{
	auto yPos = CalcClientRectTopYPos();
	const int roomForEdges = 3; // map will occupy all clientarea except this from the edges
	CRect clientRect;
	GetClientRect(clientRect);
	CRect mapRect(clientRect.left + roomForEdges,
				  clientRect.top + yPos,
				  clientRect.right - roomForEdges,
				  clientRect.bottom - roomForEdges);

    if(itsSmartMetDocumentInterface && itsSmartMetDocumentInterface->ApplicationWinRegistry().KeepMapAspectRatio())
	{
		auto selectedMapViewIndex = GetFirstSelectedMapViewIndex();
		if(selectedMapViewIndex != gNonMapViewWasSelectedIndex)
		{
			NFmiRect rect2(mapRect.left, mapRect.top, mapRect.right, mapRect.bottom);
			double aspectRatio = itsSmartMetDocumentInterface->MapViewDescTop(selectedMapViewIndex)->MapHandler()->BitmapAspectRatio();
			rect2.AdjustAspectRatio(aspectRatio, true, kTopLeft);
			CRect newRect((int)rect2.Left(), (int)rect2.Top(), (int)rect2.Right(), (int)rect2.Bottom());
			mapRect = newRect;
		}
	}

	return mapRect;
}

CZoomDlg::~CZoomDlg()
{
}

void CZoomDlg::Update(void)
{
    if(IsWindowVisible() && !IsIconic()) // Näyttöä päivitetään vain jos se on näkyvissä ja se ei ole minimized tilassa
    {
        UpdateData(TRUE);
		auto selectedMapViewIndex = GetFirstSelectedMapViewIndex();
		if(selectedMapViewIndex != gNonMapViewWasSelectedIndex)
		{
			if(itsClientView)
				((CZoomView*)itsClientView)->Update(selectedMapViewIndex);
		}
        MoveMapArea();
        Invalidate(FALSE);
    }
}

void CZoomDlg::OnShowZoomedArea() 
{
	Zoom();
}

void CZoomDlg::Zoom(void)
{
	Zoom(0, fMapView1Enabled);
	Zoom(1, fMapView2Enabled);
	Zoom(2, fMapView3Enabled);
	itsSmartMetDocumentInterface->RefreshApplicationViewsAndDialogs(__FUNCTION__);
}

bool CZoomDlg::CheckIfMapAreaZoomIsAllowed(unsigned int selectedMapViewIndex)
{
	auto mapViewIndexUsedInZooming = GetFirstSelectedMapViewIndex();
	if(selectedMapViewIndex == mapViewIndexUsedInZooming)
		return true;

	auto zoomingMapAreaIndex = itsSmartMetDocumentInterface->MapViewDescTop(mapViewIndexUsedInZooming)->SelectedMapIndex();
	auto selectedMapAreaIndex = itsSmartMetDocumentInterface->MapViewDescTop(selectedMapViewIndex)->SelectedMapIndex();
	if(fAllowMixedMapAreaZoom)
	{
		// Sallittu, jos zoomaus kartan peruskartta-alue on zoomatun peruskartta-alueen sisällä
		auto zoomingBaseMapArea = itsSmartMetDocumentInterface->GetMapHandlerInterface(mapViewIndexUsedInZooming)->TotalArea();
		auto selectedMapBaseMapArea = itsSmartMetDocumentInterface->GetMapHandlerInterface(selectedMapViewIndex)->TotalArea();
		bool zoomingAllowed = selectedMapBaseMapArea->IsInside(*zoomingBaseMapArea);
		if(!zoomingAllowed)
		{
			std::string logStr = "Zooming is not allowed for map-view-";
			logStr += std::to_string(selectedMapViewIndex + 1);
			logStr += " because map-view-";
			logStr += std::to_string(mapViewIndexUsedInZooming + 1);
			logStr += " has different base map area and the  these two areas won't cover each others correctly and zooming might cause problems";
			CatLog::logMessage(logStr, CatLog::Severity::Debug, CatLog::Category::Visualization);
		}
		return zoomingAllowed;
	}

	// Sallittua vain jos eri kartoilla on samat peruskartta-alueet käytössä
	bool zoomingAllowed = (zoomingMapAreaIndex == selectedMapAreaIndex);
	if(!zoomingAllowed)
	{
		std::string logStr = "Zooming is not allowed for map-view-";
		logStr += std::to_string(selectedMapViewIndex + 1);
		logStr += " because map-view-";
		logStr += std::to_string(mapViewIndexUsedInZooming + 1);
		logStr += " has different base map area and the \"Allow mixed map area zoom\" option is not set on";
		CatLog::logMessage(logStr, CatLog::Severity::Debug, CatLog::Category::Visualization);
	}
	return zoomingAllowed;
}

void CZoomDlg::Zoom(unsigned int selectedMapViewIndex, BOOL mapViewEnabled)
{
	if(mapViewEnabled)
	{
		if(CheckIfMapAreaZoomIsAllowed(selectedMapViewIndex))
		{
			std::string logStr = "Zooming map view ";
			logStr += std::to_string(selectedMapViewIndex + 1);
			logStr += " with area: ";
			auto zoomedArea = ((CZoomView*)itsClientView)->ZoomedArea();
			logStr += zoomedArea->AreaStr();
			CatLog::logMessage(logStr, CatLog::Severity::Info, CatLog::Category::Visualization);
			itsSmartMetDocumentInterface->SetMapArea(selectedMapViewIndex, zoomedArea);
		}
	}
}

void CZoomDlg::SetDefaultValues(void)
{
    MoveWindow(CZoomDlg::ViewPosRegistryInfo().DefaultWindowRect());
    Persist2::WriteWindowRectToWinRegistry(itsSmartMetDocumentInterface->ApplicationWinRegistry(), MakeUsedWinRegistryKeyStr(0), this);
}

// Tämä funktio alustaa kaikki dialogin tekstit editoriin valitulla kielellä.
// Tämä on ikävä kyllä tehtävä erikseen dialogin muokkaus työkalusta, eli
// tekijän pitää lisätä erikseen tänne kaikki dialogin osat, joihin 
// kieli valinta voi vaikuttaa.
void CZoomDlg::InitDialogTexts(void)
{
	SetWindowText(CA2T(::GetDictionaryString("ZoomDlgTitle").c_str()));
	CFmiWin32Helpers::SetDialogItemText(this, IDOK, "IDOK");
	CFmiWin32Helpers::SetDialogItemText(this, IDCANCEL, "IDCANCEL");
	CFmiWin32Helpers::SetDialogItemText(this, ID_SHOW_ZOOMED_AREA, "ID_SHOW_ZOOMED_AREA");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_ZOOM_MAP_VIEW_INDEX_STR, "Used map view indexes");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_CHECK_ALLOW_MIXED_MAP_AREA_ZOOMING, "Allow mixed map area zoom");
}

void CZoomDlg::ActivateMapViewDescTop(int theDescTopIndex)
{
	if(theDescTopIndex == 0)
		fMapView1Enabled = TRUE;
	if(theDescTopIndex == 1)
		fMapView2Enabled = TRUE;
	if(theDescTopIndex == 2)
		fMapView3Enabled = TRUE;

	UpdateData(FALSE);
	Update();
	SetActiveWindow();
}

void CZoomDlg::OnBnClickedCheckMapView1Enabled()
{
	UpdateData(TRUE);
	Update();
}

void CZoomDlg::OnBnClickedCheckMapView2Enabled()
{
	UpdateData(TRUE);
	Update();
}

void CZoomDlg::OnBnClickedCheckMapView3Enabled()
{
	UpdateData(TRUE);
	Update();
}


void CZoomDlg::OnBnClickedCheckAllowMixedMapAreaZooming()
{
	UpdateData(TRUE);
}
