
// MainFrm.cpp : implementation of the CMainFrame class
//

#include "stdafx.h"
#include "SmartMet.h"
#include "MainFrm.h"
#include "SmartMetDoc.h"
#include "SmartMetView.h"
#include "ZoomDlg.h"
#include "NFmiEditMapGeneralDataDoc.h"
#include "CloseDlg.h"
#include "NFmiSmartInfo.h"
#include "NFmiMetEditorModeDataWCTR.h"
#include "NFmiMetEditorOptionsData.h"
#include "NFmiDictionaryFunction.h"
#include "NFmiFileCleanerSystem.h"
#include "FmiWin32TemplateHelpers.h"
#include "FmiWin32Helpers.h"
#include "NFmiQueryData.h"
#include "NFmiDataNotificationSettingsWinRegistry.h"
#include "FmiCombineDataThread.h"
#include "FmiMacroParamUpdateThread.h"
#include "SmartMetThreads_resource.h"
#include "QueryDataToLocalCacheLoaderThread.h"
#include "FmiDataLoadingThread2.h"
#include "NFmiHelpDataInfo.h"
#include "NFmiApplicationDataBase.h"
#include "FmiAppDataToDbThread.h"
#include "NFmiMacroPathSettings.h"
#include "NFmiApplicationWinRegistry.h"
#include "NFmiSatelliteImageCacheSystem.h"
#include "HakeMessage/Main.h"
#include "HakeMessage/HakeSystemConfigurations.h"
#include "CtrlViewWin32Functions.h"
#include "CtrlViewFunctions.h"
#include "FmiHakeWarningMessages.h"
#include "NFmiLedLightStatus.h"
#include "CtrlViewGdiPlusFunctions.h"
#include <numeric>

#ifndef DISABLE_CPPRESTSDK
#include "WmsSupport.h"
#endif // DISABLE_CPPRESTSDK

#include "ApplicationInterface.h"
#include "persist2.h"

#include "commctrl.h"
#include "ntray.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace
{
    // Optimizing view updates so that system won't update all views and dialogs every time when new data is loaded in to system.
    // This is done by starting timer when new data is read (to hold some time before doing actual updates on views). So when in the waiting period a new data
    // is read, that won't fire new update views action.

    // If this flag is true, don't start new new-query-data-read-update-views-timer
    std::atomic_flag g_NewQueryDataReadUpdateViewsTimerFlag = ATOMIC_FLAG_INIT;
    // When timer is started with kFmiNewQueryDataReadUpdateViewsTimer, to here is stored the dynamic timer id that is used to kill the timer after updates.
    UINT g_NewQueryDataReadUpdateViewsTimerId;
    // Here is stored each new query data file name for final updateAllViewsAndDialogs reasonForUpdate message.
    std::string g_NewQueryDataReadList;
}

static const int STATUSBAR_TOOLTIP_ID = 1234642;

IMPLEMENT_DYNCREATE(CStatusBarResize , CStatusBar)

BEGIN_MESSAGE_MAP(CStatusBarResize, CStatusBar)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_NOTIFY(UDM_TOOLTIP_DISPLAY, NULL, NotifyDisplayTooltip)
END_MESSAGE_MAP()

CStatusBarResize::CStatusBarResize()
:CStatusBar()
{
}

void CStatusBarResize::Setup(NFmiLedLightStatusSystem* ledLightStatusSystem, int totalNumberOfPanes)
{ 
	if(!ledLightStatusSystem)
	{
		std::string errorMessage = __FUNCTION__;
		errorMessage += ": given ledLightStatusSystem was nullptr, won't be able to do led control tooltips";
		CatLog::logMessage(errorMessage, CatLog::Severity::Error, CatLog::Category::Configuration, true);
		return;
	}
	if(ledLightStatusSystem->IsInitilized())
	{
		ledLightStatusSystem_ = ledLightStatusSystem;
		wantedStretchPaneIndex_ = ledLightStatusSystem_->MapViewTextStatusbarPaneIndex();
	}
	totalNumberOfPanes_ = totalNumberOfPanes;
}

int CStatusBarResize::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if(CStatusBar::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO:  Add your specialized creation code here

	return 0;
}

void CStatusBarResize::OnSize(UINT nType, int cx, int cy)
{
	CStatusBar::OnSize(nType, cx, cy);

	std::vector<int> paneEndPositionsInPixels(totalNumberOfPanes_);
	auto& statusBarCtrl = GetStatusBarCtrl();
	statusBarCtrl.GetParts(totalNumberOfPanes_, paneEndPositionsInPixels.data());
	auto findIter = std::find(paneEndPositionsInPixels.begin(), paneEndPositionsInPixels.end(), -1);
	if(findIter != paneEndPositionsInPixels.end())
	{
		*findIter = 0;
		paneEndPositionsInPixels[wantedStretchPaneIndex_] = -1;
		statusBarCtrl.SetParts(totalNumberOfPanes_, paneEndPositionsInPixels.data());
	}
	else
	{
		// N‰m‰ laskut tehd‰‰n vain alustuksen j‰lkeen, jolloin eri pane:ille on jo laskettu paikat
		std::vector<int> paneWidthsInPixels(totalNumberOfPanes_, paneEndPositionsInPixels.front());
		for(size_t index = 1; index < paneEndPositionsInPixels.size(); index++)
		{
			paneWidthsInPixels[index] = paneEndPositionsInPixels[index] - paneEndPositionsInPixels[index - 1];
		}
		int fixedPanesWidthInPixels = 0;
		for(size_t index = 0; index < paneWidthsInPixels.size(); index++)
		{
			if(index != wantedStretchPaneIndex_)
			{
				fixedPanesWidthInPixels += paneWidthsInPixels[index];
			}
		}
		int strechedPaneWidth = cx - fixedPanesWidthInPixels - 20;
		const int minimumStrechedPaneWidthInPixels = 400;
		if(strechedPaneWidth < minimumStrechedPaneWidthInPixels)
			strechedPaneWidth = minimumStrechedPaneWidthInPixels; // Pakotetaan venytett‰v‰‰ pane:a aina hieman n‰kyviin
		std::vector<int> newPaneEndPositionsInPixels(totalNumberOfPanes_);
		int currentPaneEndPoint = 0;
		for(size_t index = 0; index < newPaneEndPositionsInPixels.size(); index++)
		{
			if(index != wantedStretchPaneIndex_)
			{
				newPaneEndPositionsInPixels[index] = currentPaneEndPoint + paneWidthsInPixels[index];
			}
			else
			{
				newPaneEndPositionsInPixels[index] = currentPaneEndPoint + strechedPaneWidth;
			}
			currentPaneEndPoint = newPaneEndPositionsInPixels[index];
		}
		statusBarCtrl.SetParts(totalNumberOfPanes_, newPaneEndPositionsInPixels.data());
	}
}

BOOL CStatusBarResize::InitTooltipControl()
{
	if(ledLightStatusSystem_)
	{
		CFmiWin32Helpers::InitializeCPPTooltip(this, itsTooltipCtrl, STATUSBAR_TOOLTIP_ID, 950);
	}
	return TRUE;
}

void CStatusBarResize::NotifyDisplayTooltip(NMHDR* pNMHDR, LRESULT* result)
{
	if(ledLightStatusSystem_)
	{
		*result = 0;
		NM_PPTOOLTIP_DISPLAY* pNotify = (NM_PPTOOLTIP_DISPLAY*)pNMHDR;

		if(pNotify->ti->nIDTool == STATUSBAR_TOOLTIP_ID)
		{
			CPoint pt = *pNotify->pt;
			ScreenToClient(&pt);
			auto statusString = GetTooltipText(pt);
			if(!statusString.empty())
			{
				CString strU_ = CA2T(statusString.c_str());
				pNotify->ti->sTooltip = strU_;
			}

		} //if
	}
} //End NotifyDisplayTooltip

BOOL CStatusBarResize::PreTranslateMessage(MSG* pMsg)
{
	if(ledLightStatusSystem_)
	{
		itsTooltipCtrl.RelayEvent(pMsg);
	}
	return CStatusBar::PreTranslateMessage(pMsg);
}

std::string CStatusBarResize::GetTooltipText(const CPoint& cursorInClientSpace)
{
	// Tutkitaan osuiko hiirenkursori mink‰‰n ledi paneelin p‰‰lle ja vain silloin tehd‰‰n status sanomia
	if(ledLightStatusSystem_ && ledLightStatusSystem_->LedPaneCount() > 0)
	{
		auto& statusBarCtrl = GetStatusBarCtrl();
		for(int index = 0; index < (int)ledLightStatusSystem_->LedPaneCount(); index++)
		{
			int paneIndex = index + ledLightStatusSystem_->StartingLedPaneIndex();
			CRect paneRect;
			statusBarCtrl.GetRect(paneIndex, paneRect);
			if(paneRect.PtInRect(cursorInClientSpace))
			{
				return ledLightStatusSystem_->GetTooltipStatusMessage(index);
			}
		}
	}
	return "";
}


// CMainFrame
const NFmiViewPosRegistryInfo CMainFrame::s_ViewPosRegistryInfo(CRect(10, 10, 600, 800), "\\MainMapView");

#define WM_TRAYNOTIFY WM_USER + 147
CTrayNotifyIcon g_TrayIcon;

IMPLEMENT_DYNCREATE(CMainFrame, CFmiUsedFrameWndParent)

const int  iMaxUserToolbars = 10;
const UINT uiFirstUserToolBarId = AFX_IDW_CONTROLBAR_FIRST + 40;
const UINT uiLastUserToolBarId = uiFirstUserToolBarId + iMaxUserToolbars - 1;

BEGIN_MESSAGE_MAP(CMainFrame, CFmiUsedFrameWndParent)
	ON_WM_CREATE()

#ifndef FMI_DISABLE_MFC_FEATURE_PACK
	// Global help commands
	ON_COMMAND(ID_HELP_FINDER, &CFmiUsedFrameWndParent::OnHelpFinder)
	ON_COMMAND(ID_HELP, &CFmiUsedFrameWndParent::OnHelp)
	ON_COMMAND(ID_CONTEXT_HELP, &CFmiUsedFrameWndParent::OnContextHelp)
	ON_COMMAND(ID_DEFAULT_HELP, &CFmiUsedFrameWndParent::OnHelpFinder)
	ON_COMMAND(ID_VIEW_CUSTOMIZE, &CMainFrame::OnViewCustomize)
	ON_REGISTERED_MESSAGE(AFX_WM_CREATETOOLBAR, &CMainFrame::OnToolbarCreateNew)
	ON_COMMAND_RANGE(ID_VIEW_APPLOOK_WIN_2000, ID_VIEW_APPLOOK_OFF_2007_AQUA, &CMainFrame::OnApplicationLook)
	ON_UPDATE_COMMAND_UI_RANGE(ID_VIEW_APPLOOK_WIN_2000, ID_VIEW_APPLOOK_OFF_2007_AQUA, &CMainFrame::OnUpdateApplicationLook)
	ON_COMMAND(ID_VIEW_CAPTION_BAR, &CMainFrame::OnViewCaptionBar)
	ON_UPDATE_COMMAND_UI(ID_VIEW_CAPTION_BAR, &CMainFrame::OnUpdateViewCaptionBar)
	ON_COMMAND(ID_VIEW_DEMODOCKVIEWS, &CMainFrame::OnViewDemodockviews)
#endif // FMI_DISABLE_MFC_FEATURE_PACK

	ON_WM_CLOSE()
	ON_WM_DESTROY()
	ON_WM_PALETTECHANGED()
	ON_WM_QUERYNEWPALETTE()
	ON_WM_TIMER()
	ON_COMMAND(ID_VIEW_SET_MAIN_FRAME_PLACE_TO_DEFAULT, OnViewSetMainFramePlaceToDefault)
	ON_NOTIFY_EX_RANGE(TTN_NEEDTEXTW, 0, 0xFFFF, OnToolTipText)
	ON_NOTIFY_EX_RANGE(TTN_NEEDTEXTA, 0, 0xFFFF, OnToolTipText)
	ON_MESSAGE(WM_TRAYNOTIFY, OnTrayNotification)
	ON_COMMAND(ID_ENABLENOTIFICATIONS, &CMainFrame::OnEnablenotifications)
	ON_UPDATE_COMMAND_UI(ID_ENABLENOTIFICATIONS, &CMainFrame::OnUpdateEnablenotifications)
	ON_COMMAND(ID_DISABLENOTIFICATIONS, &CMainFrame::OnDisablenotifications)
	ON_UPDATE_COMMAND_UI(ID_DISABLENOTIFICATIONS, &CMainFrame::OnUpdateDisablenotifications)
	ON_COMMAND(ID_NOTIFICATIONSOUNDSON, &CMainFrame::OnNotificationsoundson)
	ON_UPDATE_COMMAND_UI(ID_NOTIFICATIONSOUNDSON, &CMainFrame::OnUpdateNotificationsoundson)
	ON_COMMAND(ID_NOTIFICATIONSOUNDSOFF, &CMainFrame::OnNotificationsoundsoff)
	ON_UPDATE_COMMAND_UI(ID_NOTIFICATIONSOUNDSOFF, &CMainFrame::OnUpdateNotificationsoundsoff)
	ON_COMMAND(ID_SHOW_SYSTRAY_SYMBOL, &CMainFrame::OnShowSystraySymbol)
	ON_UPDATE_COMMAND_UI(ID_SHOW_SYSTRAY_SYMBOL, &CMainFrame::OnUpdateShowSystraySymbol)
	ON_COMMAND(ID_HIDE_SYSTRAY_SYMBOL, &CMainFrame::OnHideSystraySymbol)
	ON_UPDATE_COMMAND_UI(ID_HIDE_SYSTRAY_SYMBOL, &CMainFrame::OnUpdateHideSystraySymbol)
    ON_WM_GETMINMAXINFO()
END_MESSAGE_MAP()

const int gDisableMacroParamThread = 1;
const int gDisableDataCache1Thread = 2;
const int gDisableDataCache2Thread = 4;
const int gDisableDataCache3Thread = 8;
const int gDisableDataCacheHistoryThread = 16;
const int gDisableCombineDataThread = 32;
const int gDisableSoundingIndexDataThread = 64;
const int gDisableWarningMessageThread = 128;
const int gDisableDataLoaderThread = 256;

static int GetDisabledThreads()
{
	int disableThreads = 0;
// Ota n‰it‰ riveja pois kommentista, jos haluat disabloida tiettyj‰ threadeja testi mieless‰
//	disableThreads |= gDisableMacroParamThread;
//	disableThreads |= gDisableDataCache1Thread;
//	disableThreads |= gDisableDataCache2Thread;
//	disableThreads |= gDisableDataCache3Thread;
//	disableThreads |= gDisableDataCacheHistoryThread;
//	disableThreads |= gDisableCombineDataThread;
//	disableThreads |= gDisableSoundingIndexDataThread;
//	disableThreads |= gDisableWarningMessageThread;
//	disableThreads |= gDisableDataLoaderThread;

	return disableThreads;
}

// CMainFrame construction/destruction

CMainFrame::CMainFrame()
:itsDisableThreadsVariable(0)
{
#ifndef FMI_DISABLE_MFC_FEATURE_PACK
	EnableActiveAccessibility();
	// TODO: add member initialization code here
	theApp.m_nAppLook = theApp.GetInt(_T("ApplicationLook"), ID_VIEW_APPLOOK_VS_2005);
#endif // FMI_DISABLE_MFC_FEATURE_PACK

	itsDoc = 0;

	itsRedFlagBitmap = new CBitmap;
	itsRedFlagBitmap->LoadBitmap(IDB_BITMAP_RED_FLAG);
	itsOrangeFlagBitmap = new CBitmap;
	itsOrangeFlagBitmap->LoadBitmap(IDB_BITMAP_ORANGE_FLAG);
	itsFlagButtonIndex = -1;
	itsDisableThreadsVariable = ::GetDisabledThreads();
}

CMainFrame::~CMainFrame()
{
	CtrlView::DestroyBitmap(&itsRedFlagBitmap);
    CtrlView::DestroyBitmap(&itsOrangeFlagBitmap);
}

// FindMenuItem() will find a menu item string from the specified
// popup menu and returns its position (0-based) in the specified 
// popup menu. It returns -1 if no such menu item string is found.
static int FindMenuItem(CMenu* Menu, LPCTSTR MenuString)
{
   ASSERT(Menu);
   ASSERT(::IsMenu(Menu->GetSafeHmenu()));

   int count = Menu->GetMenuItemCount();
   for (int i = 0; i < count; i++)
   {
       CString strU_;
       if(Menu->GetMenuString(i, strU_, MF_BYPOSITION) &&
           strU_.Compare(MenuString) == 0)
         return i;
   }

   return -1;
}

static void RemoveMenuItem(CMenu* Menu, LPCTSTR subMenuString, LPCTSTR menuItemString)
{
	if(Menu)
	{
		int pos = ::FindMenuItem(Menu, subMenuString); // etsit‰‰n haluttu sub-menu
		if (pos != -1)
		{
			CMenu* submenu = Menu->GetSubMenu(pos);
			pos = ::FindMenuItem(submenu, menuItemString); // etsit‰‰n haluttu menuitem lˆydetyst‰ sub-menusta
			if (pos > -1)
				submenu->RemoveMenu(pos, MF_BYPOSITION);
		}
	}
}

// k‰y l‰pi menun ja etsii haluttua komentoa ja poistaa sen
static void RemoveMenuItem(CMenu* Menu, UINT theCommand)
{
	if(Menu)
	{
		for(UINT i = 0; i < static_cast<UINT>(Menu->GetMenuItemCount()); i++)
		{
			CMenu* subMenu = Menu->GetSubMenu(i);
			for(UINT j = 0; j < static_cast<UINT>(subMenu->GetMenuItemCount()); j++)
			{
				if(subMenu->GetMenuItemID(j) == theCommand)
					subMenu->RemoveMenu(j, MF_BYPOSITION);
			}
		}
	}
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFmiUsedFrameWndParent::OnCreate(lpCreateStruct) == -1)
		return -1;

	CSmartMetApp* app = (CSmartMetApp*)AfxGetApp();
	if(app)
	{
		itsDoc = app->GeneralDocData();
	}

#ifdef FMI_DISABLE_MFC_FEATURE_PACK
	if (!m_wndToolBar.Create(this) || !m_wndToolBar.LoadToolBar(IDR_MAINFRAME))
#else
	BOOL bNameValid;
	// set the visual manager and style based on persisted value
	OnApplicationLook(theApp.m_nAppLook);

	if (!m_wndMenuBar.Create(this))
	{
		TRACE0("Failed to create menubar\n");
		return -1;      // fail to create
	}

	m_wndMenuBar.SetPaneStyle(m_wndMenuBar.GetPaneStyle() | CBRS_SIZE_DYNAMIC | CBRS_TOOLTIPS | CBRS_FLYBY);

	// prevent the menu bar from taking the focus on activation
	CMFCPopupMenu::SetForceMenuFocus(FALSE);

	if (!m_wndToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP | CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
		!m_wndToolBar.LoadToolBar(theApp.m_bHiColorIcons ? IDR_MAINFRAME_256 : IDR_MAINFRAME))
#endif // FMI_DISABLE_MFC_FEATURE_PACK
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}

#ifdef FMI_DISABLE_MFC_FEATURE_PACK
	// 256 v‰risen ja vaihtuva kokoisen tolbarin teko vaatii n‰iden k‰yttˆˆn oton
	itsToolBarBitmapHandle = (HBITMAP) ::LoadImage(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDR_MAINFRAME_256), 
							IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION | LR_LOADMAP3DCOLORS);
	itsToolBarBitmap.Attach(itsToolBarBitmapHandle);
	itsToolBarImagelist.Create(16, 16, ILC_COLOR24, 4, 4);
	itsToolBarImagelist.Add(&itsToolBarBitmap, (CBitmap*) NULL);
	m_wndToolBar.GetToolBarCtrl().SetImageList(&itsToolBarImagelist);

#else
    CString strToolBarNameU_;
	bNameValid = strToolBarName.LoadString(IDS_TOOLBAR_STANDARD);
	ASSERT(bNameValid);
	m_wndToolBar.SetWindowText(strToolBarName);

    CString strCustomizeU_;
	bNameValid = strCustomize.LoadString(IDS_TOOLBAR_CUSTOMIZE);
	ASSERT(bNameValid);
	m_wndToolBar.EnableCustomizeButton(TRUE, ID_VIEW_CUSTOMIZE, strCustomize);

	// Allow user-defined toolbars operations:
	InitUserToolbars(NULL, uiFirstUserToolBarId, uiLastUserToolBarId);
#endif // FMI_DISABLE_MFC_FEATURE_PACK

	// Apply user settings to optionally hide some of the toolbar controls
	ConfigureToolbarControls();

	if(CreateStatusBar() != 0)
		return -1;

#ifdef FMI_DISABLE_MFC_FEATURE_PACK
	// TODO: Remove this if you don't want tool tips or a resizeable toolbar
	m_wndToolBar.SetBarStyle(m_wndToolBar.GetBarStyle() | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC);

	// TODO: Delete these three lines if you don't want the toolbar to be dockable
	//m_wndToolBar.EnableDocking(CBRS_ALIGN_ANY);
	//EnableDocking(CBRS_ALIGN_ANY);
	//DockControlBar(&m_wndToolBar);
#else
	// TODO: Delete these five lines if you don't want the toolbar and menubar to be dockable
	m_wndMenuBar.EnableDocking(CBRS_ALIGN_ANY);
	m_wndToolBar.EnableDocking(CBRS_ALIGN_ANY);
	EnableDocking(CBRS_ALIGN_ANY);
	DockPane(&m_wndMenuBar);
	DockPane(&m_wndToolBar);


	// enable Visual Studio 2005 style docking window behavior
	CDockingManager::SetDockingMode(DT_SMART);
	// enable Visual Studio 2005 style docking window auto-hide behavior
	EnableAutoHidePanes(CBRS_ALIGN_ANY);

	// Create a caption bar:
	if (!CreateCaptionBar())
	{
		TRACE0("Failed to create caption bar\n");
		return -1;      // fail to create
	}

	// Load menu item image (not placed on any standard toolbars):
	CMFCToolBar::AddToolBarForImageCollection(IDR_MENU_IMAGES, theApp.m_bHiColorIcons ? IDB_MENU_IMAGES_24 : 0);

	// HUOM! Jos dokkautuvat pane-ikkunat laitetaan t‰‰ll‰ jo p‰‰lle, sen voisi tehd‰ t‰ss‰ kohtaa.
	//OnViewDemodockviews();

	// Enable toolbar and docking window menu replacement
	EnablePaneMenu(TRUE, ID_VIEW_CUSTOMIZE, strCustomize, ID_VIEW_TOOLBAR);

	// enable quick (Alt+drag) toolbar customization
	CMFCToolBar::EnableQuickCustomization();

	if (CMFCToolBar::GetUserImages() == NULL)
	{
		// load user-defined toolbar images
		if (m_UserImages.Load(_T(".\\UserImages.bmp")))
		{
			m_UserImages.SetImageSize(CSize(16, 16), FALSE);
			CMFCToolBar::SetUserImages(&m_UserImages);
		}
	}
#endif // FMI_DISABLE_MFC_FEATURE_PACK

	std::string errorBaseStr("Error in CMainFrame::OnInitDialog while reading dialog size and position values");
//	CFmiWin32TemplateHelpers::DoWindowSizeSettings(this, false, errorBaseStr, itsDoc->Logger());
    CFmiWin32TemplateHelpers::DoWindowSizeSettingsFromWinRegistry(itsDoc->ApplicationWinRegistry(), this, false, errorBaseStr, 0);

    StartSmartMetTimers();

#ifdef FMI_DISABLE_MFC_FEATURE_PACK
	// Otetaan MFC-feature-pack liit‰nn‰iset osat pois menusta
	CMenu* pMenu = GetMenu();
	if(pMenu)
	{
		::RemoveMenuItem(pMenu, _T("&View"), _T("&Toolbars and Docking Windows")); 
		::RemoveMenuItem(pMenu, _T("&View"), _T("&Caption Bar")); 
		::RemoveMenuItem(pMenu, _T("&View"), _T("Demo dock views")); 
		::RemoveMenuItem(pMenu, _T("&View"), _T("&Application Look")); 
		::RemoveMenuItem(pMenu, _T("&Help"), _T("&Help Topics")); 
	}

    LocalizeMenuStrings();

#endif // FMI_DISABLE_MFC_FEATURE_PACK

	std::string usedTooltipStr = itsDoc->DataNotificationSettings().ToolTipUseTitle() ? itsDoc->GetApplicationTitle() : itsDoc->DataNotificationSettings().ToolTip();
    if(!g_TrayIcon.Create(this, IDR_MENU_SYS_TRAY_POPUP, CA2T(usedTooltipStr.c_str()), LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDR_MAINFRAME)), WM_TRAYNOTIFY, 0, itsDoc->DataNotificationSettings().ShowIcon()))
	{
		AfxMessageBox(_T("Failed to create SmartMet tray icon"), MB_OK | MB_ICONSTOP);
		return -1;
	}

	return 0;
}

int CMainFrame::CreateStatusBar()
{
	MakeStatusBarIndicators();
	m_wndStatusBar.Setup(&itsDoc->LedLightStatusSystem(), (int)indicatorVector.size());
	DWORD statusbarStyle = WS_CHILD | WS_VISIBLE | CBRS_BOTTOM;
	if(!m_wndStatusBar.Create(this, statusbarStyle) || !m_wndStatusBar.SetIndicators(indicatorVector.data(), (int)indicatorVector.size()))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}
	SetupLedIndicatorsInStatusbar();
	return 0;
}

void CMainFrame::MakeStatusBarIndicators()
{
	auto& ledLightStatusSystem = itsDoc->LedLightStatusSystem();
	if(!ledLightStatusSystem.IsInitilized())
	{
		// Jos ledLightStatusSystem ei ole k‰ytˆss‰, laitetaan vain perus jutut indikaattoreiksi (else haaran j‰lkeen loput)
		indicatorVector.push_back(ID_SEPARATOR);
	}
	else
	{
		// Venyv‰ teksti osio (ID_SEPARATOR) on joko alussa tai heti ledien j‰lkeen
		if(ledLightStatusSystem.MapViewTextStatusbarPaneIndex() == 0)
			indicatorVector.push_back(ID_SEPARATOR);
		if(ledLightStatusSystem.LedPaneCount() >= 1)
			indicatorVector.push_back(ID_INDICATOR_LED_CONTROL_1);
		if(ledLightStatusSystem.LedPaneCount() >= 2)
			indicatorVector.push_back(ID_INDICATOR_LED_CONTROL_2);
		if(ledLightStatusSystem.LedPaneCount() >= 3)
			indicatorVector.push_back(ID_INDICATOR_LED_CONTROL_3);
		if(ledLightStatusSystem.LedPaneCount() >= 4)
			indicatorVector.push_back(ID_INDICATOR_LED_CONTROL_4);
		if(ledLightStatusSystem.LedPaneCount() >= 5)
			indicatorVector.push_back(ID_INDICATOR_LED_CONTROL_5);
		if(ledLightStatusSystem.LedPaneCount() >= 6)
			indicatorVector.push_back(ID_INDICATOR_LED_CONTROL_6);
		if(ledLightStatusSystem.MapViewTextStatusbarPaneIndex() != 0)
			indicatorVector.push_back(ID_SEPARATOR); // TAI heti ledien j‰lkeen
	}

	// Loppuun normi CAPS/NUMLOCK/SCROL-LOCK indikaattorit
	indicatorVector.push_back(ID_INDICATOR_CAPS);
	indicatorVector.push_back(ID_INDICATOR_NUM);
	indicatorVector.push_back(ID_INDICATOR_SCRL);
}

static void SetPaneWidth(CStatusBar& statusBar, int paneId, int wantedWidth)
{
	UINT nId = 0;
	UINT nStyle = 0;
	int cxWidth = 0;
	statusBar.GetPaneInfo(paneId, nId, nStyle, cxWidth);
	cxWidth = wantedWidth;
	statusBar.SetPaneInfo(paneId, nId, nStyle, cxWidth);
}

void CMainFrame::SetupLedIndicatorsInStatusbar()
{
	auto& ledLightStatusSystem = itsDoc->LedLightStatusSystem();
	if(ledLightStatusSystem.IsInitilized())
	{
		int maxPaneIndex = (int)ledLightStatusSystem.LedPaneCount() + ledLightStatusSystem.StartingLedPaneIndex();
		for(int paneId = ledLightStatusSystem.StartingLedPaneIndex(); paneId < maxPaneIndex; paneId++)
		{
			::SetPaneWidth(m_wndStatusBar, paneId, 16);
		}

		DoLedLightsActionUpdates();
		m_wndStatusBar.InitTooltipControl();
	}
}

void CMainFrame::DoLedLightsActionUpdates()
{
	if(itsDoc->LedLightStatusSystem().IsInitilized())
	{
		for(auto& ledLightStatus : itsDoc->LedLightStatusSystem().LedLightStatusVector())
		{
			auto colorChangedStatus = ledLightStatus.hasLedColorChanged();
			if(colorChangedStatus.first)
			{
				UpdateStatusBarIcons(ledLightStatus.statusBarPaneIndex(), colorChangedStatus.second);
			}
		}
	}
}

void CMainFrame::UpdateStatusBarIcons(int paneId, NFmiLedColor ledColor)
{
	UINT iconId = IDI_ICON_LED_GRAY;

	switch(ledColor)
	{
	case NFmiLedColor::Blue:
		iconId = IDI_ICON_LED_BLUE;
		break;
	case NFmiLedColor::Gray:
		iconId = IDI_ICON_LED_GRAY;
		break;
	case NFmiLedColor::Green:
		iconId = IDI_ICON_LED_GREEN;
		break;
	case NFmiLedColor::LightBlue:
		iconId = IDI_ICON_LED_LIGHT_BLUE;
		break;
	case NFmiLedColor::LightGreen:
		iconId = IDI_ICON_LED_LIGHT_GREEN;
		break;
	case NFmiLedColor::Orange:
		iconId = IDI_ICON_LED_ORANGE;
		break;
	case NFmiLedColor::Purple:
		iconId = IDI_ICON_LED_PURPLE;
		break;
	case NFmiLedColor::Red:
		iconId = IDI_ICON_LED_RED;
		break;
	case NFmiLedColor::Yellow:
		iconId = IDI_ICON_LED_YELLOW;
		break;
	default:
		break;
	}

	if(iconId)
	{
		HICON hIcon = (HICON)::LoadImage(AfxGetInstanceHandle(), MAKEINTRESOURCE(iconId), IMAGE_ICON, 16, 16, LR_SHARED);
		if(hIcon)
		{
			m_wndStatusBar.GetStatusBarCtrl().SetIcon(paneId, hIcon);
		}
	}
	else
	{
		m_wndStatusBar.GetStatusBarCtrl().SetIcon(paneId, NULL);
	}

}

void CMainFrame::StartSmartMetTimers()
{
    itsCleanDataTimer = static_cast<UINT>(SetTimer(kFmiCleanDataTimer, 1 * 5 * 60 * 1000, NULL)); // siivotaan datahakemistot kerran 5 minuutin kuluttua, siell‰ timer asetetaan sitten miten pit‰‰
    itsDebugDataSizeLoggerTimer = static_cast<UINT>(SetTimer(kFmiDebugDataSizeLoggerTimer, 1 * 1 * 30 * 1000, NULL)); // aloitetaan raportointi 30 sekunnin kuluttua, muutetaan sitten harvemmaksi
    itsCheckAnimationLockedModeTimeBagsTimer = static_cast<UINT>(SetTimer(kFmiCheckAnimationLockedModeTimeBagsTimer, 1 * 1 * 63 * 1000, NULL)); // laitan animaatio havainto lukitus timerin 63 sekuntiin, ett‰ se ei mene aina samoihin aikoihin kuin muutkin kerran minuutissa pyˆr‰ht‰v‰t timerit
    itsCheckForNewSatelDataTimer = static_cast<UINT>(SetTimer(kFmiCheckForNewSatelDataTimer, 1 * 57 * 1000, NULL)); // laitan satel/k‰siteanalyysi datan tarkistuksen timerin 57 sekuntiin, ett‰ se ei mene aina samoihin aikoihin kuin muutkin kerran minuutissa pyˆr‰ht‰v‰t timerit
    itsAutoSaveTimer = static_cast<UINT>(SetTimer(kFmiAutoSaveTimer, 1 * 60 * 1000, NULL));
    double updateInterValInHours = itsDoc->ApplicationDataBase().UpdateIntervalInHours();
    if(updateInterValInHours > 0)
        itsDataToDBUpdateTimer = static_cast<UINT>(SetTimer(kFmiDataToDBUpdateTimer, static_cast<UINT>(updateInterValInHours * 60 * 60 * 1000), NULL)); // kerran vuorokaudessa l‰hetys
    itsCleanOldDataFromMemoryTimer = static_cast<UINT>(SetTimer(kFmiCleanOldDataFromMemoryTimer, 5 * 60 * 1000, NULL)); // siivotaan queryDatoja muistissa aina 5 minuutin v‰lein
    itsStoreViewPosToWinRegistryTimer = static_cast<UINT>(SetTimer(kFmiStoreViewPosToWinRegistryTimer, 117 * 1000, NULL)); // tehd‰‰n ikkunoiden koko+sijainti talletuksia rekisteriin n. parin minuutin v‰lein (117 sekuntia)
    itsStoreCrashBackupViewMacroTimer = static_cast<UINT>(SetTimer(kFmiStoreCrashBackupViewMacroTimer, 87 * 1000, NULL)); // tehd‰‰n crash backup viewmacro talletuksia n. 1.5 minuutin v‰lein (87 sekuntia)
    itsGenerateBetaProductsTimer = static_cast<UINT>(SetTimer(kFmiGenerateBetaProductsTimer, 60 * 1000, NULL)); // Tarkastellaan minuutin v‰lein, ett‰ pit‰‰kˆ beta-producteja tehd‰ 
    itsLoggingSystemManagementTimer = static_cast<UINT>(SetTimer(kFmiLoggingSystemManagementTimer, 12 * 60 * 1000, NULL)); // Lokitus systeemi‰ pit‰‰ hallinnoida aika ajoin, viestien trimmau muistista ja crash-reporteriin mahdollisesti p‰ivitetty lokitiedoston nimi
	if(itsDoc->LedLightStatusSystem().IsInitilized())
	{
		itsLedLightsActionTimer = static_cast<UINT>(SetTimer(kFmiLedLightsActionTimer, 200, NULL)); // ledej‰ s‰‰det‰‰n 1/5 sekunnin v‰lein
	}
	if(itsDoc->GetCombinedMapHandler()->wmsSupportAvailable())
	{
		auto renewWmsSystemIntervalInHours = itsDoc->GetCombinedMapHandler()->getWmsSupport()->getSetup()->renewWmsSystemIntervalInHours;
		if(renewWmsSystemIntervalInHours > 0)
		{
			const UINT hourInMilliseconds = 60 * 60 * 1000;
			// WmsSupport regeneroinnin tarkoitus on uusia WmsSupport otus tietyn v‰liajoin, koska on huomattu ainakin
			// Beta-tuotanto koneissa ett‰ wms datan saanti loppuu usein 1-3 vuorokauden kuluttua.
			// Sellainen 6 tunnin v‰lein Wms-systeemin vaihto voisi olla kokeilemisen arvoista.
			itsWmsSupportRenewerTimer = static_cast<UINT>(SetTimer(kFmiWmsSupportRenewerTimer, static_cast<UINT>(renewWmsSystemIntervalInHours * hourInMilliseconds), NULL));
		}
	}
}

static void LocalizeMenuStrings(CMenu *pMenu)
{
	if(pMenu)
	{
        for(int i = 0; i < pMenu->GetMenuItemCount(); i++)
        {
            CMenu *pSubMenu = pMenu->GetSubMenu(i);
            LocalizeMenuStrings(pSubMenu);
            CString menuItemStrU_;
            pMenu->GetMenuString(i, menuItemStrU_, MF_BYPOSITION);
            if(menuItemStrU_.GetLength())
            {
                UINT itemId = pMenu->GetMenuItemID(i);
                auto newMenuItemStrU_ = CtrlView::StringToWString(::GetDictionaryString(CT2A(menuItemStrU_)));
                menuItemStrU_ = newMenuItemStrU_.c_str();
                pMenu->ModifyMenu(i, MF_BYPOSITION | MF_STRING, itemId, menuItemStrU_);
            }
        }
    }
}

void CMainFrame::LocalizeMenuStrings()
{
	CMenu *pMenu = GetMenu();
    ::LocalizeMenuStrings(pMenu);
}

// T‰ss‰ metodissa on tarkoitus k‰ynnist‰‰ ne working-threadit, jotka k‰ynnistet‰‰n aina.
// StartDataLoadingWorkingThread -metodissa k‰ynnistet‰‰n ne working-threadit, jotka
// k‰ynnistet‰‰n vain ns. operatiivisessa tilassa.
void CMainFrame::StartAdditionalWorkerThreads()
{
	DoAppDataBaseCollection(NFmiApplicationDataBase::kStart);

	// k‰ynnistet‰‰n macroParam update threadi kerran, ja se pit‰‰ ensin initialisoida. T‰m‰ threadi k‰ynnistet‰‰n aina.
	CFmiMacroParamUpdateThread::InitMacroParamSystem(itsDoc->MacroParamSystem());
	int macroParamThreadDelayInMS = 6 * 1000;
	if(itsDoc->MachineThreadCount() >= 6)
		macroParamThreadDelayInMS /= 3; // jos konessa on paljon coreja, ei tarvitse viivytt‰‰ alkua niin paljoa
	CFmiMacroParamUpdateThread::SetFirstTimeDelay(macroParamThreadDelayInMS);
	if((itsDisableThreadsVariable & gDisableMacroParamThread) == 0)
		CWinThread *macroParamUpdateThread = AfxBeginThread(CFmiMacroParamUpdateThread::DoThread, nullptr, THREAD_PRIORITY_BELOW_NORMAL);

	StartQDataCacheThreads();
}

// T‰lle piti tehd‰ oma metodi, koska SmartMetin asetuksia muuttamalla t‰m‰ pit‰‰ k‰ynnist‰‰ joskus erikseen tai uudelleen.
void CMainFrame::StartQDataCacheThreads()
{
	if(itsDoc->HelpDataInfoSystem()->UseQueryDataCache() == false)
		return ; // ei k‰ynnistet‰ cache-worker-threadeja, koska asetukset sanovat ett‰ niit‰ ei k‰ytet‰

    std::string smartMetBinariesDirectory = itsDoc->ApplicationDataBase().GetDecodedApplicationDirectory(); 
	double maximumFileSizeMB = itsDoc->HelpDataInfoSystem()->CacheMaximumFileSizeMB();
	// k‰ynnistet‰‰n qdata cache-loader threadi kerran, ja se pit‰‰ ensin initialisoida. T‰m‰ threadi k‰ynnistet‰‰n aina.
	auto loadDataAtStartUp = itsDoc->ApplicationWinRegistry().ConfigurationRelatedWinRegistry().LoadDataAtStartUp();
	auto autoLoadNewCacheData = itsDoc->ApplicationWinRegistry().ConfigurationRelatedWinRegistry().AutoLoadNewCacheData();
	QueryDataToLocalCacheLoaderThread::InitHelpDataInfo(*itsDoc->HelpDataInfoSystem(), smartMetBinariesDirectory, itsDoc->FileCleanerSystem().CleaningTimeStepInHours(), loadDataAtStartUp, autoLoadNewCacheData, itsDoc->WorkingDirectory(), maximumFileSizeMB);

	std::thread t(QueryDataToLocalCacheLoaderThread::DoThread);
	t.detach(); // Detach the thread
}

void CMainFrame::StartHistoryDataCacheThread()
{
    double maximumFileSizeMB = itsDoc->HelpDataInfoSystem()->CacheMaximumFileSizeMB();
    // T‰m‰ yritt‰‰ k‰ynnist‰‰ uuden worker threadin. Jos se on jo k‰ynniss‰, uusi threadi lopettaa saman 2 sekunnin odottelun j‰lkeen.
//    ::MakeDataLoaderThread(4, 0, maximumFileSizeMB, THREAD_PRIORITY_NORMAL, itsQDataCacheLoaderDatas, 0, true); // laitetaan tosi isot tiedostot tulemaan pinemm‰ll‰ prioriteetill‰
}

void CMainFrame::StopQDataCacheThreads()
{
	QueryDataToLocalCacheLoaderThread::CloseNow();
}

typedef std::deque<std::pair<time_t, std::string> > MessageContainer;

static MessageContainer g_lastNotificationMessages; // t‰ss‰ on viimeiset viestit

// TheMessages-containerissa on vanhat viestit aina per‰ll‰ ja uudemmat edess‰!
static void ClearOldMessages(MessageContainer &theMessages, time_t currentTime, int theExpirationTimeInSeconds)
{
	do
	{
		if(theMessages.size() > 0)
		{
			if(currentTime - theMessages[theMessages.size()-1].first > theExpirationTimeInSeconds)
				theMessages.pop_back();
			else
				break;
		}
		else
			break;
	} while(true);
}

// t‰m‰ kokoaa uusimmista notifikaatioista yhdisrtelm‰, kuitenkin niin ett‰ stringin pituus on maksimissa 255 merkki‰, mik‰ on notifikaatio rajoitus
static std::string MakeTotalNotification(MessageContainer &theMessages)
{
	std::string totalStr;
	for(size_t i=0; i<theMessages.size(); i++)
	{
		if(i > 0)
			totalStr += "\n\n"; // laitetaan eri viestien erottimeksi rivinvaihdot, ett‰ tulee yksi tyhj‰ rivi v‰liin

		totalStr += theMessages[i].second;
	}
	totalStr.resize(255);
	return totalStr;
}

// theStyle is 0=Warning,1=Error,2=Info,3=None,4=User. This sets the used icon in balloon, must add user icon to paramList if that is wanted to use.
void CMainFrame::SetNotificationMessage(const std::string &theNotificationMsgStr, const std::string &theNotificationTitle, int theStyle, int theTimeout, bool fNoSound)
{
	// Pidet‰‰n listaa, miss‰ on viimeisimm‰t viestit ja tulostetaan niit‰ viimeisen 30 sekunnin ajalta
	// Kuitenkin niin ett‰ lopulliseen Notifikaatio tekstiin mahtuu maksimissaan 255 merkki‰, mik‰ on k‰ytetyn data struktuurin raja

	time_t t;
	time(&t);
	g_lastNotificationMessages.push_front(std::make_pair(t, theNotificationMsgStr));
	::ClearOldMessages(g_lastNotificationMessages, t, 60 * 3); // pidet‰‰n vaikka kolme viimeisinta minuuttia tallessa
	std::string finalNotifStr = ::MakeTotalNotification(g_lastNotificationMessages);
    g_TrayIcon.SetBalloonDetails(CA2T(finalNotifStr.c_str()), CA2T(theNotificationTitle.c_str()), static_cast<CTrayNotifyIcon::BalloonStyle>(theStyle), theTimeout, 0, fNoSound);
}

LRESULT CMainFrame::OnTrayNotification(WPARAM wParam, LPARAM lParam)
{
  //Delegate all the work back to the default implementation in
  //CTrayNotifyIcon.
  g_TrayIcon.OnTrayNotification(wParam, lParam);
  return 0L;
}

void CMainFrame::ConfigureToolbarControls()  
{
	// Hide / Show various toolbar controls based on the configuration options
	// Configuration option -> COMMAND ID mapping
    SetControlHiding("MetEditor::Toolbar::SelectParamDlg", ID_BUTTON_VIEW_SELECT_PARAM_DIALOG);
    SetControlHiding("MetEditor::Toolbar::LoadData", ID_BUTTON_LOAD_DATA);
	SetControlHiding("MetEditor::Toolbar::StoreData", ID_BUTTON_STORE_DATA);
	SetControlHiding("MetEditor::Toolbar::DataToDatabase", ID_BUTTON_DATA_TO_DATABASE);
	SetControlHiding("MetEditor::Toolbar::HelpEditorMode", ID_BUTTON_HELP_EDITOR_MODE); //x = unable to find corresponding menu item
	SetControlHiding("MetEditor::Toolbar::DataQualityChecker", ID_BUTTON_DATA_QUALITY_CHECKER);
	SetControlHiding("MetEditor::Toolbar::Print", ID_FILE_PRINT);
	SetControlHiding("MetEditor::Toolbar::ZoomDialog", ID_BUTTON_ZOOM_DIALOG);
	SetControlHiding("MetEditor::Toolbar::DataArea", ID_BUTTON_DATA_AREA);
	SetControlHiding("MetEditor::Toolbar::SelectFinlandMap", ID_BUTTON_SELECT_FINLAND_MAP);
	SetControlHiding("MetEditor::Toolbar::SelectScandinavianMap", ID_BUTTON_SELECT_SCANDINAVIA_MAP);
	SetControlHiding("MetEditor::Toolbar::SelectEuropeMap", ID_BUTTON_SELECT_EUROPE_MAP);
	SetControlHiding("MetEditor::Toolbar::SelectGlobe", ID_BUTTON_GLOBE);
	SetControlHiding("MetEditor::Toolbar::ViewGridSelectionDlg", ID_MENUITEM_VIEW_GRID_SELECTION_DLG);
	SetControlHiding("MetEditor::Toolbar::ExtraMapViewDlg1", ID_EXTRA_MAP_VIEW_1);
	SetControlHiding("MetEditor::Toolbar::ExtraMapViewDlg2", ID_EXTRA_MAP_VIEW_2);
	SetControlHiding("MetEditor::Toolbar::TimeEditValuesDlg", ID_BUTTON_TIME_EDIT_VALUES_DLG);
	SetControlHiding("MetEditor::Toolbar::SelectionToolDlg", ID_BUTTON_SELECTION_TOOL_DLG);
	SetControlHiding("MetEditor::Toolbar::FilterDialog", ID_BUTTON_FILTER_DIALOG);
	SetControlHiding("MetEditor::Toolbar::BrushToolDlg", ID_BUTTON_BRUSH_TOOL_DLG); //x
	SetControlHiding("MetEditor::Toolbar::SmartToolDlg", ID_BUTTON_OPEN_SMART_TOOL_DLG);
	SetControlHiding("MetEditor::Toolbar::EditorControlPointMode", ID_BUTTON_EDITOR_CONTROL_POINT_MODE);//x
	SetControlHiding("MetEditor::Toolbar::ValidateData", ID_BUTTON_VALIDATE_DATA); //x
	SetControlHiding("MetEditor::Toolbar::ShowCrossSection", ID_BUTTON_TEMP_DLG);
	SetControlHiding("MetEditor::Toolbar::StoreViewMacro", ID_BUTTON_SHOW_CROSS_SECTION);
	SetControlHiding("MetEditor::Toolbar::ObservationComparisonMode", ID_BUTTON_OBSERVATION_COMPARISON_MODE);//x
	SetControlHiding("MetEditor::Toolbar::SynopDataGridView", ID_BUTTON_SYNOP_DATA_GRID_VIEW);
	SetControlHiding("MetEditor::Toolbar::Trajectory", ID_BUTTON_TRAJECTORY);
	SetControlHiding("MetEditor::Toolbar::Animation", ID_BUTTON_ANIMATION);
	SetControlHiding("MetEditor::Toolbar::Refresh", ID_BUTTON_REFRESH);
	SetControlHiding("MetEditor::Toolbar::MakeEditedDataCopy", ID_BUTTON_MAKE_EDITED_DATA_COPY);
	SetControlHiding("MetEditor::Toolbar::Undo", ID_EDIT_UNDO);
	SetControlHiding("MetEditor::Toolbar::Redo", ID_EDIT_REDO);
}

#ifndef FMI_DISABLE_MFC_FEATURE_PACK
static void RemoveMenuItem(CMFCMenuBar &theMenuBar, int commandId)
{
	CMenu* pMenu = CMenu::FromHandle(theMenuBar.GetHMenu());
	if(pMenu)
	{
		for(int i = 0; i < static_cast<int>(pMenu->GetMenuItemCount()); i++)
		{
			CMenu* pSubMenu = pMenu->GetSubMenu(i);
			if(pSubMenu) 
				pSubMenu->RemoveMenu(commandId, MF_BYCOMMAND);
		}
	}
}
#endif // FMI_DISABLE_MFC_FEATURE_PACK

/**
* Load given configuration option and hide the corresponding toolbar control
* when the value of the option is "Hide" (case-sensitive at the moment).
*/
void CMainFrame::SetControlHiding(const std::string &confOption, int commandId)
{
	static int currentIndex = -1; // HUOM! t‰t‰ funktiota voidaan kutsua vain kerran!!!! T‰h‰n lasketaan qualityDataChecker-anapin indeksi

	// Default behaviour is to show the control when the corresponding configuration option is missing.
	std::string value = NFmiSettingsImpl::Instance().Value(confOption, "Show");
	bool hide = value.compare("Hide") == 0;
	if(hide)
#ifdef FMI_DISABLE_MFC_FEATURE_PACK
	{
		m_wndToolBar.GetToolBarCtrl().HideButton(commandId, true);
		::RemoveMenuItem(GetMenu(), commandId); // poistetaan hide-tapauksessa myˆs mahdollinen vastaava menu-kohta
	}
#else
	{
		// piilotetaan nappula toolbarista
		int index = m_wndToolBar.CommandToIndex(commandId);
		if(index >= 0)
		{
			CMFCToolBarButton *button = m_wndToolBar.GetButton(index);
			if(button)
				button->SetVisible(FALSE);
		}
		// Sitten toiminto piilotetaan menu-valikosta
		::RemoveMenuItem(m_wndMenuBar, commandId);
	}
#endif // FMI_DISABLE_MFC_FEATURE_PACK
	else
	{
		currentIndex++;
		if(commandId == ID_BUTTON_DATA_QUALITY_CHECKER)
			itsFlagButtonIndex = currentIndex;
	}
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CFmiUsedFrameWndParent::PreCreateWindow(cs) )
		return FALSE;
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return TRUE;
}

#ifndef FMI_DISABLE_MFC_FEATURE_PACK
BOOL CMainFrame::CreateDockingWindows()
{
	BOOL bNameValid;

	// Create class view
    CString strClassViewU_;
	bNameValid = strClassView.LoadString(IDS_CLASS_VIEW);
	ASSERT(bNameValid);
	if (!m_wndClassView.Create(strClassView, this, CRect(0, 0, 200, 200), TRUE, ID_VIEW_CLASSVIEW, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | CBRS_LEFT | CBRS_FLOAT_MULTI))
	{
		TRACE0("Failed to create Class View window\n");
		return FALSE; // failed to create
	}

	// Create file view
    CString strFileViewU_;
	bNameValid = strFileView.LoadString(IDS_FILE_VIEW);
	ASSERT(bNameValid);
	if (!m_wndFileView.Create(strFileView, this, CRect(0, 0, 200, 200), TRUE, ID_VIEW_FILEVIEW, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | CBRS_LEFT| CBRS_FLOAT_MULTI))
	{
		TRACE0("Failed to create File View window\n");
		return FALSE; // failed to create
	}

	// Create output window
    CString strOutputWndU_;
	bNameValid = strOutputWnd.LoadString(IDS_OUTPUT_WND);
	ASSERT(bNameValid);
	if (!m_wndOutput.Create(strOutputWnd, this, CRect(0, 0, 100, 100), TRUE, ID_VIEW_OUTPUTWND, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | CBRS_BOTTOM | CBRS_FLOAT_MULTI))
	{
		TRACE0("Failed to create Output window\n");
		return FALSE; // failed to create
	}

	// Create properties window
    CString strPropertiesWndU_;
	bNameValid = strPropertiesWnd.LoadString(IDS_PROPERTIES_WND);
	ASSERT(bNameValid);
	if (!m_wndProperties.Create(strPropertiesWnd, this, CRect(0, 0, 200, 200), TRUE, ID_VIEW_PROPERTIESWND, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | CBRS_RIGHT | CBRS_FLOAT_MULTI))
	{
		TRACE0("Failed to create Properties window\n");
		return FALSE; // failed to create
	}

	SetDockingWindowIcons(theApp.m_bHiColorIcons);
	return TRUE;
}

void CMainFrame::SetDockingWindowIcons(BOOL bHiColorIcons)
{
	HICON hFileViewIcon = (HICON) ::LoadImage(::AfxGetResourceHandle(), MAKEINTRESOURCE(bHiColorIcons ? IDI_FILE_VIEW_HC : IDI_FILE_VIEW), IMAGE_ICON, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), 0);
	m_wndFileView.SetIcon(hFileViewIcon, FALSE);

	HICON hClassViewIcon = (HICON) ::LoadImage(::AfxGetResourceHandle(), MAKEINTRESOURCE(bHiColorIcons ? IDI_CLASS_VIEW_HC : IDI_CLASS_VIEW), IMAGE_ICON, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), 0);
	m_wndClassView.SetIcon(hClassViewIcon, FALSE);

	HICON hOutputBarIcon = (HICON) ::LoadImage(::AfxGetResourceHandle(), MAKEINTRESOURCE(bHiColorIcons ? IDI_OUTPUT_WND_HC : IDI_OUTPUT_WND), IMAGE_ICON, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), 0);
	m_wndOutput.SetIcon(hOutputBarIcon, FALSE);

	HICON hPropertiesBarIcon = (HICON) ::LoadImage(::AfxGetResourceHandle(), MAKEINTRESOURCE(bHiColorIcons ? IDI_PROPERTIES_WND_HC : IDI_PROPERTIES_WND), IMAGE_ICON, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), 0);
	m_wndProperties.SetIcon(hPropertiesBarIcon, FALSE);

}

BOOL CMainFrame::CreateCaptionBar()
{
	if (!m_wndCaptionBar.Create(WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS, this, ID_VIEW_CAPTION_BAR, -1, TRUE))
	{
		TRACE0("Failed to create caption bar\n");
		return FALSE;
	}

	BOOL bNameValid;

    CString strTemp, strTemp2U_;
	bNameValid = strTemp.LoadString(IDS_CAPTION_BUTTON);
	ASSERT(bNameValid);
	m_wndCaptionBar.SetButton(strTemp, ID_TOOLS_OPTIONS, CMFCCaptionBar::ALIGN_LEFT, FALSE);
	bNameValid = strTemp.LoadString(IDS_CAPTION_BUTTON_TIP);
	ASSERT(bNameValid);
	m_wndCaptionBar.SetButtonToolTip(strTemp);

	bNameValid = strTemp.LoadString(IDS_CAPTION_TEXT);
	ASSERT(bNameValid);
	m_wndCaptionBar.SetText(strTemp, CMFCCaptionBar::ALIGN_LEFT);

	m_wndCaptionBar.SetBitmap(IDB_INFO, RGB(255, 255, 255), FALSE, CMFCCaptionBar::ALIGN_LEFT);
	bNameValid = strTemp.LoadString(IDS_CAPTION_IMAGE_TIP);
	ASSERT(bNameValid);
	bNameValid = strTemp2.LoadString(IDS_CAPTION_IMAGE_TEXT);
	ASSERT(bNameValid);
	m_wndCaptionBar.SetImageToolTip(strTemp, strTemp2);

	// Laitetaan aluksi captionbar piiloon
	m_wndCaptionBar.ShowWindow(FALSE);

	return TRUE;
}
#endif // FMI_DISABLE_MFC_FEATURE_PACK

// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CFmiUsedFrameWndParent::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CFmiUsedFrameWndParent::Dump(dc);
}
#endif //_DEBUG


// CMainFrame message handlers

#ifndef FMI_DISABLE_MFC_FEATURE_PACK
void CMainFrame::OnViewCustomize()
{
	CMFCToolBarsCustomizeDialog* pDlgCust = new CMFCToolBarsCustomizeDialog(this, TRUE /* scan menus */);
	pDlgCust->EnableUserDefinedToolbars();
	pDlgCust->Create();
}

LRESULT CMainFrame::OnToolbarCreateNew(WPARAM wp,LPARAM lp)
{
	LRESULT lres = CFrameWndEx::OnToolbarCreateNew(wp,lp);
	if (lres == 0)
	{
		return 0;
	}

	CMFCToolBar* pUserToolbar = (CMFCToolBar*)lres;
	ASSERT_VALID(pUserToolbar);

	BOOL bNameValid;
    CString strCustomizeU_;
	bNameValid = strCustomize.LoadString(IDS_TOOLBAR_CUSTOMIZE);
	ASSERT(bNameValid);

	pUserToolbar->EnableCustomizeButton(TRUE, ID_VIEW_CUSTOMIZE, strCustomize);
	return lres;
}

void CMainFrame::OnApplicationLook(UINT id)
{
	CWaitCursor wait;

	theApp.m_nAppLook = id;

	switch (theApp.m_nAppLook)
	{
	case ID_VIEW_APPLOOK_WIN_2000:
		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManager));
		break;

	case ID_VIEW_APPLOOK_OFF_XP:
		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerOfficeXP));
		break;

	case ID_VIEW_APPLOOK_WIN_XP:
		CMFCVisualManagerWindows::m_b3DTabsXPTheme = TRUE;
		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows));
		break;

	case ID_VIEW_APPLOOK_OFF_2003:
		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerOffice2003));
		CDockingManager::SetDockingMode(DT_SMART);
		break;

	case ID_VIEW_APPLOOK_VS_2005:
		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerVS2005));
		CDockingManager::SetDockingMode(DT_SMART);
		break;

	default:
		switch (theApp.m_nAppLook)
		{
		case ID_VIEW_APPLOOK_OFF_2007_BLUE:
			CMFCVisualManagerOffice2007::SetStyle(CMFCVisualManagerOffice2007::Office2007_LunaBlue);
			break;

		case ID_VIEW_APPLOOK_OFF_2007_BLACK:
			CMFCVisualManagerOffice2007::SetStyle(CMFCVisualManagerOffice2007::Office2007_ObsidianBlack);
			break;

		case ID_VIEW_APPLOOK_OFF_2007_SILVER:
			CMFCVisualManagerOffice2007::SetStyle(CMFCVisualManagerOffice2007::Office2007_Silver);
			break;

		case ID_VIEW_APPLOOK_OFF_2007_AQUA:
			CMFCVisualManagerOffice2007::SetStyle(CMFCVisualManagerOffice2007::Office2007_Aqua);
			break;
		}

		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerOffice2007));
		CDockingManager::SetDockingMode(DT_SMART);
	}

	RedrawWindow(NULL, NULL, RDW_ALLCHILDREN | RDW_INVALIDATE | RDW_UPDATENOW | RDW_FRAME | RDW_ERASE);

	theApp.WriteInt(_T("ApplicationLook"), theApp.m_nAppLook);
}

void CMainFrame::OnUpdateApplicationLook(CCmdUI* pCmdUI)
{
	pCmdUI->SetRadio(theApp.m_nAppLook == pCmdUI->m_nID);
}

void CMainFrame::OnViewCaptionBar()
{
	m_wndCaptionBar.ShowWindow(m_wndCaptionBar.IsVisible() ? SW_HIDE : SW_SHOW);
	RecalcLayout(FALSE);
}

void CMainFrame::OnUpdateViewCaptionBar(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(m_wndCaptionBar.IsVisible());
}
#endif // FMI_DISABLE_MFC_FEATURE_PACK

BOOL CMainFrame::LoadFrame(UINT nIDResource, DWORD dwDefaultStyle, CWnd* pParentWnd, CCreateContext* pContext)
{
#ifdef FMI_DISABLE_MFC_FEATURE_PACK
	// TODO: Add your specialized code here and/or call the base class
	return CFmiUsedFrameWndParent::LoadFrame(nIDResource, dwDefaultStyle, pParentWnd, pContext);
#else
	// base class does the real work
	if (!CFmiUsedFrameWndParent::LoadFrame(nIDResource, dwDefaultStyle, pParentWnd, pContext))
	{
		return FALSE;
	}

	// enable customization button for all user toolbars
	BOOL bNameValid;
    CString strCustomizeU_;
	bNameValid = strCustomize.LoadString(IDS_TOOLBAR_CUSTOMIZE);
	ASSERT(bNameValid);

	for (int i = 0; i < iMaxUserToolbars; i ++)
	{
		CMFCToolBar* pUserToolbar = GetUserToolBarByIndex(i);
		if (pUserToolbar != NULL)
		{
			pUserToolbar->EnableCustomizeButton(TRUE, ID_VIEW_CUSTOMIZE, strCustomize);
		}
	}

	return TRUE;
#endif // FMI_DISABLE_MFC_FEATURE_PACK
}

void CMainFrame::OnClose()
{
	bool displayNotCloseDlg = itsDoc->IsOperationalModeOn() ? false : true;

	CCloseDlg closeDlg;
	if(displayNotCloseDlg || closeDlg.DoModal() == IDOK)
	{
		{
            theApp.AllowApplicationToClose(true);
			itsDoc->LogMessage("Give Working-threads 15 s time to stop.", CatLog::Severity::Info, CatLog::Category::Operational);
			DoAppDataBaseCollection(NFmiApplicationDataBase::kClose);
			NFmiLedLightStatusSystem::ProgramStopsNow();

            // HUOM! Jos WmsSupport:in alustus on ep‰onnistunut, ei saa tehd‰ tappok‰sky‰ ja odottelua
            // koska mystisest‰ syyst‰ t‰llˆin CFmiDataLoadingThread2:en lopetus ep‰onnistuu DEBUG moodissa (ei release).
#ifndef DISABLE_CPPRESTSDK
			auto& combinedMapHandler = *itsDoc->GetCombinedMapHandler();
            if(combinedMapHandler.wmsSupportAvailable())
				combinedMapHandler.getWmsSupport()->kill();
#endif // DISABLE_CPPRESTSDK
			CFmiCombineDataThread::CloseNow(); // sama t‰ss‰ combineData-threadille
			CFmiSoundingIndexDataThread::CloseNow(); // sama t‰ss‰ soundingIndexData-threadille
			CFmiMacroParamUpdateThread::CloseNow(); // t‰ss‰ sama macroParam p‰ivitys -threadille
			QueryDataToLocalCacheLoaderThread::CloseNow(); // t‰ss‰ sama queryData cachetus -threadeille (3 kpl kerralla)
            NFmiSatelliteImageCacheSystem::StopUpdateThreads();
#ifndef DISABLE_CPPRESTSDK
            itsDoc->WarningCenterSystem().kill();
#endif // DISABLE_CPPRESTSDK
            CFmiDataLoadingThread2::CloseNow(); // sama t‰ss‰ combineData-threadille

        	CSmartMetDoc* doc = (CSmartMetDoc*)GetActiveDocument();
			doc->DoOnClose(); // T‰ss‰ tehd‰‰n tietyt asiat, ennen kuin menn‰‰n NFmiEditMapGeneralDataDoc::StoreSupplementaryData -metodiin.

			if(CFmiCombineDataThread::WaitToClose(6 * 1000))
				itsDoc->LogMessage("CombineData working-thread stopped, continue closing...", CatLog::Severity::Info, CatLog::Category::Operational);
			else
				itsDoc->LogMessage("CombineData working-thread didn't stop, continue closing anyway...", CatLog::Severity::Error, CatLog::Category::Operational);
			if(CFmiSoundingIndexDataThread::WaitToClose(6 * 1000))
				itsDoc->LogMessage("soundingIndexData working-thread stopped, continue closing...", CatLog::Severity::Info, CatLog::Category::Operational);
			else
				itsDoc->LogMessage("soundingIndexData working-thread didn't stop, continue closing anyway...", CatLog::Severity::Error, CatLog::Category::Operational);
			if(CFmiMacroParamUpdateThread::WaitToClose(5 * 1000))
				itsDoc->LogMessage("MacroParam update-thread stopped, continue closing...", CatLog::Severity::Info, CatLog::Category::Operational);
			else
				itsDoc->LogMessage("MacroParam update-thread didn't stop, continue closing anyway...", CatLog::Severity::Error, CatLog::Category::Operational);
			if(CFmiDataLoadingThread2::WaitToClose(12 * 1000))
				itsDoc->LogMessage("DataLoadingThread-thread stopped, continue closing...", CatLog::Severity::Info, CatLog::Category::Operational);
			else
				itsDoc->LogMessage("DataLoadingThread-thread didn't stop, continue closing anyway...", CatLog::Severity::Error, CatLog::Category::Operational);
			WaitQDataCacheThreadsToStop();
			if(CFmiAppDataToDbThread::WaitToClose(10 * 1000))
				itsDoc->LogMessage("AppDataToDbThread-thread stopped, continue closing...", CatLog::Severity::Info, CatLog::Category::Operational);
			else
				itsDoc->LogMessage("AppDataToDbThread-thread didn't stop, continue closing anyway...", CatLog::Severity::Error, CatLog::Category::Operational);
            if(NFmiSatelliteImageCacheSystem::WaitUpdateThreadsToStop(1*1000))
                itsDoc->LogMessage("SatelliteImageCache-threads stopped, continue closing...", CatLog::Severity::Info, CatLog::Category::Operational);
            else
                itsDoc->LogMessage("SatelliteImageCache-threads didn't stop, continue closing anyway...", CatLog::Severity::Error, CatLog::Category::Operational);
#ifndef DISABLE_CPPRESTSDK
            if(itsDoc->WarningCenterSystem().isDead(std::chrono::milliseconds(10 * 1000)))
                itsDoc->LogMessage("Hake message -threads stopped, continue closing...", CatLog::Severity::Info, CatLog::Category::Operational);
            else
                itsDoc->LogMessage("Hake message -threads didn't stop, continue closing anyway...", CatLog::Severity::Error, CatLog::Category::Operational);

            if(combinedMapHandler.wmsSupportAvailable())
            {
                if(combinedMapHandler.waitWmsSupportToDie(std::chrono::milliseconds(10 * 1000)))
                    itsDoc->LogMessage("WmsSupport -threads stopped, continue closing...", CatLog::Severity::Info, CatLog::Category::Operational);
                else
                    itsDoc->LogMessage("WmsSupport -threads didn't stop, continue closing anyway...", CatLog::Severity::Error, CatLog::Category::Operational);
            }
#endif // DISABLE_CPPRESTSDK

            // N‰m‰ pit‰‰ tehd‰ vasta kun working-thread on lopettanut, koska muuten voi tulla ongelmia
			// koska dokumentin StoreSupplementaryData tuhoaa ne dll:t, jotka muuten ehk‰ viel‰ lukisivat
			// datoja.
			doc->StoreData(true);
			doc->UpdateTrajectorySystem(); // pakko p‰ivitt‰‰ arvoja ennen talletusta
			itsDoc->StoreSupplementaryData();

			itsDoc->MakeClosingLogMessage();
		}
		CFmiUsedFrameWndParent::OnClose();
	}
}

void CMainFrame::WaitQDataCacheThreadsToStop()
{
	int waitTimeInSeconds = 10;
	if(QueryDataToLocalCacheLoaderThread::WaitToClose(waitTimeInSeconds * 1000))
		itsDoc->LogMessage("QueryData to local cache copying threads stopped, continue closing...", CatLog::Severity::Info, CatLog::Category::Operational);
	else
		itsDoc->LogMessage("Some problem with QueryData to local cache copying threads stoppage, continue closing anyway...", CatLog::Severity::Error, CatLog::Category::Operational);
}

void CMainFrame::ParameterSelectionSystemUpdateTimerStart(int waitTimeInSeconds)
{
    itsParameterSelectionSystemUpdateTimer = static_cast<UINT>(SetTimer(kFmiParameterSelectionSystemUpdateTimer, waitTimeInSeconds * 1000, NULL));
}

void CMainFrame::UpdateCrashRptLogFile()
{
    CSmartMetApp* app = (CSmartMetApp*)AfxGetApp();
    app->UpdateCrashRptLogFile();
}

void CMainFrame::TrimmInMemoryLogMessages()
{
    CatLog::trimmOldestMessages(CatLog::Severity::Trace);
}

void CMainFrame::OnTimer(UINT_PTR nIDEvent)
{
	static int counter = 0;
	counter++;
//	TRACE("%7d nIDEvent = %d\n", counter, nIDEvent);
	CSmartMetDoc* cdoc = (CSmartMetDoc*)GetActiveDocument();
	if(!cdoc)
		return;

	switch(nIDEvent)
	{
		case kFmiLedLightsActionTimer:
		{
			DoLedLightsActionUpdates();
			return;
		}

		case kFmiCheckAnimationLockedModeTimeBagsTimer:
		{
			itsDoc->GetCombinedMapHandler()->checkAnimationLockedModeTimeBags(CtrlViewUtils::kDoAllMapViewDescTopIndex, false);
			return;
		}
		case kFmiCheckForNewSatelDataTimer:
		{
			KillTimer(itsCheckForNewSatelDataTimer);
			int checkFrequenceInMinutes = itsDoc->SatelDataRefreshTimerInMinutes();
			if(checkFrequenceInMinutes > 0)
			{
				itsDoc->GetCombinedMapHandler()->checkForNewConceptualModelData();
			}
			else
				checkFrequenceInMinutes = 1; // k‰ynnistet‰‰n t‰m‰ timeri kuitenkin ainakin kerran minuutissa, jos joku muuttaa asetuksia, t‰llˆin ei tarvitse erikseen k‰ynnist‰‰ timeria uudestaan
			itsCheckForNewSatelDataTimer = static_cast<UINT>(SetTimer(kFmiCheckForNewSatelDataTimer, checkFrequenceInMinutes * 60 * 1000, NULL));

			return;
		}

		case kFmiCleanDataTimer:
		{
			KillTimer(itsCleanDataTimer);
			double cleaningFreqInMinutes = itsDoc->FileCleanerSystem().CleaningTimeStepInHours() * 60;
			if(cleaningFreqInMinutes > 0)
			{
				itsDoc->CleanDataDirectories();
				itsCleanDataTimer = static_cast<UINT>(SetTimer(kFmiCleanDataTimer, static_cast<UINT>(cleaningFreqInMinutes * 60 * 1000), NULL));
			}
			return ;
		}

		case kFmiDebugDataSizeLoggerTimer:
		{
			static bool firstTime = true;
			KillTimer(itsDebugDataSizeLoggerTimer);
			if(firstTime)
			{
				firstTime = false;
				itsDebugDataSizeLoggerTimer = static_cast<UINT>(SetTimer(kFmiDebugDataSizeLoggerTimer, static_cast<UINT>(2 * 60 * 1000), NULL)); // 1. raportin j‰lkeen tehd‰‰n yksi raportti nopeammin
			}
			else
				itsDebugDataSizeLoggerTimer = static_cast<UINT>(SetTimer(kFmiDebugDataSizeLoggerTimer, static_cast<UINT>(20 * 60 * 1000), NULL)); // 2. raportin j‰lkeen tehd‰‰n loput raportit hieman harvemmin
			itsDoc->ReportInfoOrganizerDataConsumption();
			itsDoc->ReportProcessMemoryUsage();
			return ;
		}

		case kFmiAutoSaveTimer:
		{ // tapetaan ja k‰ynnistet‰‰n autosave timer (vaikka se ei olisi k‰ytˆss‰)
			KillTimer(itsAutoSaveTimer);
			cdoc->DoAutoSave();

			int autoSaveFreqInMinutes = itsDoc->MetEditorOptionsData().AutoSaveFrequensInMinutes();
			itsAutoSaveTimer = static_cast<UINT>(SetTimer(kFmiAutoSaveTimer, autoSaveFreqInMinutes * 60 * 1000, NULL));
			return;
		}

		case kFmiAutoStartUpLoadTimer:
		{
			KillTimer(itsAutoStartUpLoadTimer);
			itsDoc->TryAutoStartUpLoad();
			if(itsDoc->EditedDataNeedsToBeLoaded())
			{
				itsAutoStartUpLoadTimer = static_cast<UINT>(SetTimer(kFmiAutoStartUpLoadTimer, 10 * 1000, NULL));
                ApplicationInterface::GetSmartMetView()->Update(); // t‰m‰n avulla elapsed time laskuri p‰ivittyy p‰‰karttan‰ytˆss‰
			}
			return;
		}

		case kFmiToggleFlagButtonTimer:
		{
			static bool lastWasRed = false;
			KillTimer(itsToggleFlagButtonTimer);
			if(itsDoc->IsEditedDataInReadOnlyMode() && itsFlagButtonIndex != -1)
			{
				if(lastWasRed)
					itsToolBarImagelist.Replace(itsFlagButtonIndex, itsOrangeFlagBitmap, NULL);
				else
					itsToolBarImagelist.Replace(itsFlagButtonIndex, itsRedFlagBitmap, NULL);
				lastWasRed = !lastWasRed;
				itsToggleFlagButtonTimer = static_cast<UINT>(SetTimer(kFmiToggleFlagButtonTimer, 1 * 1000, NULL));
			}
			else
				itsToolBarImagelist.Replace(itsFlagButtonIndex, itsOrangeFlagBitmap, NULL); // viimeiseksi laitetaan oranssi lippu p‰‰lle
			m_wndToolBar.Invalidate(FALSE);
			if(!lastWasRed && itsDoc->RunningTimeInSeconds() > 5 * 60)
				KillTimer(itsToggleFlagButtonTimer); // jos SmartMet on ollut k‰ynniss‰ yli 5 minuuttia ja viimeksi on ollut oranssi lippu p‰‰ll‰, lopetetaan lipun vilkutus kokonaan
			return;
		}

		case kFmiDataToDBUpdateTimer:
		{
			DoAppDataBaseCollection(NFmiApplicationDataBase::kUpdate);
			return;
		}


		case kFmiCleanOldDataFromMemoryTimer:
		{
			itsDoc->CleanUnusedDataFromMemory();
			return;
		}

        case kFmiStoreViewPosToWinRegistryTimer:
		{
            cdoc->SaveViewPositionsToRegistry();
			return;
		}

        case kFmiStoreCrashBackupViewMacroTimer:
        {
			itsDoc->StoreBackUpViewMacro(false);
            return;
        }

        case kFmiGenerateBetaProductsTimer:
        {
			// T‰m‰ tehd‰‰n aina minuutin v‰lein, siksi laitan t‰h‰n myˆs tarkastelut siit‰
			// ett‰ onko joku/jotkin querydatat myˆh‰ss‰. Tehd‰‰n se ensin, koska tarkastelu on nopea.
			itsDoc->DoIsAnyQueryDataLateChecks();

			// Lopuksi tehd‰‰n 1) beta-tuotteiden generointi tarkastelut, jotka saattavat jumittaa 
			// smartmetin pidemm‰ksikin aikaa. 2) MacroParamData tuotanto
			// Huom! Molemmat on pakko tehd‰ samalla funktiolla.
            itsDoc->DoGenerateAutomationProductChecks();
            return;
        }

        case kFmiParameterSelectionSystemUpdateTimer:
        {
            KillTimer(itsParameterSelectionSystemUpdateTimer);
            itsDoc->UpdateParameterSelectionSystem();
            return;
        }

        case kFmiLoggingSystemManagementTimer:
        {
            UpdateCrashRptLogFile();
            TrimmInMemoryLogMessages();
            return;
        }

        case kFmiNewQueryDataReadUpdateViewsTimer:
        {
            KillTimer(g_NewQueryDataReadUpdateViewsTimerId);
            ((CSmartMetDoc*)GetActiveDocument())->UpdateAllViewsAndDialogs("CMainFrame: Loaded new data in SmartMet's database: " + g_NewQueryDataReadList);
            g_NewQueryDataReadList.clear();
            g_NewQueryDataReadUpdateViewsTimerFlag.clear();
            return;
        }

		case kFmiWmsSupportRenewerTimer:
		{
			itsDoc->GetCombinedMapHandler()->startWmsSupportRenewalProcess(false);
			return;
		}
    }

	CFmiUsedFrameWndParent::OnTimer(nIDEvent);
}

void CMainFrame::CheckForAutoLoadTimerStart()
{
	if(itsDoc->EditedDataNeedsToBeLoaded())
	{
		itsAutoStartUpLoadTimer = static_cast<UINT>(SetTimer(kFmiAutoStartUpLoadTimer, 10 * 1000, NULL)); // pistet‰‰n 10 sekunnin intervallilla tarkastamaan lˆytyykˆ editoitavaa pohja dataa
	}
}

void CMainFrame::PutWarningFlagTimerOn()
{
	if(itsDoc->EditedSmartInfo() && itsDoc->IsEditedDataInReadOnlyMode() && itsFlagButtonIndex != -1)
	{
		itsToggleFlagButtonTimer = static_cast<UINT>(SetTimer(kFmiToggleFlagButtonTimer, 1 * 1000, NULL)); // pistet‰‰n 1 sekunnin intervallilla vaihtamaan lippu-napi kuvaa
	}
}

void CMainFrame::StartDataLoadingWorkingThread()
{
	// datan lataus thread k‰ynnistys, jos ei olla ns. normaali moodissa
	if(itsDoc->EditorModeDataWCTR()->EditorMode() != NFmiMetEditorModeDataWCTR::kNormal)
	{
		if(itsDoc->EditorModeDataWCTR()->InNormalModeStillInDataLoadDialog())
			return ; // ollaan oikeasti normaali moodissa ja datanlataus dialogi on auki, t‰llˆin ei laitetan data threadeja p‰‰lle

		static bool firstTime = true;
		if(firstTime)
		{
			firstTime = false;

            // T‰m‰ doExternalDataThreads juttu on ongelmien debuggaamis avustusta, eli
            // jos ei halua kaiken maailman datan haku systeemien tekev‰n tˆit‰‰n, laita t‰h‰n false.
            const bool doExternalDataThreads = true;
            if(doExternalDataThreads)
            {
                if(itsDoc->ApplicationWinRegistry().GenerateTimeCombinationData())
                {
                    // K‰ynnistet‰‰n CombineData-threadi vain yhden kerran, jolloin se j‰‰ sitten pyˆrim‰‰n sinne itsekseen,
                    // kunnes sille sanotaan ett‰ lopeta (ohjelmaa suljettaessa).
                    // T‰m‰ on hyv‰ sijoittaa t‰nne normaalin working-threadin k‰ynnistykseen, koska aina ei haluta ett‰
                    // kyseinen threadi l‰htee edes pyˆrim‰‰n.
                    CFmiCombineDataThread::InitCombineDataInfos(*itsDoc->HelpDataInfoSystem(), itsDoc->ApplicationDataBase().GetDecodedApplicationDirectory());
                    int combineThreadDelayInMS = 60 * 1000;
                    if(itsDoc->MachineThreadCount() >= 6)
                        combineThreadDelayInMS = 30 * 1000; // jos konessa on paljon coreja, ei tarvitse viivytt‰‰ alkua niin paljoa
                    CFmiCombineDataThread::SetFirstTimeDelay(combineThreadDelayInMS);
                    if((itsDisableThreadsVariable & gDisableCombineDataThread) == 0)
                        CWinThread* combineDataThread = AfxBeginThread(CFmiCombineDataThread::DoThread, nullptr, THREAD_PRIORITY_BELOW_NORMAL);
                }
                // k‰ynnistet‰‰n myˆs SoundingIndexDataThread kerran, se pit‰‰ myˆs ensin initialisoida.
                CFmiSoundingIndexDataThread::InitSoundingIndexDataInfos(*(itsDoc->HelpDataInfoSystem()), itsDoc->AutoGeneratedSoundingIndexBasePath()); // t‰m‰ pit‰‰ siirt‰‰ threadin k‰ynnistyst‰ edelt‰v‰ksi kohdaksi
                int soundingIndexThreadDelayInMS = 70 * 1000;
                if(itsDoc->MachineThreadCount() >= 6)
                    soundingIndexThreadDelayInMS = 35 * 1000; // jos konessa on paljon coreja, ei tarvitse viivytt‰‰ alkua niin paljoa
                CFmiSoundingIndexDataThread::SetFirstTimeDelay(soundingIndexThreadDelayInMS);
                if((itsDisableThreadsVariable & gDisableSoundingIndexDataThread) == 0)
                    CWinThread* soundingIndexDataThread = AfxBeginThread(CFmiSoundingIndexDataThread::DoThread, nullptr, THREAD_PRIORITY_BELOW_NORMAL);

                // K‰ynnistet‰‰n Hake sanomien luku, jos on jotain dataa luettavaksi
#ifndef DISABLE_CPPRESTSDK
                if(itsDoc->WarningCenterSystem().isThereAnyWorkToDo())
                {
					// Hake sanomat luetaan nykyisin Dropboxista lokaali levylt‰, joten voidaan aikaistaa t‰t‰ k‰ynnistyst‰ esim. 10 sekuntiin
					int hakeWarningThreadDelayInMS = 10 * 1000;
                    itsDoc->WarningCenterSystem().setUpdateApplicationCallback(&CFmiHakeWarningMessages::UpdateApplicationAfterChanges);
                    itsDoc->WarningCenterSystem().goToWorkAfter(std::chrono::milliseconds(hakeWarningThreadDelayInMS));
                }
#endif // DISABLE_CPPRESTSDK

            } // doExternalDataThreads

			// data loading threadin k‰ynnistys 
            // T‰m‰ on doExternalDataThreads -blokin ulkona, koska t‰t‰ ei haluta yleens‰ edes debuggauksessa laittaa pois p‰‰lt‰.
			// HUOM! t‰m‰ pit‰‰ aloittaa viimeisen‰, koska muut threadit (ainakin CFmiSoundingIndexDataThread) voivat lis‰t‰ luettavia datoja.
			CFmiDataLoadingThread2::InitDynamicHelpDataInfo(*itsDoc->HelpDataInfoSystem(), itsDoc->DataNotificationSettings() , itsDoc->Language());
			if((itsDisableThreadsVariable & gDisableDataLoaderThread) == 0)
				CWinThread *dataLoadingThread = AfxBeginThread(CFmiDataLoadingThread2::DoThread, nullptr, THREAD_PRIORITY_BELOW_NORMAL);
		}
	}
}

void CMainFrame::OnViewSetMainFramePlaceToDefault()
{
    MoveWindow(CMainFrame::ViewPosRegistryInfo().DefaultWindowRect());
    Persist2::WriteWindowRectToWinRegistry(itsDoc->ApplicationWinRegistry(), CMainFrame::ViewPosRegistryInfo().WinRegistryKeyStr(), this);
}

void CMainFrame::OnUpdateFrameTitle(BOOL bAddToTitle)
{
	if(itsDoc)
	{
        this->SetTitle(CA2T(itsDoc->GetApplicationTitle().c_str())); // t‰m‰ on v‰h‰n outo paikka s‰‰t‰‰ framen title‰, mutta OnCreate:ssa ei ole viel‰ dokumenttia saatavilla

		boost::shared_ptr<NFmiFastQueryInfo> info = itsDoc->EditedSmartInfo();
		if(info && dynamic_cast<NFmiSmartInfo*>(info.get())->LoadedFromFile())
		{
			CFmiUsedFrameWndParent::OnUpdateFrameTitle(bAddToTitle);
			return ;
		}
	}
	// Jos ei ole dataa ladatta tiedostosta, j‰‰ ohjelman nimeksi pelkk‰ ohjelman nimi,
	// ei "dokumentti - ohjelmannimi"
	SetWindowText(m_strTitle); // m_strTitle sis‰lt‰‰ alkuper‰isen ohjelman nimen, mik‰ on annettu IDR_MAINFRAME:ssa
}

void CMainFrame::OnWorkinThreadDataRead2()
{
	std::vector<LoadedQueryDataHolder> qDatas;
	if(CFmiDataLoadingThread2::GetLoadedDatas(qDatas))
	{
        std::string loadedFileNames;
		for(size_t i =0; i < qDatas.size(); i++)
		{
			LoadedQueryDataHolder &tmp = qDatas[i];
			// HUOM! en voinut tehd‰ shared_ptr virityst‰ t‰h‰n datan siirtoon, koska siin‰ olevaa pointteria ei voi vaputtaa ja
			// olen tekem‰ss‰ asiaan liittyv‰‰ suurempaa remonttia SmartMet 6.0:aan. Lis‰ksi auto_ptr-otuksia ei voi laittaa std-containereihin.
			// Siksi t‰m‰ tiedon siirto vuotaa muistia, jos esim. ohjelma lopetetaan kesken kaiken.
            if(tmp.itsQueryData && tmp.itsQueryData->Info())
            {
				bool dataWasDeleted = false;
                itsDoc->AddQueryData(tmp.itsQueryData.release(), tmp.itsDataFileName, tmp.itsDataFilePattern, tmp.itsDataType, tmp.itsNotificationStr, false, dataWasDeleted);
                if(!loadedFileNames.empty())
                    loadedFileNames += ", ";
                loadedFileNames += tmp.itsDataFileName;
            }
			else 
			{    // HUOM! joskus tulee ehk‰ jonkin luku virheen takia queryData, jonka rawData on roskaa ja info on 0-pointteri. Sellainen data ignoorataan 
				 // t‰ss‰, huom vuotaa muistia, koska en voi deletoida kun rawData-pointteri osoittaa ties minne.
				std::string errMessage = "Data loading failed, following data was read, but it was invalid and not used: ";
				errMessage += tmp.itsDataFileName;
				itsDoc->LogMessage(errMessage, CatLog::Severity::Error, CatLog::Category::Operational);
			}
		}
        StartNewQueryDataLoadedUpdateTimer(loadedFileNames);
	}
}

void CMainFrame::StartNewQueryDataLoadedUpdateTimer(const std::string &loadedFileNames)
{
    // If atomic flag was not in set state, start update timer
    if(!g_NewQueryDataReadUpdateViewsTimerFlag.test_and_set(std::memory_order_acquire))
    {
        // When there is new query data loaded, we want to wait few seconds before we update all views, so that we are not in update cyle all the time
        // because new query data files  may come in bunches.
        g_NewQueryDataReadUpdateViewsTimerId = static_cast<UINT>(SetTimer(kFmiNewQueryDataReadUpdateViewsTimer, static_cast<UINT>(5 * 1000), NULL));
    }
    // Adding these loaded file names to final views update message file list.
    if(!g_NewQueryDataReadList.empty())
        g_NewQueryDataReadList += ",";
    g_NewQueryDataReadList += loadedFileNames;
}

void CMainFrame::GetNewWarningMessages()
{
#ifndef DISABLE_CPPRESTSDK
    if(itsDoc)
	{
        itsDoc->CheckForNewWarningMessageData();
        if(itsDoc->WarningCenterSystem().getLegacyData().WarningCenterViewOn())
        {
            ApplicationInterface::GetApplicationInterfaceImplementation()->ApplyUpdatedViewsFlag(SmartMetViewId::AllMapViews);
            itsDoc->RefreshApplicationViewsAndDialogs("CMainFrame: New Warning messages read", TRUE, TRUE, 0); // p‰ivitet‰‰n p‰‰ ikkunaa, jos varoitus dialogi on p‰‰ll‰
        }
	}
#endif // DISABLE_CPPRESTSDK
}

void CMainFrame::DoMacroParamUpdate()
{
	if(itsDoc)
	{
		if(CFmiMacroParamUpdateThread::MakePossibleMacroParamSystemUpdateInDoc(itsDoc->MacroParamSystem()))
		{
			static bool firstTime = true;
			if(firstTime)
			{ // Kun 1. kerran on luettu macroParamit sis‰‰n, pit‰‰ varmistaa kaikkien ruutujen p‰ivitys. Koska jos joku on jo ladannut vieMakron, jossa
				// on makroParameita, ne pit‰‰ piirt‰‰ nyt uudestaan.
				firstTime = false;
				itsDoc->GetCombinedMapHandler()->mapViewDirty(CtrlViewUtils::kDoAllMapViewDescTopIndex, false, true, true, false, false, true);
			}
            ApplicationInterface::GetApplicationInterfaceImplementation()->ApplyUpdatedViewsFlag(SmartMetViewId::AllMapViews | SmartMetViewId::CrossSectionView | SmartMetViewId::TimeSerialView | SmartMetViewId::SmartToolDlg);
            itsDoc->RefreshApplicationViewsAndDialogs("CMainFrame: Macro params have been updated"); // t‰m‰n on tarkoitus p‰ivitt‰‰ vain SmartToolView, mutta sill‰ ei ole omaa p‰ivitys k‰sky‰ (ainakaan viel‰)
		}
	}
}

BOOL CMainFrame::OnWndMsg(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
//	if(message == ID_MESSAGE_WORKING_THREAD_DATA_READ)
	//	OnWorkinThreadDataRead();
	if(message == ID_MESSAGE_WORKING_THREAD_DATA_READ2)
		OnWorkinThreadDataRead2();
	else if(message == ID_MESSAGE_NEW_HAKE_WARNING_AVAILABLE)
		GetNewWarningMessages();
	else if(message == ID_MESSAGE_MACRO_PARAMS_UPDATE)
		DoMacroParamUpdate();
    else if(message == ID_MESSAGE_START_HISTORY_THREAD)
        StartHistoryDataCacheThread();

	return CFmiUsedFrameWndParent::OnWndMsg(message, wParam, lParam, pResult);
}

// t‰m‰n overriden avulla saadaan tooltippiin kieli versio aikaiseksi
// eli t‰m‰ tooltip on toolbar-nappuloiden tooltippi
BOOL CMainFrame::OnToolTipText(UINT id, NMHDR* pNMHDR, LRESULT* pResult)
{
	BOOL status = CFmiUsedFrameWndParent::OnToolTipText(id, pNMHDR, pResult);

	if(status)
	{
		CSmartMetApp* app = (CSmartMetApp*)AfxGetApp();
        TOOLTIPTEXT* pTTT = (TOOLTIPTEXT*)pNMHDR;
        UINT nID = static_cast<UINT>(pNMHDR->idFrom);
        CString strU_;
        strU_ = pTTT->szText;
        CString realStrU_(app->GetToolTipString(nID, strU_));
        _tcsncpy(pTTT->szText, realStrU_, static_cast<int>(_tcslen(pTTT->szText)));
	}
    return status;
}

// t‰m‰ hoitaa statusbarin tekstit moni kieli versiossa, kun toolbarin nappien avustus teksti‰ haetaan
void CMainFrame::GetMessageString(UINT nID, CString& rMessageU_) const
{
	// TODO: Add your specialized code here and/or call the base class

	CFmiUsedFrameWndParent::GetMessageString(nID, rMessageU_);

	CSmartMetApp* app = (CSmartMetApp*)AfxGetApp();
	// kielen vaihdon pit‰‰ tapahtua t‰ss‰ v‰liss‰ (statusbar teksti, joka saadaan toolbar + hiiri sen p‰‰ll‰)
    CString realStrU_ = app->GetToolTipString(nID, rMessageU_);
    rMessageU_ = realStrU_;

}

void CMainFrame::OnEnablenotifications()
{
	itsDoc->DataNotificationSettings().Use(true);
}

void CMainFrame::OnUpdateEnablenotifications(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(itsDoc->DataNotificationSettings().Use() == false);
}

void CMainFrame::OnDisablenotifications()
{
	itsDoc->DataNotificationSettings().Use(false);
}

void CMainFrame::OnUpdateDisablenotifications(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(itsDoc->DataNotificationSettings().Use() == true);
}

void CMainFrame::OnNotificationsoundson()
{
	itsDoc->DataNotificationSettings().UseSound(true);
}

void CMainFrame::OnUpdateNotificationsoundson(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(itsDoc->DataNotificationSettings().UseSound() == false);
}

void CMainFrame::OnNotificationsoundsoff()
{
	itsDoc->DataNotificationSettings().UseSound(false);
}

void CMainFrame::OnUpdateNotificationsoundsoff(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(itsDoc->DataNotificationSettings().UseSound() == true);
}

void CMainFrame::OnShowSystraySymbol()
{
	g_TrayIcon.Show();
    itsDoc->DataNotificationSettings().ShowIcon(true);
}

void CMainFrame::OnUpdateShowSystraySymbol(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(g_TrayIcon.IsHidden());
}

void CMainFrame::OnHideSystraySymbol()
{
	g_TrayIcon.Hide();
    itsDoc->DataNotificationSettings().ShowIcon(false);
}

void CMainFrame::OnUpdateHideSystraySymbol(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(g_TrayIcon.IsShowing());
}

#ifndef FMI_DISABLE_MFC_FEATURE_PACK
void CMainFrame::OnViewDemodockviews()
{
	static bool firstTime = true;

	if(firstTime)
	{
		firstTime = false;
		// create docking windows
		if (!CreateDockingWindows())
		{
			TRACE0("Failed to create docking windows\n");
			return ;
		}

		m_wndFileView.EnableDocking(CBRS_ALIGN_ANY);
		m_wndClassView.EnableDocking(CBRS_ALIGN_ANY);
		DockPane(&m_wndFileView);
		CDockablePane* pTabbedBar = NULL;
		m_wndClassView.AttachToTabWnd(&m_wndFileView, DM_SHOW, TRUE, &pTabbedBar);
		m_wndOutput.EnableDocking(CBRS_ALIGN_ANY);
		DockPane(&m_wndOutput);
		m_wndProperties.EnableDocking(CBRS_ALIGN_ANY);
		DockPane(&m_wndProperties);

		// laitetaan properties-osio (oikealla puolella) aluksi autohide-modeen ja piiloon
		m_wndProperties.ToggleAutoHide();
		m_wndProperties.Slide(FALSE, TRUE);
		// laitetaan output-osio (ala osassa) aluksi autohide-modeen ja piiloon
		m_wndOutput.ToggleAutoHide();
		m_wndOutput.Slide(FALSE, TRUE);
		// laitetaan fileView ja classView -osio (vasemmalla puolella) aluksi autohide-modeen ja piiloon
		// HUOM! t‰m‰ pit‰‰ tehd‰ niiden container-oliolla, koska suora m_wndFileView- tai 
		// m_wndClassView-olioiden k‰pistwely ei toimi. Syy on varmaan siin‰ ett‰ on useita (2 kpl) tabeja samassa.
		pTabbedBar->ToggleAutoHide();
		pTabbedBar->Slide(FALSE, TRUE);

		// Create a caption bar:
		if (!CreateCaptionBar())
		{
			TRACE0("Failed to create caption bar\n");
			return ;      // fail to create
		}

	}
}
#endif // FMI_DISABLE_MFC_FEATURE_PACK

void CMainFrame::DoAppDataBaseCollection(int theAction) // theAction = NFmiApplicationDataBase::Action
{
	itsDoc->ApplicationDataBase().CollectSmartMetData(static_cast<NFmiApplicationDataBase::Action>(theAction), itsDoc->Language(), itsDoc->RunningTimeInSeconds(), itsDoc->IsToolMasterAvailable(), itsDoc->InfoOrganizer());
	itsDoc->LogMessage(NFmiStringTools::UrlDecode(itsDoc->ApplicationDataBase().MakeUrlParamString()), CatLog::Severity::Info, CatLog::Category::NetRequest); // lokitetaan info aina ensin
	if(itsDoc->ApplicationDataBase().UseDataSending())
	{
        CWinThread *AppDataToDbThread = AfxBeginThread(CFmiAppDataToDbThread::DoThread, &itsDoc->ApplicationDataBase(), THREAD_PRIORITY_BELOW_NORMAL);
    }
}

void CMainFrame::OnGetMinMaxInfo(MINMAXINFO* lpMMI)
{
    // the minimum client rectangle (in that is lying the view window)
    CRect rc(0, 0, 250, 250);
    // compute the required size of the frame window rectangle
    // based on the desired client-rectangle size
    CalcWindowRect(rc);

    lpMMI->ptMinTrackSize.x = rc.Width();
    lpMMI->ptMinTrackSize.y = rc.Height();
}

