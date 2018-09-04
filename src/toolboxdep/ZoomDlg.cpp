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

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CZoomDlg dialog
const NFmiViewPosRegistryInfo CZoomDlg::s_ViewPosRegistryInfo(CRect(400, 100, 800, 600), "\\ZoomView");

CZoomDlg::CZoomDlg(SmartMetDocumentInterface *smartMetDocumentInterface, CWnd* pParent)
: CDialog(CZoomDlg::IDD, pParent)
,itsClientView(0)
,itsSmartMetDocumentInterface(smartMetDocumentInterface)
,itsSelectedMapViewDescTopIndex(0)
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
	DDX_Radio(pDX, IDC_RADIO_ZOOM_MAP_VIEW1, itsSelectedMapViewDescTopIndex);
}


BEGIN_MESSAGE_MAP(CZoomDlg, CDialog)
	//{{AFX_MSG_MAP(CZoomDlg)
	ON_WM_SIZE()
	ON_BN_CLICKED(ID_SHOW_ZOOMED_AREA, OnShowZoomedArea)
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_RADIO_ZOOM_MAP_VIEW1, OnBnClickedRadioZoomMapView1)
	ON_BN_CLICKED(IDC_RADIO_ZOOM_MAP_VIEW2, OnBnClickedRadioZoomMapView2)
	ON_BN_CLICKED(IDC_RADIO_ZOOM_MAP_VIEW3, OnBnClickedRadioZoomMapView3)
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

CRect CZoomDlg::CalcMapArea()
{
	const int pixelRoomForButtons = 35; // room for OK and Cancel buttons
	const int roomForEdges = 5; // map will occupy all clientarea except this from the edges
	CRect clientRect;
	GetClientRect(clientRect);
	CRect mapRect(clientRect.left + roomForEdges,
				  clientRect.top + pixelRoomForButtons,
				  clientRect.right - roomForEdges,
				  clientRect.bottom - roomForEdges);

    if(itsSmartMetDocumentInterface && itsSmartMetDocumentInterface->ApplicationWinRegistry().KeepMapAspectRatio())
	{
		NFmiRect rect2(mapRect.left, mapRect.top, mapRect.right, mapRect.bottom);
		double aspectRatio = itsSmartMetDocumentInterface->MapViewDescTop(itsSelectedMapViewDescTopIndex)->MapHandler()->BitmapAspectRatio();
		rect2.AdjustAspectRatio(aspectRatio, true, kTopLeft);
		CRect newRect((int)rect2.Left(), (int)rect2.Top(), (int)rect2.Right(), (int)rect2.Bottom());
		mapRect = newRect;
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
        if(itsClientView)
            ((CZoomView*)itsClientView)->Update(itsSelectedMapViewDescTopIndex);
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
    itsSmartMetDocumentInterface->AreaViewDirty(itsSelectedMapViewDescTopIndex, true, true);
    itsSmartMetDocumentInterface->MapViewDescTop(itsSelectedMapViewDescTopIndex)->BorderDrawDirty(true);
    CatLog::logMessage("Zooming map view.", CatLog::Severity::Info, CatLog::Category::Visualization);
    itsSmartMetDocumentInterface->SetMapArea(itsSelectedMapViewDescTopIndex, ((CZoomView*)itsClientView)->ZoomedArea());
    itsSmartMetDocumentInterface->RefreshApplicationViewsAndDialogs(__FUNCTION__);
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
	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_ZOOM_MAP_VIEW_INDEX_STR, "Map view indexx");
}

void CZoomDlg::OnBnClickedRadioZoomMapView1()
{
	Update();
}

void CZoomDlg::OnBnClickedRadioZoomMapView2()
{
	Update();
}

void CZoomDlg::OnBnClickedRadioZoomMapView3()
{
	Update();
}

void CZoomDlg::ActivateMapViewDescTop(int theDescTopIndex)
{
	itsSelectedMapViewDescTopIndex = theDescTopIndex;
	UpdateData(FALSE);
	Update();
	SetActiveWindow();
}
