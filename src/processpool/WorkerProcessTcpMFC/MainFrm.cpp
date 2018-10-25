
// MainFrm.cpp : implementation of the CMainFrame class
//

#include "stdafx.h"
#include "WorkerProcessTcpMFC.h"

#include "MainFrm.h"
#include "NFmiObsDataGridding.h"
#include "MfcTools.h"

#include "logging.h"
#include "tcp_tools.h"
#include "toolmaster_tools.h"
#include "process_helpers.h"
#include "worker_to_master_connection.h"

#include "NFmiRect.h"

#include "boost/lexical_cast.hpp"
#include <boost/thread/thread.hpp> 
#include <boost/random.hpp>
#include <boost/format.hpp>
#include <boost/algorithm/string.hpp>

#include <agx\agx.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace
{
	CSemaphore gWorkerLoopThreadRunning; // tämän avulla yritetään lopettaan jatkuvasti pyörivä worker thread 'siististi'
    bool gProgramIsClosing = false;
    boost::thread gWorkerThread;
}

// CMainFrame

IMPLEMENT_DYNCREATE(CMainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
	ON_WM_CREATE()
    ON_WM_TIMER()
    ON_WM_CLOSE()
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
	ID_INDICATOR_SCRL,
};

// CMainFrame construction/destruction

CMainFrame::CMainFrame()
{
	// TODO: add member initialization code here
}

CMainFrame::~CMainFrame()
{
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	if (!m_wndToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP | CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
		!m_wndToolBar.LoadToolBar(IDR_MAINFRAME))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}

	if (!m_wndStatusBar.Create(this))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}
	m_wndStatusBar.SetIndicators(indicators, sizeof(indicators)/sizeof(UINT));

	// TODO: Delete these three lines if you don't want the toolbar to be dockable
	m_wndToolBar.EnableDocking(CBRS_ALIGN_ANY);
	EnableDocking(CBRS_ALIGN_ANY);
	DockControlBar(&m_wndToolBar);

	itsCheckClosingTimer = static_cast<UINT>(SetTimer(kFmiCheckClosingTimer, 500, NULL)); // tarkistetaan pari kertaa sekunnissa että pitääkö ohjelma sulkea, käsky tulee threadista, joten se pitää tehdä näin timerin kautta

	return 0;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CFrameWnd::PreCreateWindow(cs) )
		return FALSE;
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return TRUE;
}

// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CFrameWnd::Dump(dc);
}
#endif //_DEBUG


// CMainFrame message handlers

void CMainFrame::OnTimer(UINT_PTR nIDEvent)
{
	switch(nIDEvent)
	{
		case kFmiCheckClosingTimer:
		{
            if(gProgramIsClosing)
                PostQuitMessage(0);
			return;
		}
    }
    CFrameWnd::OnTimer(nIDEvent);
}

void CMainFrame::OnClose()
{
    if(mToolmasterInUse)
        toolmaster_tools::close();

    CFrameWnd::OnClose();
}

static NFmiRect get_relative_rect(const std::string &relative_rect_string)
{
    try
    {
        std::vector<std::string> border_strings;
        boost::split(border_strings, relative_rect_string, boost::is_any_of(","), boost::token_compress_on);
        if(border_strings.size() != 4)
            throw std::runtime_error(std::string("Given task had not correct relative area (should be in form: left,top,right,bottom), it was: ") + relative_rect_string);
        double left = boost::lexical_cast<double>(border_strings[0]);
        double top = boost::lexical_cast<double>(border_strings[1]);
        double right = boost::lexical_cast<double>(border_strings[2]);
        double bottom = boost::lexical_cast<double>(border_strings[3]);
        return NFmiRect(left, top, right, bottom);
    }
    catch(std::exception &e)
    {
        std::string error_string("Error while deducing working relative area from task:\n");
        error_string += e.what();
        log_message(error_string, logging::trivial::error);
    }
    catch(...)
    {
        std::string error_string("Unknown error while deducing working relative area from task");
        log_message(error_string, logging::trivial::error);
    }

    return NFmiRect(0,0,1,1); // jos joku meni pieleen, palautetaan default laatikko
}

static bool do_gridding_work_with_toolmaster(const tcp_tools::task_structure &task_in, tcp_tools::work_result_structure &work_result_out, bool use_verbose_logging)
{
    static int counter = 0;
    counter++;

    std::stringstream out;
    out << task_in;
    std::string task_string = use_verbose_logging ? "Starting to do gridding task: " : "gridding...";
    if(use_verbose_logging)
        task_string += out.str();
    log_message(task_string, logging::trivial::info);

    std::vector<float> gridding_values(task_in.size_x_ * task_in.size_y_, kFloatMissing);
    NFmiRect relative_rect = ::get_relative_rect(task_in.relative_area_string_);
    std::vector<float> x_values(task_in.x_values_);
    std::vector<float> y_values(task_in.y_values_);
    std::vector<float> z_values(task_in.z_values_);
    if(use_verbose_logging)
        tcp_tools::log_point_values(task_in, x_values, y_values, z_values);
    FmiGriddingFunction griddingFunction = static_cast<FmiGriddingFunction>(task_in.gridding_function_);
    if(!toolmaster_tools::do_gridding(task_in.size_x_, task_in.size_y_, relative_rect, x_values, y_values, z_values, griddingFunction, gridding_values))
    {
        work_result_out = tcp_tools::work_result_structure(task_in.job_index_, task_in.data_time_index_, task_in.job_time_t_, task_in.size_x_, task_in.size_y_, std::vector<float>(), task_in.smartmet_guid_);
		return false;
	}
    else
    {
        std::string logMessage("Task finished");
        if(use_verbose_logging)
            logMessage += ", putting results back to master-process";
        log_message(logMessage, logging::trivial::info);
        work_result_out = tcp_tools::work_result_structure(task_in.job_index_, task_in.data_time_index_, task_in.job_time_t_, task_in.size_x_, task_in.size_y_, gridding_values, task_in.smartmet_guid_);
        return true;
    }
}

void CMainFrame::worker_loop_function(const tcp_tools::multi_process_tcp_pool_options &mpp_options)
{
    if(gProgramIsClosing)
        return ;
    CSingleLock singleLock(&gWorkerLoopThreadRunning);
	if(!singleLock.Lock(0))
        return ; // tähän ei pitäisi mennä, ellei pääohjelma ole jo lopettamassa

    try
    {
        worker_to_master_connection::start_worker(mpp_options.worker_name, do_gridding_work_with_toolmaster, mpp_options.verbose_logging);
        worker_to_master_connection::worker_service().run();
    }
    catch(boost::interprocess::interprocess_exception &ex)
    {
        log_message(ex.what(), logging::trivial::error);
    }
    catch(std::exception &ex)
    {
        log_message(ex.what(), logging::trivial::error);
    }
    catch(...)
    {
        log_message("Unknown exception thrown", logging::trivial::error);
    }

    log_message("Stopping", logging::trivial::info);
    gProgramIsClosing = true;
}

void CMainFrame::StartWorkerLoopThread(void)
{
    WorkerProcessMFCOptions &options = theApp.mWorkerOptions;
    tcp_tools::multi_process_tcp_pool_options mpp_options;
    mpp_options.worker_name = options.name_;
    mpp_options.verbose_logging = options.verbose_log_;

    init_logger(mpp_options.worker_name, static_cast<logging::trivial::severity_level>(options.log_level_), options.log_file_path_);
    work_queue_verbose_logging(mpp_options.verbose_logging); // laitetaan myös work_queue:n verbose-log tila päälle
  
    std::string log_string("Started (ver.");
    log_string += GetFileVersionOfApplication(CA2T(options.exe_path_.c_str()));
    log_string += ") with cmd args: ";
    log_string += options.cmd_args_;
    log_message(log_string, logging::trivial::info);

    mToolmasterInUse = toolmaster_tools::init(true, mToolmasterContext);

    log_message("Starting worker-loop-thread", logging::trivial::info);
    gWorkerThread = boost::thread(&CMainFrame::worker_loop_function, this, mpp_options);
}
