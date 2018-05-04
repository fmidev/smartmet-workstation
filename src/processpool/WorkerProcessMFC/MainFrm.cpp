
// MainFrm.cpp : implementation of the CMainFrame class
//

#include "stdafx.h"
#include "WorkerProcessMFC.h"

#include "MainFrm.h"
#include "NFmiObsDataGridding_from_ModifyEditedData.h"
#include "MfcTools.h"

#include "logging.h"
#include "process_helpers.h"
#include "toolmaster_tools.h"

#include "NFmiRect.h"

#include "boost/lexical_cast.hpp"
#include <boost/thread/thread.hpp> 
#include <boost/random.hpp>
#include <boost/format.hpp>
#include <boost/algorithm/string.hpp>

//#include <agx\agx.h>

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
:mToolmasterInUse(false)
,mToolmasterContext(-1)
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


work_result_t make_gridding_result(const task_holder_t &task, process_helpers::result_allocator_holder_ptr_t &result_allocator_holder, const std::vector<float> &gridding_values)
{
    std::string guid(task->smartmet_guid_.begin(), task->smartmet_guid_.end());
    return work_result_t(*result_allocator_holder->float_allocator_.get(), *result_allocator_holder->char_allocator_.get(), task->job_index_, task->data_time_index_, task->job_time_t_, task->size_x_, task->size_y_, gridding_values, guid);
}

void log_point_values(const task_holder_t &task, const std::vector<float> &x_values, const std::vector<float> &y_values, const std::vector<float> &z_values)
{
    const logging::trivial::severity_level point_values_log_level = logging::trivial::trace;
    if(get_used_severity_level() >= point_values_log_level)
    { // turha rakentaa tätä työlästä stringiä, jos sitä ei kuitenkaan logata
        const std::string point_value_format_string = "%8.2f";
        std::string log_string = "Locations points for job-index: ";
        log_string += boost::lexical_cast<std::string>(task->job_index_);
        log_string += " (data-time-index: ";
        log_string += boost::lexical_cast<std::string>(task->data_time_index_);
        log_string += " guid: ";
        log_string += std::string(task->smartmet_guid_.begin(), task->smartmet_guid_.end());
        log_string += ") has XYZ-points:\n";
        for(size_t i = 0; i < x_values.size(); i++)
        {
            log_string += boost::str(boost::format(point_value_format_string) % x_values[i]);
            log_string += boost::str(boost::format(point_value_format_string) % y_values[i]);
            log_string += boost::str(boost::format(point_value_format_string) % z_values[i]);
            log_string += "\n";
        }
        log_message(log_string, point_values_log_level);
    }
}

boost::random::mt19937 random_generator; // produces randomness out of thin air see pseudo-random number generators
boost::random::uniform_int_distribution<> point_count_rand(4, 15); 
boost::random::uniform_real_distribution<float> relative_place_rand(0.f, 1.f); 
boost::random::uniform_real_distribution<float> z_value_rand(0.f, 5.f); 

void make_random_point_values(std::vector<float> &x_values, std::vector<float> &y_values, std::vector<float> &z_values)
{
    size_t point_count = point_count_rand(random_generator);
    for(size_t i = 0; i < point_count; i++)
    {
        x_values.push_back(relative_place_rand(random_generator));
        y_values.push_back(relative_place_rand(random_generator));
        z_values.push_back(z_value_rand(random_generator));
    }
}

NFmiRect get_relative_rect(task_holder_t &task)
{
    try
    {
        std::string relative_rect_string(task->relative_area_string_.begin(), task->relative_area_string_.end());
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

template<typename WorkerProcessStuff>
bool do_gridding_work_with_toolmaster(WorkerProcessStuff &workerProcessStuff)
{
    task_holder_t &task = workerProcessStuff.workqueue->try_pop_task();
    result_queue_ptr_t &work_result_queue = workerProcessStuff.work_result_queue;
    process_helpers::result_allocator_holder_ptr_t &result_allocator_holder = workerProcessStuff.result_allocator_holder;
    if(task)
    {
        workerProcessStuff.running_info->increase_task_counter();
        std::stringstream out;
        out << *task;
        std::string task_string = "Starting to do gridding task: ";
        task_string += out.str();
        log_message(task_string, logging::trivial::info);

        std::vector<float> gridding_values(task->size_x_ * task->size_y_, kFloatMissing);
        NFmiRect relative_rect = get_relative_rect(task);
        std::vector<float> x_values(task->x_values_.begin(), task->x_values_.end());
        std::vector<float> y_values(task->y_values_.begin(), task->y_values_.end());
        std::vector<float> z_values(task->z_values_.begin(), task->z_values_.end());
        log_point_values(task, x_values, y_values, z_values);
        FmiGriddingFunction griddingFunction = static_cast<FmiGriddingFunction>(task->gridding_function_);
        if(!toolmaster_tools::do_gridding(task->size_x_, task->size_y_, relative_rect, x_values, y_values, z_values, griddingFunction, gridding_values))
        {
            work_result_queue->push_task(make_gridding_result(task, result_allocator_holder, std::vector<float>()));
			return false;
		}
        else
        {
            std::string logMessage("Task finished");
            if(workerProcessStuff.mpp_options.verbose_logging)
                logMessage += ", putting results to result-queue";
            log_message(logMessage, logging::trivial::info);
            work_result_queue->push_task(make_gridding_result(task, result_allocator_holder, gridding_values));
            return true;
        }
    }
    else
    {
        log_message("Given task was empty", logging::trivial::trace);
        return false;
    }
}


typedef process_helpers::worker_process_stuff<task_queue_t, result_queue_t> worker_process_stuff_t;

void CMainFrame::worker_loop_function(const std::string &index_string, const process_helpers::multi_process_pool_options &mpp_options_in)
{
    if(gProgramIsClosing)
        return ;
    CSingleLock singleLock(&gWorkerLoopThreadRunning);
	if(!singleLock.Lock(0))
        return ; // tähän ei pitäisi mennä, ellei pääohjelma ole jo lopettamassa

    try
    {
        worker_process_stuff_t worker_process_data(index_string, mpp_options_in);
        process_helpers::do_work(worker_process_data, do_gridding_work_with_toolmaster<worker_process_stuff_t>);

        //log_message("Sleeping for 1 second", logging::trivial::info);
        //boost::this_thread::sleep(boost::posix_time::milliseconds(1000));
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
    std::string index_string = boost::lexical_cast<std::string>(options.index_);
    std::string workerName = options.name_ + "#" + index_string;

    process_helpers::multi_process_pool_options mpp_options;
    mpp_options.verbose_logging = options.verbose_log_;

    init_logger(workerName, static_cast<logging::trivial::severity_level>(options.log_level_), options.log_file_path_);
    work_queue_verbose_logging(mpp_options.verbose_logging); // laitetaan myös work_queue:n verbose-log tila päälle
  
    std::string log_string("Started (ver.");
    log_string += GetFileVersionOfApplication(CA2T(options.exe_path_.c_str()));
    log_string += ") with cmd args: ";
    log_string += options.cmd_args_;
    log_message(log_string, logging::trivial::info);

    mToolmasterInUse = toolmaster_tools::init(true, mToolmasterContext);

    log_message("Starting worker-loop-thread", logging::trivial::info);
    gWorkerThread = boost::thread(&CMainFrame::worker_loop_function, this, index_string, mpp_options);
}
