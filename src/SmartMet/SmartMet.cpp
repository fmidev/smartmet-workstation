
// SmartMet.cpp : Defines the class behaviors for the application.
//

//#include <vld.h> // poista tämä, jos visual leak detector aiheuttaa liikaa overheadia debug ajossa

#include "SmartMet.h"
#include "MainFrm.h"
#include "SmartMetDoc.h"
#include "SmartMetView.h"
#include "NFmiEditMapGeneralDataDoc.h"
#include "NFmiCmdLine.h"
#include "NFmiDictionaryFunction.h"
#include "splasher.h"
#include "FmiMyCommandLineInfo.h"
#include "SplashThreadHolder.h"
#include "NFmiApplicationDataBase.h"
#include "NFmiBasicSmartMetConfigurations.h"
#include "NFmiQueryData.h"
#include "FmiLogViewer.h"
#include "CtrlViewFunctions.h"
#include "QueryDataReading.h"
#include "NFmiApplicationWinRegistry.h"
#include "ApplicationInterfaceForSmartMet.h"
#include "ToolMasterColorCube.h"
#include "ToolMasterHelperFunctions.h"
#include "CombinedMapHandlerInterface.h"
#include "FmiCombineDataThread.h"

#include "boost/format.hpp"

namespace
{
	// Tämä osio SmartMetin asetuksia initialisoidaan ennen jättimäisen NFmiEditMapGeneralDataDoc -olion
	// alustusta. Tätä tarvitaan kun CrashRpt-systeemi alustetaan. Sitten kun lopulta alustetaan itse 
	// NFmiEditMapGeneralDataDoc -olio, annetaan tämä otus parametrina, jolloin tästä tehdään kopio 
	// NFmiEditMapGeneralDataDoc -olion sisäiseen vastaavaan dataosaan.
	NFmiBasicSmartMetConfigurations gBasicSmartMetConfigurations; 
}

#ifdef NDEBUG // laitetaan crash-report vain release versioon
	#ifdef _WIN64 // laitetaan crash-report vain 64-bit versioon
		#define DO_CRASH_REPORTING 1 // Laita tämä kommenttiin, jos et halua laittaa kaatumis raportointia
        #include "CrashRpt.h"
	#endif
#endif

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace Gdiplus;

// CSmartMetApp

BEGIN_MESSAGE_MAP(CSmartMetApp, CFmiUsedAppParent)

	ON_COMMAND(ID_APP_ABOUT, &CSmartMetApp::OnAppAbout)
	// Standard file based document commands
	ON_COMMAND(ID_FILE_NEW, &CFmiUsedAppParent::OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, &CFmiUsedAppParent::OnFileOpen)
	// Standard print setup command
	ON_COMMAND(ID_FILE_PRINT_SETUP, &CFmiUsedAppParent::OnFilePrintSetup)
END_MESSAGE_MAP()


// CSmartMetApp construction

CSmartMetApp::CSmartMetApp()
{
#ifndef FMI_DISABLE_MFC_FEATURE_PACK
	EnableHtmlHelp();
#endif // FMI_DISABLE_MFC_FEATURE_PACK

	m_bHiColorIcons = TRUE;

	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
	fUseToolMasterIfAvailable = true;
}

CSmartMetApp::~CSmartMetApp()
{
	CrashRptUninstall();
}

// The one and only CSmartMetApp object

CSmartMetApp theApp;
// This identifier was generated to be statistically unique for your app
// You may change it if you prefer to choose a specific identifier

// {680EBBCB-C820-462D-BA42-5FD06F65CF4D}
static const CLSID clsid =
{ 0x680EBBCB, 0xC820, 0x462D, { 0xBA, 0x42, 0x5F, 0xD0, 0x6F, 0x65, 0xCF, 0x4D } };

const GUID CDECL BASED_CODE _tlid =
		{ 0xE28FB933, 0xB37, 0x4FA5, { 0x9B, 0xC4, 0xCC, 0xE9, 0x80, 0x48, 0x1F, 0x10 } };
const WORD _wVerMajor = 1;
const WORD _wVerMinor = 0;

// CSmartMetApp initialization

BOOL CSmartMetApp::InitInstance()
{
#ifndef FMI_DISABLE_MFC_FEATURE_PACK
	// InitCommonControlsEx() is required on Windows XP if an application
	// manifest specifies use of ComCtl32.dll version 6 or later to enable
	// visual styles.  Otherwise, any window creation will fail.
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// Set this to include all the common control classes you want to use
	// in your application.
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);
#endif // FMI_DISABLE_MFC_FEATURE_PACK

	AfxInitRichEdit(); // Tämä pitää kutsua että ne dialogit, joissa on käytetty Rich-edit controllia toimisivat (mm. SmartToolDlg ja luotausnäytön txt-dialogi).
						// En tiedä kumpi muutos aiheutti ongelman: siirtyminen VC++ 2010:aan vai siirtyminen käyttämään MFC feature packia.

	CFmiUsedAppParent::InitInstance();

    // Tämä on kutsuttava ennen gBasicSmartMetConfigurations.Init -kutsua
    if(!ParseCommandLineArguments())
        return FALSE;
    // Tämä on kutsuttava aina ja ennen CrashRptInstall-kutsua ja ennen CSplashThreadHolder luontia, koska sinne tulee dynaamista tekstiä editorin versiota ja build päivämääristä
    if(!gBasicSmartMetConfigurations.Init(Toolmaster::MakeAvsToolmasterVersionString()))
        return FALSE;

	CSplashThreadHolder pSplashThreadHolder(SplashStart());

#ifndef FMI_DISABLE_MFC_FEATURE_PACK
	if (!AfxSocketInit())
	{
		AfxMessageBox(IDP_SOCKETS_INIT_FAILED);
		return FALSE;
	}
#endif // FMI_DISABLE_MFC_FEATURE_PACK

	// Initialize OLE libraries
	if (!AfxOleInit())
	{
		AfxMessageBox(IDP_OLE_INIT_FAILED);
		return FALSE;
	}
	AfxEnableControlContainer();
	// Standard initialization
	// If you are not using these features and wish to reduce the size
	// of your final executable, you should remove from the following
	// the specific initialization routines you do not need
	// Change the registry key under which our settings are stored
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization
	SetRegistryKey(_T("Local AppWizard-Generated Applications"));
	LoadStdProfileSettings(0);  // Load standard INI file options (including MRU)

#ifndef FMI_DISABLE_MFC_FEATURE_PACK
	InitContextMenuManager();

	InitKeyboardManager();

	InitTooltipManager();
	CMFCToolTipInfo ttParams;
	ttParams.m_bVislManagerTheme = TRUE;
	theApp.GetTooltipManager()->SetTooltipParams(AFX_TOOLTIP_TYPE_ALL,
		RUNTIME_CLASS(CMFCToolTipCtrl), &ttParams);
#endif // FMI_DISABLE_MFC_FEATURE_PACK

	CrashRptInstall(); // tämä on tarkoitus kutsua ennen InitGeneralDataDoc-kutsua, mutta tätä ei voinut laittaa suoraan applikaation konstruktoriin (kaatuu).

	// Initialize GDI+, pitää initialisoida ennen itsGeneralData-olion luomista!
	InitGdiplus();
    InitApplicationInterface(); // Kutsuttava ennen kuin doc-view systeemi ja generalDataDoc luodaan
    if(!InitGeneralDataDoc())
        return FALSE;

	// Register the application's document templates.  Document templates
	//  serve as the connection between documents, frame windows and views
	CSingleDocTemplate* pDocTemplate;
	pDocTemplate = new CSingleDocTemplate(
		IDR_MAINFRAME,
		RUNTIME_CLASS(CSmartMetDoc),
		RUNTIME_CLASS(CMainFrame),       // main SDI frame window
		RUNTIME_CLASS(CSmartMetView));
	if (!pDocTemplate)
		return FALSE;
	AddDocTemplate(pDocTemplate);

#ifndef FMI_DISABLE_MFC_FEATURE_PACK
	// Connect the COleTemplateServer to the document template
	//  The COleTemplateServer creates new documents on behalf
	//  of requesting OLE containers by using information
	//  specified in the document template
	m_server.ConnectTemplate(clsid, pDocTemplate, TRUE);
		// Note: SDI applications register server objects only if /Embedding
		//   or /Automation is present on the command line

	// Enable DDE Execute open
	EnableShellOpen();
	RegisterShellFileTypes(TRUE);
#endif // FMI_DISABLE_MFC_FEATURE_PACK

	// Parse command line for standard shell commands, DDE, file open
	CFmiMyCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);

	// App was launched with /Embedding or /Automation switch.
	// Run app as automation server.
	if (cmdInfo.m_bRunEmbedded || cmdInfo.m_bRunAutomated)
	{
#ifndef FMI_DISABLE_MFC_FEATURE_PACK
		// Register all OLE server factories as running.  This enables the
		//  OLE libraries to create objects from other applications
		COleTemplateServer::RegisterAll();
#endif // FMI_DISABLE_MFC_FEATURE_PACK

		// Don't show the main window
		return TRUE;
	}
#ifndef FMI_DISABLE_MFC_FEATURE_PACK
	// App was launched with /Unregserver or /Unregister switch.  Unregister
	// typelibrary.  Other unregistration occurs in ProcessShellCommand().
	else if (cmdInfo.m_nShellCommand == CCommandLineInfo::AppUnregister)
	{
		UnregisterShellFileTypes();
		m_server.UpdateRegistry(OAT_DISPATCH_OBJECT, NULL, NULL, FALSE);
		AfxOleUnregisterTypeLib(_tlid, _wVerMajor, _wVerMinor);
	}
	// App was launched standalone or with other switches (e.g. /Register
	// or /Regserver).  Update registry entries, including typelibrary.
	else
	{
		m_server.UpdateRegistry(OAT_DISPATCH_OBJECT);
		COleObjectFactory::UpdateRegistryAll();
		AfxOleRegisterTypeLib(AfxGetInstanceHandle(), _tlid);
	}
#endif // FMI_DISABLE_MFC_FEATURE_PACK

	// Dispatch commands specified on the command line.  Will return FALSE if
	// app was launched with /RegServer, /Register, /Unregserver or /Unregister.
	if (!ProcessShellCommand(cmdInfo))
		return FALSE;

    DoToolMasterInitialization(); // m_pMainWnd luodaan vasta ProcessShellCommand -metodissa, joten sen jälkeen voidaan ToolMaster initialisoida

	LoadFileAtStartUp(&cmdInfo);
//	CreateMenuDynamically();

	// The one and only window has been initialized, so show and update it
	m_pMainWnd->ShowWindow(SW_SHOW);
	m_pMainWnd->UpdateWindow();
	// call DragAcceptFiles only if there's a suffix
	//  In an SDI app, this should occur after ProcessShellCommand
	// Enable drag/drop open
	m_pMainWnd->DragAcceptFiles();
	pSplashThreadHolder.HideSplash(); // haluan sulkea splash-screenin ennen tuota viimeista initialisaatiota
	DoFinalInitializations();
    itsGeneralData->FillStartupViewMacro(); // Otetaan SmartMetista 'alkukuva' näyttömakroon kaikkien alustuksien jälkeen

	return TRUE;
}

static ApplicationInterface& GetApplicationInterface()
{
    static ApplicationInterfaceForSmartMet applicationInterfaceForSmartMet;
    return applicationInterfaceForSmartMet;
}

void CSmartMetApp::InitApplicationInterface()
{
    ApplicationInterface::GetApplicationInterfaceImplementation = [&]() {return &::GetApplicationInterface(); };
}


// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
    CAboutDlg(const std::string &theTitleStr, const std::string &theVersionStr, const std::vector<DrawStringData> &theDynamicTextsDataVector);
    ~CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()

    CRect GetActualWindowRect();

    CString	itsTitleStrU_;
    std::vector<DrawStringData> itsDynamicTextsDataVector;
    CBitmap versionBaseImage;
public:
    afx_msg void OnPaint();
};

CAboutDlg::CAboutDlg(const std::string &theTitleStr, const std::string & theVersionStr, const std::vector<DrawStringData> &theDynamicTextsDataVector)
    : CDialog(CAboutDlg::IDD)
    , itsDynamicTextsDataVector(theDynamicTextsDataVector)
{
	itsTitleStrU_ = CA2T(theTitleStr.c_str());
	EnableActiveAccessibility();
}

CAboutDlg::~CAboutDlg()
{
    versionBaseImage.DeleteObject();
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
    ON_WM_PAINT()
END_MESSAGE_MAP()

CRect CAboutDlg::GetActualWindowRect()
{
    WINDOWPLACEMENT wndpl;
    wndpl.length = sizeof(WINDOWPLACEMENT);
    // gets current window position and iconized/maximized status
    BOOL bRet = GetWindowPlacement(&wndpl);

    BOOL status = versionBaseImage.LoadBitmap(IDB_BITMAP_ABOUT_DLG_VER_5_10_NO_TEXTS);
    //Get the width and height of the DIBSection
    BITMAP bm;
    int luku = versionBaseImage.GetObject(sizeof(bm), &bm);

    return CRect(wndpl.rcNormalPosition.left, wndpl.rcNormalPosition.top, wndpl.rcNormalPosition.left + bm.bmWidth, wndpl.rcNormalPosition.top + bm.bmHeight);
}

BOOL CAboutDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
    SetWindowText(itsTitleStrU_);
	this->SetIcon(AfxGetApp()->LoadIcon(IDR_MAINFRAME), false);
    auto actualRect = GetActualWindowRect();
    MoveWindow(actualRect, FALSE);

    return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

// App command to run the dialog
void CSmartMetApp::OnAppAbout()
{
    CAboutDlg aboutDlg(::GetDictionaryString("About SmartMet"), itsGeneralData->EditorVersionStr(), gBasicSmartMetConfigurations.SplashScreenTextDataVector());
	aboutDlg.DoModal();
}

void CAboutDlg::OnPaint()
{
    CPaintDC dc(this); // device context for painting

    //Get the width and height of the DIBSection
    BITMAP bm;
    int luku = versionBaseImage.GetObject(sizeof(bm), &bm);

    //select the palette and bitmap to the DC
    CDC memDC;
    memDC.CreateCompatibleDC(&dc);
    CBitmap* pOldBitmap = memDC.SelectObject(&versionBaseImage);
    dc.BitBlt(0, 0, bm.bmWidth, bm.bmHeight, &memDC, 0, 0, SRCCOPY);

    // Piirretään dynaamiset tekstit bitmapin päälle
    ::DrawDynamicTexts(&dc, itsDynamicTextsDataVector);

    memDC.SelectObject(pOldBitmap);
    memDC.DeleteDC();

    // Do not call CDialog::OnPaint() for painting messages
}

// CSmartMetApp customization load/save methods

#ifndef FMI_DISABLE_MFC_FEATURE_PACK
void CSmartMetApp::PreLoadState()
{
	BOOL bNameValid;
    CString strNameU_;
	bNameValid = strName.LoadString(IDS_EDIT_MENU);
	ASSERT(bNameValid);
	GetContextMenuManager()->AddMenu(strName, IDR_POPUP_EDIT);
	bNameValid = strName.LoadString(IDS_EXPLORER);
	ASSERT(bNameValid);
	GetContextMenuManager()->AddMenu(strName, IDR_POPUP_EXPLORER);
}

void CSmartMetApp::LoadCustomState()
{
}

void CSmartMetApp::SaveCustomState()
{
}
#endif // FMI_DISABLE_MFC_FEATURE_PACK

// CSmartMetApp message handlers

void CSmartMetApp::DoFinalInitializations(void)
{
	((CSmartMetDoc*)((CMainFrame*)m_pMainWnd)->GetActiveDocument())->UpdateAllViewsAndDialogs(__FUNCTION__);

	// tehdään automaattinen datan lataus, jos niin on haluttu
	if(itsGeneralData->DoAutoLoadDataAtStartUp())
	{
		itsGeneralData->EditedDataNeedsToBeLoaded(true);

		((CSmartMetDoc*)((CMainFrame*)m_pMainWnd)->GetActiveDocument())->LoadDataOnStartUp();
		((CMainFrame*)m_pMainWnd)->CheckForAutoLoadTimerStart();
	}
	((CMainFrame*)m_pMainWnd)->StartAdditionalWorkerThreads(); // käynnistetään muut working-theadit, vasta kun MainFrame on luotu.
}

void CSmartMetApp::DoFirstTimeOnIdleInitializations()
{
    static bool firstTime = true;
    if(firstTime)
    { // en saanut muuten auto zoomia toimimaan kun editori käynnistetään kuin laittamalla tanne refresh komento ruudulle kun 1. ollaan onidlessa.
        firstTime = false;

        // Tämä pitää tehdä 1. kerran vasta kun kaikki FrameWorkin initialisoinnit on tehty
        ((CSmartMetDoc*)((CMainFrame*)m_pMainWnd)->GetActiveDocument())->SetAllViewIconsDynamically();

        // Static help data must be read once after MainFrame has created (is related with AddParamSystem updating intriques)
        itsGeneralData->LoadStaticHelpData();

        if(itsGeneralData->ApplicationWinRegistry().KeepMapAspectRatio())
        {
            itsGeneralData->GetCombinedMapHandler()->mapViewDirty(CtrlViewUtils::kDoAllMapViewDescTopIndex, true, true, true, false, false, false);
            CSmartMetView *mapView = ApplicationInterface::GetSmartMetView();
            if(mapView)
                mapView->GetDocument()->UpdateAllViewsAndDialogs(__FUNCTION__);
        }
    }
}

BOOL CSmartMetApp::OnIdle(LONG lCount)
{
	BOOL status = CWinApp::OnIdle(lCount);

	if(itsGeneralData)
	{
        DoFirstTimeOnIdleInitializations();

        if(itsGeneralData->DoImageCacheUpdates() == 1)
            return 1; // jos olis imageCache ruudunpäivityksiä, lopetetaan onIdle siihen ja palautetaan 1

		// palauttaa 0 jos ei animaatiota ja 1 jo tehdään animaatiota
		return itsGeneralData->DoAllAnimations();
	}
	return 0; // onidle käsittely loppuu 0:lla
}

NFmiEditMapGeneralDataDoc* CSmartMetApp::GeneralDocData()
{
	return itsGeneralData;
}
/*
void CSmartMetApp::CreateMenuDynamically(void)
{
	itsMenuCreator.CreateMenuDynamically(AfxGetMainWnd());
}
*/

void CSmartMetApp::CloseToolMaster()
{
    if(itsGeneralData && itsGeneralData->IsToolMasterAvailable())
    {
        Toolmaster::CloseToolMaster();
    }
}

int CSmartMetApp::ExitInstance()
{
    // Tätä pitää kutsua ennen kuin itsGeneralData tuhotaan!
    CloseToolMaster();

	// HUOM! itsGeneralData pitää tuhota ennen kuin GDI+ systeemi lopetetaan, koska
	// generaldatalla on Gdiplus-bitmappeja tallessa, jotka gdishutdown näyttää tuhoavan.
	// Eli ne tuhotaan tässä ensin generaldatan destruktorissa, muuten ne yritettäisiin
	// tuhota listoista kun ne ovat ei 0-pointtereita, jolloin ohjelma kaatuisi.
	delete itsGeneralData;

	TermGdiplus();
#ifndef FMI_DISABLE_MFC_FEATURE_PACK
	ControlBarCleanUp();
#endif // FMI_DISABLE_MFC_FEATURE_PACK

	return CWinApp::ExitInstance();
}

static void AddPossibleButtonTooltip(CSmartMetApp::ControlIdTextMap &tooltipItems, UINT commandID, const std::string &settingStr)
{
    std::string commandStr = NFmiSettings::Optional<std::string>(settingStr, "");
    if(!commandStr.empty())
        tooltipItems.insert(CSmartMetApp::ControlIdTextMap::value_type(commandID, commandStr));
}

// Ajetaan kun GeneralDataDoc on initialisoitu (eli kun konfiguraatiot on alustettu).
void CSmartMetApp::InitNonDictionaryToolbarItems()
{
    try
    {
        // Toolbarissa olevat kartta nappien tooltip tekstit pitää ottaa konfiguraatioista.
        // Täällä täytetään id-text map, josta kyseiset tekstit saa vastaavaa button id:tä vastaan.
        // Pääkarttanäytön toolbar button id:t
        ::AddPossibleButtonTooltip(itsNonDictionaryToolbarItems, ID_BUTTON_SELECT_FINLAND_MAP, "MetEditor::ButtonTooltip::Map1");
        ::AddPossibleButtonTooltip(itsNonDictionaryToolbarItems, ID_BUTTON_SELECT_SCANDINAVIA_MAP, "MetEditor::ButtonTooltip::Map2");
        ::AddPossibleButtonTooltip(itsNonDictionaryToolbarItems, ID_BUTTON_SELECT_EUROPE_MAP, "MetEditor::ButtonTooltip::Map3");
        ::AddPossibleButtonTooltip(itsNonDictionaryToolbarItems, ID_BUTTON_GLOBE, "MetEditor::ButtonTooltip::Map4");
        // Apukarttanäyttöjen toolbar button id:t laitetaan lukuina, koska en halua includata SmartMetToolboxDep\SmartMetToolboxDep_resource.h -tiedostoa
        ::AddPossibleButtonTooltip(itsNonDictionaryToolbarItems, 32816, "MetEditor::ButtonTooltip::Map1");
        ::AddPossibleButtonTooltip(itsNonDictionaryToolbarItems, 32946, "MetEditor::ButtonTooltip::Map2");
        ::AddPossibleButtonTooltip(itsNonDictionaryToolbarItems, 32817, "MetEditor::ButtonTooltip::Map3");
        ::AddPossibleButtonTooltip(itsNonDictionaryToolbarItems, 32886, "MetEditor::ButtonTooltip::Map4");
    }
    catch(std::exception& e)
    {
        std::string errorString = "Error in CSmartMetApp::InitNonDictionaryToolbarItems: ";
        errorString += e.what();
        CatLog::logMessage(errorString, CatLog::Severity::Error, CatLog::Category::Operational, true);
    }
}

// Tämän funktion avulla saadaan status ja tooltip stringeistä
// oikea kieliversio stringi ulos.
CString CSmartMetApp::GetToolTipString(UINT commandID, CString &theMagickWordU_)
{
    CString strU_;
    std::string tmpStdStr;

    auto iter = itsNonDictionaryToolbarItems.find(commandID);
    if(iter != itsNonDictionaryToolbarItems.end())
        tmpStdStr = iter->second;
	else // muuten haetaan tooltip teksti normaalisti sana kirjasta
        tmpStdStr = CT2A(theMagickWordU_);

    strU_ = CA2T(::GetDictionaryString(tmpStdStr.c_str()).c_str());
    return strU_;
}

CSplashThread* CSmartMetApp::SplashStart(void)
{
	//Bring up the splash screen in a secondary UI thread
	CSplashThread* pSplashThread = (CSplashThread*) AfxBeginThread(RUNTIME_CLASS(CSplashThread), THREAD_PRIORITY_NORMAL, 0, CREATE_SUSPENDED);
	if (pSplashThread == NULL)
	{
		AfxMessageBox(_T("Failed to create splash screen"), MB_OK | MB_ICONSTOP);
		return 0;
	}
	ASSERT(pSplashThread->IsKindOf(RUNTIME_CLASS(CSplashThread)));

	//Tell the splash screen to load the bitmap from a resource called IDB_SPLASH.
	//Alternatively, you could use the line:
	//  pSplashThread->SetBitmapToUse(GetHomeDirectory() + _T("TEST.BMP"));
	//if you wanted the splasher code to load a bitmap from file at runtime.
	pSplashThread->SetBitmapToUse(IDB_BITMAP_ABOUT_DLG_VER_5_10_NO_TEXTS);
    pSplashThread->SetSplashScreenTextDataVector(gBasicSmartMetConfigurations.SplashScreenTextDataVector());

	pSplashThread->ResumeThread();  //Resume the thread now that we have set it up
	return pSplashThread;
}

void CSmartMetApp::LoadFileAtStartUp(CCommandLineInfo *theCmdInfo)
{
	if(theCmdInfo && theCmdInfo->m_nShellCommand == CCommandLineInfo::FileOpen) // jos on tiputettu editor-ikonin päälle tai annettu komento rivilla tiedoston nimi, mikä pitää avata
	{
		std::string fileName = CT2A(theCmdInfo->m_strFileName);
		try
		{
            std::unique_ptr<NFmiQueryData> data = QueryDataReading::ReadDataFromFile(fileName, true);
            bool dataWasDeleted = false;
            itsGeneralData->AddQueryData(data.release(), fileName, "", NFmiInfoData::kEditable, "", true, dataWasDeleted);
            ((CSmartMetDoc*)((CMainFrame*)m_pMainWnd)->GetActiveDocument())->SetPathName(CA2T(fileName.c_str())); // pitää laittaa AddData:n jälkeen, muuten tiedoston nimi ei tule oikein ohjelman otsikkoon.
        }
		catch(...)
		{ // tähän voisi tehdä varoituksia ja muita virityksiä
		}
	}
}

// tämä pitää tehdä vasta ProcessShellCommand:in jälkeen, koska siellä syntyy View
// AVS esimerkeissä luotiin MainFrame ennen ProcessShellCommand:ia, ei syntynyt
// uutta MainFramea ilmeisesti koska oli MDI systeemi
bool CSmartMetApp::DoToolMasterInitialization(void)
{
    if(Toolmaster::DoToolMasterInitialization(m_pMainWnd, fUseToolMasterIfAvailable))
    {
        itsGeneralData->ToolMasterAvailable(true);
    }
    itsGeneralData->InitGriddingProperties();
    return itsGeneralData->IsToolMasterAvailable();
}

#pragma warning( push )
#pragma warning( disable : 4996 )

// Unicode moodissa käytetään tätä muuttamaan 
// wchar_t ** __wargv -> char** argv
// Huolehtii myös luodun C-string taulukon tuhoamisesta.
// Jos ei Unicode-moodissa, ei luoda taulukkoa, vaan käytetään originaalia.
class UnicodeCommandLineConverter
{
public:
    UnicodeCommandLineConverter(int argc, TCHAR ** targv)
    :argc_(argc)
    ,targv_(targv)
    ,argv_(0)
    {
#ifdef _UNICODE
        if(argc_)
        {
            argv_ = new char*[argc_];
            for(int i = 0; i < argc_; i++)
            {
                size_t length = std::wcslen(targv_[i]);
                argv_[i] = new char[length + 1];
                strncpy(argv_[i], CT2A(targv_[i]), length);
                argv_[i][length] = '\0';
            }
        }
#endif
    }
    ~UnicodeCommandLineConverter()
    {
        if(argc_ && argv_)
        {
            for(int i = 0; i < argc_; i++)
            {
                delete[] argv_[i];
            }
            delete []argv_;
        }
    }

    char** getargv()
    {
#ifdef _UNICODE
        return argv_;
#else
        return targv_;
#endif
    }

private:
    int argc_;
    TCHAR ** targv_; // ei omista, ei tuhoa
    char** argv_; // omistaa, tuhoaa
};

// Undo the above warning disabling.
#pragma warning( pop )

// Ottaa ohjaus polun, -p argumentti, jossa joko hakemisto, jossa on smartmet.conf tiedosto tai 
// suoraan tiedosto, jota käytetään 'smartmet.conf':ina.
// Ohjelman otsikko teksti -t argumenttina.
// d optiolla säädetään käynnistystä Visual C++:sta (erilainen working path)
// n optiolla voidaan sanoa että ei käytetä ToolMasteria ollenkaan.
// s optiolla voidaan antaa extra asetuksia listassa (NFmiSettings asetuksia)
// v optiolla voidaan laittaa enemmän lokitusta päälle (ainakin initialisoinnissa)
// c optiolla disabloidaan CrashReporter systeemin käyttö kokonaan.
//
// return true jos jatketaan, false, jos halutaan lopettaa ohjelman suoritus.
bool CSmartMetApp::ParseCommandLineArguments()
{
    UnicodeCommandLineConverter cmdLineConverter(__argc, __targv);
    NFmiCmdLine cmd(__argc, const_cast<const char**>(cmdLineConverter.getargv()), "p!t!dns!vcb!");
	// ei tarkisteta onko comento rivi muuten oikeellinen, koska muut optiot otetaan muualta

	if(cmd.isOption('d')) // HUOM!!! tämän pitää olla ennen -p optio tarkastelua
	{
		gBasicSmartMetConfigurations.DeveloperModePath(true);
	}

	if(cmd.isOption('p'))
	{
		std::string controlPath = cmd.OptionValue('p');
        if(!gBasicSmartMetConfigurations.SetControlPath(controlPath))
            return false;
	}
    else
    {
        if(!gBasicSmartMetConfigurations.SetControlPath("control_scand_edit_local_conf"))
            return false;
    }

	if(cmd.isOption('t'))
	{
		std::string frameTitle = cmd.OptionValue('t');
		gBasicSmartMetConfigurations.SetApplicationTitle(frameTitle);
	}
	else
		gBasicSmartMetConfigurations.SetApplicationTitle("SmartMet");

	if(cmd.isOption('n'))
		fUseToolMasterIfAvailable = false;

	if(cmd.isOption('s'))
	{
		std::string extraSettingsStr = cmd.OptionValue('s');
		std::vector<std::string> settingsVector = NFmiStringTools::Split(extraSettingsStr, ";");
		for(int i = 0; i < settingsVector.size(); i++)
		{
			std::vector<std::string> keyValuePair = NFmiStringTools::Split(settingsVector[i], "=");
			if(keyValuePair.size() == 2)
			{
				NFmiSettings::Set(keyValuePair[0], keyValuePair[1], false); // false tarkoittaa että lisätään uusia asetuksia
			}
		}
	}

	if(cmd.isOption('v'))
	{
        gBasicSmartMetConfigurations.Verbose(true);
	}
    if(cmd.isOption('c'))
    {
        gBasicSmartMetConfigurations.EnableCrashReporter(false);
    }
    if(cmd.isOption('b'))
    {
        std::string betaAutomationListPath = cmd.OptionValue('b');
        gBasicSmartMetConfigurations.BetaAutomationListPath(betaAutomationListPath);
    }

    return true;
}

bool CSmartMetApp::InitGdiplus()
{
	Gdiplus::GdiplusStartupInput gdiplusStartupInput;
	Status status = GdiplusStartup(&m_gdiplusToken,
                                  &gdiplusStartupInput,
                                  NULL);
	return (Gdiplus::Ok == status);
}
void CSmartMetApp::TermGdiplus()
{
   GdiplusShutdown(m_gdiplusToken);
}

bool CSmartMetApp::InitGeneralDataDoc(void)
{
    // UGLY but I couldn't put this in mainframe
	itsGeneralData = new NFmiEditMapGeneralDataDoc(WM_USER + 100); 

    try
    {
        itsGeneralData->Init(gBasicSmartMetConfigurations, CSmartMetDoc::MakeMapWindowPosMap(), CSmartMetDoc::MakeOtherWindowPosMap());
    }
    catch(AbortSmartMetInitializationGracefullyException &)
    {
        return false;
    }

	itsGeneralData->InitGenDocDataAdapter(itsGeneralData); // this is really 'masterpiece', new GenDocDataAdapter must be initialized with generalDoc-pointer. This 
															// GenDocDataAdapter will be used with the ne and improved data modification engine in ModifyEditedData-library.
    InitNonDictionaryToolbarItems();
    return true;
}

void CSmartMetApp::UpdateCrashRptLogFile()
{
#ifdef DO_CRASH_REPORTING
    // Add our log file to the error report
    if(gBasicSmartMetConfigurations.EnableCrashReporter())
    {
        ::crAddFile2(CA2T(CatLog::currentLogFilePath().c_str()), NULL, _TEXT("Log File"), CR_AF_MAKE_FILE_COPY);
        ::crAddFile2(CA2T(CFmiCombineDataThread::MakeDailyDataCombinationLogFilePath(false).c_str()), NULL, _TEXT("Data-combining Log File"), CR_AF_MAKE_FILE_COPY);
        ::crAddFile2(CA2T(CFmiQueryDataCacheLoaderThread::MakeDailyUnpackLogFilePath().c_str()), NULL, _TEXT("Unpack-data Log File"), CR_AF_MAKE_FILE_COPY);
    }
#endif
}

#include <rtcapi.h>

int exception_handler(LPEXCEPTION_POINTERS p)
{
    CatLog::logMessage("SmartMet has detected unknown exception while crash reporting system was disabled and now SmartMet just shuts down", CatLog::Severity::Error, CatLog::Category::Operational, true);
    exit(1);
}
int runtime_check_handler(int errorType, const char *filename, int linenumber, const char *moduleName, const char *format, ...)
{
    auto errorString = boost::format("type %1% at %2% line %3% in %4%") % errorType % filename % linenumber % moduleName;
    CatLog::logMessage("SmartMet has detected error: " + errorString.str() + ", while crash reporting system was disabled and now SmartMet just shuts down", CatLog::Severity::Error, CatLog::Category::Operational, true);
    exit(1);
}

// Jos CrashReporter on disabloitu smartMetissa -c optiolla,
// Nämä estävät Windowsin WER (Windows Error Reporting) dialogien käynnistymisen kaatumisen yhteydessä.
// Näitä ei saa pois päältä millään win-registry asetuksilla, varsinkaan jos koneeseen on asennettu
// Visual C++ kääntäjä, jolloin systeemi haluaa tarkistaa vielä käyttäjältä että haluaako hän debugata ohjelman kaatoa.
void disableWindowsWerAndDebugDialogs()
{
    DWORD dwMode = SetErrorMode(SEM_NOGPFAULTERRORBOX);
    SetErrorMode(dwMode | SEM_NOGPFAULTERRORBOX);
    SetUnhandledExceptionFilter((LPTOP_LEVEL_EXCEPTION_FILTER)&exception_handler);
    _RTC_SetErrorFunc(&runtime_check_handler);

    CatLog::logMessage("SmartMet has disabled the crash reporting system (-c command line argument used) and all the Windows Report Error dialogs", CatLog::Severity::Info, CatLog::Category::Configuration);
}

bool CSmartMetApp::CrashRptInstall(void)
{
#ifdef DO_CRASH_REPORTING
    if(gBasicSmartMetConfigurations.EnableCrashReporter())
    {
        // http lähetys on varattu lähinnä Latvialaisille, joille sähköpostin lähetys on estetty tietoturva systeemien takia
        bool useHttpTransfer = NFmiSettings::Optional<bool>("SmartMet::UseHttpTransferForCrashReports", false);

        // Install crash reporting
        CR_INSTALL_INFO info;
        memset(&info, 0, sizeof(CR_INSTALL_INFO));
        info.cb = sizeof(CR_INSTALL_INFO);
        info.pszAppName = CA2T(gBasicSmartMetConfigurations.ApplicationDataBase().appname.c_str());
        info.pszAppVersion = CA2T(gBasicSmartMetConfigurations.ApplicationDataBase().appversion.c_str());
        if(useHttpTransfer)
        {
            // URL for sending error reports over HTTP.
            info.pszUrl = _T("http://services.weatherproof.fi/editor/crashrpt.php");
            // Use binary encoding for HTTP uploads (recommended).
            info.dwFlags |= CR_INST_HTTP_BINARY_ENCODING;
            info.uPriorities[CR_HTTP] = 3;  // First try send report over HTTP 
        }
        // Nyt käytössä vain sähköpostilähetys
        info.pszEmailTo = _T("marko.pietarinen@fmi.fi");
        // Install all available exception handlers.
        info.dwFlags |= CR_INST_ALL_POSSIBLE_HANDLERS;
        // Provide privacy policy URL
        //  info.pszPrivacyPolicyURL = _T("http://fmi.smartmetserver.fi/privacy.html");
        info.uPriorities[CR_SMTP] = 2;  // Second try send report over SMTP  
        info.uPriorities[CR_SMAPI] = 1; // Third try send report over Simple MAPI    

        int nResult = crInstall(&info);
        if(nResult != 0)
        {
            TCHAR buff[256];
            ::crGetLastErrorMsg(buff, 256);
            MessageBox(NULL, buff, _TEXT("CrashRpt Install error"), MB_OK);
            return false;
        }

        UpdateCrashRptLogFile();

        // Take screenshot of the app window at the moment of crash
        ::crAddScreenshot(CR_AS_VIRTUAL_SCREEN);

        CatLog::logMessage("SmartMet has enabled the crash reporting system", CatLog::Severity::Debug, CatLog::Category::Configuration);
    }
    else
    {
        disableWindowsWerAndDebugDialogs();
    }
#endif // DO_CRASH_REPORTING
    return true;
}

void CSmartMetApp::CrashRptUninstall(void)
{
#ifdef DO_CRASH_REPORTING
  // Uninstall crash reporting
    if(gBasicSmartMetConfigurations.EnableCrashReporter())
        ::crUninstall();
#endif // DO_CRASH_REPORTING
}

static void DoErrorReporting(const std::string &messageStart, const std::string &functionName, const std::string &messageEnd)
{
    std::string errorString = messageStart;
    errorString += " ";
    errorString += functionName;
    if(!messageEnd.empty())
    {
        errorString += ": ";
        errorString += messageEnd;
    }
    CatLog::logMessage(errorString, CatLog::Severity::Error, CatLog::Category::Operational, true);
}

// This make sure that certain default locales, in win32 environment, are set to english.
// This fixes used symbol bug, when using special fonts (like Synop and Mirri), are shown correctly in
// PCs that use certain languages (like russian). Problem lies somewhere in used default language
// and using string conversion macros (CA2T, etc.) that can do strange conversions when used character's
// value is between 128 and 255. ENGLISH_US sets latin-1 as default codepage that is needed for special fonts here...
void CSmartMetApp::SetWin32LocalesToEnglish()
{
    SetThreadLocale(MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), SORT_DEFAULT));
}

int CSmartMetApp::Run()
{
    SetWin32LocalesToEnglish();

    int status = 0;
    do
    {
        try
        {
            status = CFmiUsedAppParent::Run();
        }
        catch(std::exception &e)
        {
            ::DoErrorReporting("Exception was catched in", __FUNCTION__, e.what());
        }
        catch(...)
        {
            ::DoErrorReporting("Unknown Exception was catched in", __FUNCTION__, "");
        }

    } while(!AllowApplicationToClose());
    return status;
}
