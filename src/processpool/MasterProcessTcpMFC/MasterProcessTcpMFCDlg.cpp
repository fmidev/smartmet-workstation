
// MasterProcessTcpMFCDlg.cpp : implementation file
//

#include "stdafx.h"
#include "MasterProcessTcpMFC.h"
#include "MasterProcessTcpMFCDlg.h"
#include "afxdialogex.h"
#include "logging.h"
#include "tcp_tools.h"
#include "MfcTools.h"
#include "master_to_worker_connection.h"
#include "process_helpers.h"
#include "NFmiQueryDataUtil.h"

#include <boost/thread/thread.hpp> 
#include <boost/log/sinks/basic_sink_backend.hpp>
#include <boost/log/sinks/sync_frontend.hpp>
#include <boost/lexical_cast.hpp>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace
{
	CSemaphore gMasterLoopThreadRunning; // t‰m‰n avulla yritet‰‰n lopettaan jatkuvasti pyˆriv‰ master thread 'siististi'
    bool gProgramIsClosing = false;
    boost::thread gMasterThread;

    std::vector<std::string> gLogStringList;
    int gLogListCounter;
	CSemaphore gLoggingSemaphore; // t‰m‰n avulla voidaan lokittaa eri s‰ikeist‰ synkronoidusti

    UINT gLastKeyPressed;
}

namespace log_sinks = logging::sinks;

// The backend starts an external application to display notifications
class ListViewLogger : public log_sinks::basic_formatted_sink_backend<char, log_sinks::synchronized_feeding>
{
public:

    // The function consumes the log records that come from the frontend
    void consume(logging::record_view const& /* rec */ , string_type const& command_line)
    {
        CSingleLock singleLock(&gLoggingSemaphore);
        if(singleLock.Lock(50)) // yritt‰‰ lukita 50 ms ajan, jos ei onnistu, hyl‰t‰‰n lokiviesti.
        {
            gLogStringList.push_back(command_line); // laitetaan vasta listaan, varsinainen lokitus tapahtuu myˆhemmin timerilla Master-dialogissa
        }
    }

};

// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CMasterProcessTcpMFCDlg dialog



CMasterProcessTcpMFCDlg::CMasterProcessTcpMFCDlg(const MasterProcessMFCOptions &options, CWnd* pParent /*=NULL*/)
	: CDialogEx(CMasterProcessTcpMFCDlg::IDD, pParent)
    , mDoOnClose(false)
    , mOptions(options)
    , m_visible(true)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CMasterProcessTcpMFCDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_LIST_LOG_VIEW, mLogListView);
}

BEGIN_MESSAGE_MAP(CMasterProcessTcpMFCDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
    ON_WM_SIZE()
    ON_WM_CLOSE()
    ON_WM_TIMER()
    ON_WM_CHAR()
    ON_WM_WINDOWPOSCHANGING()
END_MESSAGE_MAP()


// CMasterProcessTcpMFCDlg message handlers

BOOL CMasterProcessTcpMFCDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
    FillClientAreaWithLogView();
	itsLoggingTimer = static_cast<UINT>(SetTimer(kFmiLoggingTimer, 50, NULL)); // tarkistetaan 20 kertaa sekunnissa ett‰ pit‰‰kˆ lokittaa jotain, koska lokiviesti‰ tulee eri s‰ikeist‰, pit‰‰ kikkailla
    StartMasterLoopThread();
	itsCheckClosingTimer = static_cast<UINT>(SetTimer(kFmiCheckClosingTimer, 100, NULL)); // tarkistetaan 10 kertaa sekunnissa ett‰ pit‰‰kˆ ohjelma sulkea, k‰sky tulee threadista, joten se pit‰‰ tehd‰ n‰in timerin kautta

    if(mOptions.hide_)
        //    ShowWindow(SW_HIDE); // Ei toimi n‰in
        m_visible = false;

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CMasterProcessTcpMFCDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CMasterProcessTcpMFCDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CMasterProcessTcpMFCDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CMasterProcessTcpMFCDlg::OnSize(UINT nType, int cx, int cy)
{
    CDialogEx::OnSize(nType, cx, cy);

    FillClientAreaWithLogView();
}

void CMasterProcessTcpMFCDlg::FillClientAreaWithLogView(void)
{
    static bool firstTime = true;
    if(firstTime)
        firstTime = false;
    else
    {
        CRect clientRect;
        GetClientRect(clientRect);
        mLogListView.MoveWindow(clientRect);
    }
}


void CMasterProcessTcpMFCDlg::OnClose()
{
    mDoOnClose = true; // CDialogEx::OnClose kutsuu viel‰ OnCancel:ia, joten laitetaan lippu p‰‰lle, jotta nyt OnCancel:issa voidaan lopettaa
    DoWhenClosing();
    CDialogEx::OnClose();
}


void CMasterProcessTcpMFCDlg::OnOK()
{
    // Ei suljeta masteria Return -n‰pp‰imen painallukseen
//    DoWhenClosing();
//    CDialogEx::OnOK();
}


void CMasterProcessTcpMFCDlg::OnCancel()
{
    // Ei suljeta masteria Esc -n‰pp‰imen painallukseen, mutta jos t‰t‰ kutsutaan OnClose:sta (mDoOnClose = true), jatketaan lopetukseen
//    DoWhenClosing();
    if(mDoOnClose)
        CDialogEx::OnCancel();
}

void CMasterProcessTcpMFCDlg::DoWhenClosing(void)
{
    // 1. laita master-loopin lopetus lippu p‰‰lle
    gProgramIsClosing = true;
    // 2. odota tietty aika hallittua lopetusta
    log_message("Waiting master-thread to stop", logging::trivial::info);
	CSingleLock singleLock(&gMasterLoopThreadRunning);
	if(singleLock.Lock(5000))
    {
        log_message("Master-thread gracefully stopped", logging::trivial::info);
        return ;
    }
    log_message("Master-thread was not gracefully stopped", logging::trivial::error);

}

void CMasterProcessTcpMFCDlg::init_listview_logging()
{
    gLogListCounter = 0;

    boost::shared_ptr< logging::core > core = logging::core::get();

    typedef log_sinks::synchronous_sink< ListViewLogger > sink_t;
    boost::shared_ptr< sink_t > sink(new sink_t());

    core->add_sink(sink);
}

static size_t global_task_index = 0;
const std::string master_name = "MasterTcpMFC";
const std::string worker_base_name = "workerTcpMFC";
const std::string worker_project_name = "WorkerProcessTcpMFC";
const std::string worker_executable_name = worker_project_name + ".exe";

int check_if_key_pressed()
{
    int ch = gLastKeyPressed;
    gLastKeyPressed = 0;
    return ch;
}

static void generate_starting_test_tasks(void)
{
    master_to_worker_connection::set_last_working_action_time();
    size_t running_task_index = 1;
    tcp_tools::fill_work_queue(master_to_worker_connection::work_queue(), running_task_index);
    log_message("Writing generated workqueue", logging::trivial::info);
    tcp_tools::log_concurrent_queue(master_to_worker_connection::work_queue(), logging::trivial::info);
}

void master_loop_function(const MasterProcessMFCOptions &options)
{
    if(gProgramIsClosing)
        return ;
	CSingleLock singleLock(&gMasterLoopThreadRunning);
	if(!singleLock.Lock(0))
        return ; // t‰h‰n ei pit‰isi menn‰, ellei p‰‰ohjelma ole jo lopettamassa

    try
    {
        std::vector<std::string> extra_args;
        extra_args.push_back("-H"); // t‰m‰ piilottaa MFC-workerit (DOS-workerit ovat piilossa defaulttina (en tied‰ miksi))
        extra_args.push_back("-l");
        extra_args.push_back(options.log_file_path_);
        extra_args.push_back("-L");
        extra_args.push_back(boost::lexical_cast<std::string>(options.log_level_));
        if(options.verbose_log_)
            extra_args.push_back("-v");

        // Anna parametriksi joku ei "g_missing_random_seed" vakio luku, jos haluat vakio 'random' lukuja.
        tcp_tools::seed_random_generator(tcp_tools::g_missing_random_seed);

        if(options.demo_mode_)
            ::generate_starting_test_tasks();

        master_to_worker_connection::initialize_master(options.verbose_log_);
        master_to_worker_connection::set_keyboard_check_function(::check_if_key_pressed);

        int worker_count = static_cast<int>(NFmiQueryDataUtil::GetReasonableWorkingThreadCount(75.));
        master_to_worker_connection::startup_workers(worker_count, worker_base_name, options.worker_exe_file_path_, worker_executable_name, worker_project_name, extra_args, true);

        master_to_worker_connection::master_service().run();
    }
    catch(std::exception &ex)
    {
        log_message(ex.what(), logging::trivial::error);
        MessageBox(AfxGetMainWnd()->GetSafeHwnd(), CA2T(ex.what()), _TEXT("Error starting master-loop"), MB_ICONERROR | MB_OK);
    }
    catch(...)
    {
        log_message("Unknown exception thrown", logging::trivial::error);
        MessageBox(AfxGetMainWnd()->GetSafeHwnd(), _TEXT("Unknown error"), _TEXT("Error starting master-loop"), MB_ICONERROR | MB_OK);
    }

    log_message("Stopping", logging::trivial::info);
    gProgramIsClosing = true;
}

void CMasterProcessTcpMFCDlg::StartMasterLoopThread(void)
{
    init_logger(master_name, static_cast<logging::trivial::severity_level>(mOptions.log_level_), mOptions.log_file_path_);
    init_listview_logging();

    std::string log_string("Started (ver.");
    log_string += GetFileVersionOfApplication(CA2T(mOptions.exe_path_.c_str()));
    log_string += ") with cmd args: ";
    log_string += mOptions.cmd_args_;
    log_message(log_string, logging::trivial::info);

    log_message("Starting master-loop-thread", logging::trivial::info);
    gMasterThread = boost::thread(master_loop_function, boost::ref(mOptions));
}

void CMasterProcessTcpMFCDlg::log_on_list_view()
{
    if(!gProgramIsClosing) // jos ohjelma on odottamassa sulkeutumista, ei voi lokittaa, koska se lukitsee systeemin, lis‰ksi viestej‰ ei ehtisi n‰hd‰ muutenkaan
    {
        CSingleLock singlelock(&gLoggingSemaphore);
        if(singlelock.Lock(30))
        {
            for(size_t i= 0; i < gLogStringList.size(); i++)
            {
                mLogListView.InsertItem(gLogListCounter++, CA2T(gLogStringList[i].c_str()));
            }
            gLogStringList.clear();
        }
    }
}

void CMasterProcessTcpMFCDlg::OnTimer(UINT_PTR nIDEvent)
{
	switch(nIDEvent)
	{
		case kFmiLoggingTimer:
		{
            log_on_list_view();
			return;
		}
		case kFmiCheckClosingTimer:
		{
            if(gProgramIsClosing)
                PostQuitMessage(0);
			return;
		}
    }

    CDialogEx::OnTimer(nIDEvent);
}


// HUOM! WM_CHAR -messagen k‰sittely ei n‰emm‰ toimi dialogi pohjaisessa 
// applikaatiossa, siksi k‰yt‰n PreTranslateMessage-funktiota.
BOOL CMasterProcessTcpMFCDlg::PreTranslateMessage(MSG* pMsg)
{
    if(pMsg->message==WM_KEYDOWN)
    {
        if(pMsg->wParam == 'q' || pMsg->wParam == 'Q' || pMsg->wParam == 'e' || pMsg->wParam == 'E' || (pMsg->wParam >= '0' && pMsg->wParam <= '9'))
            gLastKeyPressed = static_cast<UINT>(pMsg->wParam);
    }	
    return CDialogEx::PreTranslateMessage(pMsg);
}


void CMasterProcessTcpMFCDlg::OnWindowPosChanging(WINDOWPOS* lpwndpos)
{
    if (!m_visible)
        lpwndpos->flags &= ~SWP_SHOWWINDOW;

    CDialogEx::OnWindowPosChanging(lpwndpos);
}
