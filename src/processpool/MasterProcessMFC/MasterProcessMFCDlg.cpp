
// MasterProcessMFCDlg.cpp : implementation file
//

#include "stdafx.h"
#include "MasterProcessMFC.h"
#include "MasterProcessMFCDlg.h"
#include "MfcTools.h"
#include "afxdialogex.h"
#include "logging.h"
#include "work_queue.h"
#include "process_helpers.h"

#include <boost/thread/thread.hpp> 
#include <boost/log/sinks/basic_sink_backend.hpp>
#include <boost/log/expressions.hpp>
#include "boost/format.hpp"
#include <boost/log/sinks/text_file_backend.hpp>
#include <boost/log/sinks/text_ostream_backend.hpp>
#include <boost/log/sinks/sync_frontend.hpp>

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


// CMasterProcessMFCDlg dialog



CMasterProcessMFCDlg::CMasterProcessMFCDlg(const MasterProcessMFCOptions &options, CWnd* pParent /*=NULL*/)
	: CDialogEx(CMasterProcessMFCDlg::IDD, pParent)
    , mDoOnClose(false)
    , mOptions(options)
    , m_visible(true)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CMasterProcessMFCDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_LIST_LOG_VIEW, mLogListView);
}

BEGIN_MESSAGE_MAP(CMasterProcessMFCDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
    ON_WM_SIZE()
    ON_WM_CLOSE()
    ON_WM_TIMER()
    ON_WM_CHAR()
    ON_WM_WINDOWPOSCHANGING()
END_MESSAGE_MAP()


// CMasterProcessMFCDlg message handlers

BOOL CMasterProcessMFCDlg::OnInitDialog()
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

void CMasterProcessMFCDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CMasterProcessMFCDlg::OnPaint()
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
HCURSOR CMasterProcessMFCDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CMasterProcessMFCDlg::OnSize(UINT nType, int cx, int cy)
{
    CDialogEx::OnSize(nType, cx, cy);

    FillClientAreaWithLogView();
}

void CMasterProcessMFCDlg::FillClientAreaWithLogView(void)
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


void CMasterProcessMFCDlg::OnClose()
{
    mDoOnClose = true; // CDialogEx::OnClose kutsuu viel‰ OnCancel:ia, joten laitetaan lippu p‰‰lle, jotta nyt OnCancel:issa voidaan lopettaa
    DoWhenClosing();
    CDialogEx::OnClose();
}


void CMasterProcessMFCDlg::OnOK()
{
    // Ei suljeta masteria Return -n‰pp‰imen painallukseen
//    DoWhenClosing();
//    CDialogEx::OnOK();
}


void CMasterProcessMFCDlg::OnCancel()
{
    // Ei suljeta masteria Esc -n‰pp‰imen painallukseen, mutta jos t‰t‰ kutsutaan OnClose:sta (mDoOnClose = true), jatketaan lopetukseen
//    DoWhenClosing();
    if(mDoOnClose)
        CDialogEx::OnCancel();
}

void CMasterProcessMFCDlg::DoWhenClosing(void)
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

void CMasterProcessMFCDlg::init_listview_logging()
{
    gLogListCounter = 0;

    boost::shared_ptr< logging::core > core = logging::core::get();

    typedef log_sinks::synchronous_sink< ListViewLogger > sink_t;
    boost::shared_ptr< sink_t > sink(new sink_t());

    core->add_sink(sink);
}

typedef process_helpers::master_process_stuff<task_queue_t, result_queue_t> master_process_stuff_t;
static size_t global_task_index = 0;
const std::string worker_base_name = "worker";
const std::string worker_project_name = "WorkerProcessMFC";
const std::string worker_executable_name = worker_project_name + ".exe";

int check_if_key_pressed()
{
    int ch = gLastKeyPressed;
    gLastKeyPressed = 0;
    return ch;
}

void master_loop_function(const MasterProcessMFCOptions &options)
{
    if(gProgramIsClosing)
        return ;
	CSingleLock singleLock(&gMasterLoopThreadRunning);
	if(!singleLock.Lock(0))
        return ; // t‰h‰n ei pit‰isi menn‰, ellei p‰‰ohjelma ole jo lopettamassa

    int worker_count = static_cast<int>(boost::thread::hardware_concurrency());

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

        process_helpers::multi_process_pool_options mpp_options;
        mpp_options.task_queue_size_in_bytes = options.work_queue_size_in_bytes;
        mpp_options.result_queue_size_in_bytes = options.result_queue_size_in_bytes;
        mpp_options.verbose_logging = options.verbose_log_;
        mpp_options.worker_executable_absolute_path = options.worker_exe_file_path_;

        base_worker_process_start_info base_process_info(worker_executable_name, worker_project_name, worker_base_name, extra_args);
//        master_process_stuff_t master_process_data(worker_count, global_task_index, base_process_info, options.demo_mode_);
        master_process_stuff_t master_process_data(worker_count, base_process_info, mpp_options);
        if(options.demo_mode_)
            fill_work_queue2(master_process_data);
        process_helpers::master_loop(master_process_data, check_if_key_pressed, gProgramIsClosing, options.demo_mode_);
    }
    catch(boost::interprocess::interprocess_exception &ex)
    {
        log_message(ex.what(), logging::trivial::error);
        MessageBox(AfxGetMainWnd()->GetSafeHwnd(), CA2T(ex.what()), _TEXT("Error starting master-loop"), MB_ICONERROR | MB_OK);
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

void CMasterProcessMFCDlg::StartMasterLoopThread(void)
{
    init_logger("MasterMFC", static_cast<logging::trivial::severity_level>(mOptions.log_level_), mOptions.log_file_path_);
    work_queue_verbose_logging(mOptions.verbose_log_); // laitetaan myˆs work_queue:n verbose-log tila p‰‰lle
    init_listview_logging();

    std::string log_string("Started (ver.");
    log_string += GetFileVersionOfApplication(CA2T(mOptions.exe_path_.c_str()));
    log_string += ") with cmd args: ";
    log_string += mOptions.cmd_args_;
    log_message(log_string, logging::trivial::info);

    log_message("Starting master-loop-thread", logging::trivial::info);
    gMasterThread = boost::thread(master_loop_function, boost::ref(mOptions));
}

void CMasterProcessMFCDlg::log_on_list_view()
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

void CMasterProcessMFCDlg::OnTimer(UINT_PTR nIDEvent)
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
BOOL CMasterProcessMFCDlg::PreTranslateMessage(MSG* pMsg)
{
    if(pMsg->message==WM_KEYDOWN)
    {
        if(pMsg->wParam == 'q' || pMsg->wParam == 'Q' || pMsg->wParam == 'e' || pMsg->wParam == 'E' || (pMsg->wParam >= '0' && pMsg->wParam <= '9'))
            gLastKeyPressed = static_cast<UINT>(pMsg->wParam);
    }	
    return CDialogEx::PreTranslateMessage(pMsg);
}


void CMasterProcessMFCDlg::OnWindowPosChanging(WINDOWPOS* lpwndpos)
{
    if (!m_visible)
        lpwndpos->flags &= ~SWP_SHOWWINDOW;

    CDialogEx::OnWindowPosChanging(lpwndpos);
}
