#include "stdafx.h"
#include "ProgramOptions.h"

#include "logging.h"
#include "process_helpers.h"
#include <boost/program_options.hpp>
#include <fstream>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace po = boost::program_options;
#ifdef _UNICODE
typedef po::wcommand_line_parser used_command_line_parser;
#else
typedef po::command_line_parser used_command_line_parser;
#endif

static CString make_cmd_args_string(int argc, TCHAR *argv[])
{
    CString str;
    for(int i = 1; i < argc; i++)
    {
        str += argv[i];
        str += _TEXT(" ");
    }
    return str;
}

MasterProcessMFCOptions::MasterProcessMFCOptions()
:hide_(false)
,demo_mode_(false)
,log_file_path_(default_logging_file)
,log_level_(logging::trivial::info)
,verbose_log_(false)
,work_queue_size_in_bytes(process_helpers::work_queue_segment_size)
,result_queue_size_in_bytes(process_helpers::work_result_queue_segment_size)
{
}

// optiot:
// 1. -H argumentti piilottaa master-projektin
// 2. -d argumentti menee demo-moodiin, jolloin tehdään feikki töitä työlistaan
bool MasterProcessMFCOptions::parse_options(int argc, TCHAR *argv[])
{
    static const std::string hide_string = "hide";
    static const std::string demo_string = "demo-mode";
    static const std::string logpath_string = "log-path";
    static const std::string loglevel_string = "loglevel";
    static const std::string workqueuesize_string = "workqueuesize";
    static const std::string workerexepath_string = "workerexepath";
    static const std::string resultqueuesize_string = "resultqueuesize";
    static const std::string verboselog_string = "verboselog";

    exe_path_ = CT2A(argv[0]);
    cmd_args_ = CT2A(make_cmd_args_string(argc, argv));
    po::options_description cmdOptions("MasterMFC process, options");
    try
    {
        cmdOptions.add_options()
            (std::string(hide_string+",H").c_str(),po::bool_switch(&hide_),"Hide masterMFC process")
            (std::string(demo_string+",d").c_str(),po::bool_switch(&demo_mode_),"Put masterMFC process in demo-mode")
            (std::string(logpath_string+",l").c_str(),po::value(&log_file_path_),"Log file path")
            (std::string(loglevel_string+",L").c_str(),po::value(&log_level_),"Log level")
            (std::string(workqueuesize_string+",w").c_str(),po::value(&work_queue_size_in_bytes),"Work queue size in bytes")
            (std::string(workerexepath_string+",W").c_str(),po::value(&worker_exe_file_path_),"Worker executable path")
            (std::string(resultqueuesize_string+",r").c_str(),po::value(&result_queue_size_in_bytes),"Result queue size in bytes")
            (std::string(verboselog_string+",v").c_str(),po::bool_switch(&verbose_log_),"Verbose logging")
        ;

    po::variables_map opt;
    po::store(used_command_line_parser(argc, argv).options(cmdOptions).run(), opt);
    po::notify(opt);

    }
    catch(std::exception & ex)
    {
        std::stringstream out;
        out << "Error in " << __FUNCTION__ << ": " << ex.what() << std::endl;
        out << std::endl;
        out << cmdOptions << std::endl;
        MessageBox(AfxGetMainWnd()->GetSafeHwnd(), CA2T(out.str().c_str()), _TEXT("Error parsing options"), MB_ICONERROR | MB_OK);
    }
    catch(...)
    {
        std::stringstream out;
        out << "Unknown error in " << __FUNCTION__ << std::endl;
        out << std::endl;
        out << cmdOptions << std::endl;
        MessageBox(AfxGetMainWnd()->GetSafeHwnd(), CA2T(out.str().c_str()), _TEXT("Error parsing options"), MB_ICONERROR | MB_OK);
    }
    return true;
}


WorkerProcessMFCOptions::WorkerProcessMFCOptions()
:name_("Worker")
,index_(0)
,hide_(false)
,log_file_path_(default_logging_file)
,log_level_(logging::trivial::info)
,verbose_log_(false)
{
}

// debuggaus funktio
static void write_cmd_args_to_file(int argc, TCHAR *argv[], const std::string &file_name)
{
    std::ofstream out(file_name);
    if(out)
    {
        for(int i = 0; i < argc; i++)
        {
            out << i << ": " << CT2A(argv[i]) << std::endl;
        }
    }
}

// optiot:
// 1. -n optio = työläisen nimi (esim. Worker)
// 2. -i optio = worker-id (esim. 1-n)
// 3. -h argumentti piilottaa worker-prosessin
bool WorkerProcessMFCOptions::parse_options(int argc, TCHAR *argv[])
{
    namespace po = boost::program_options;
    static const std::string name_string = "name";
    static const std::string index_string = "index";
    static const std::string hide_string = "hide";
    static const std::string logpath_string = "logpath";
    static const std::string loglevel_string = "loglevel";
    static const std::string verboselog_string = "verboselog";

//    write_cmd_args_to_file(argc, argv, "D:\\worker_cmd_args.txt");

    exe_path_ = CT2A(argv[0]);
    cmd_args_ = CT2A(make_cmd_args_string(argc, argv));

    po::options_description cmdOptions("Worker process, options");
    try
    {
        cmdOptions.add_options()
            (std::string(name_string+",n").c_str(),po::value(&name_),"Process name")
            (std::string(logpath_string+",l").c_str(),po::value(&log_file_path_),"Log file path")
            (std::string(loglevel_string+",L").c_str(),po::value(&log_level_),"Log level")
            (std::string(index_string+",i").c_str(),po::value(&index_),"Process index")
            (std::string(hide_string+",H").c_str(),po::bool_switch(&hide_),"Hide worker process")
            (std::string(verboselog_string+",v").c_str(),po::bool_switch(&verbose_log_),"Verbose logging")
        ;

    po::variables_map opt;
    po::store(used_command_line_parser(argc,argv).options(cmdOptions).run(), opt);
    po::notify(opt);

    }
    catch(std::exception & ex)
    {
        std::stringstream out;
        out << "Error in " << __FUNCTION__ << ": " << ex.what() << std::endl;
        out << std::endl;
        out << cmdOptions << std::endl;
        MessageBox(AfxGetMainWnd()->GetSafeHwnd(), CA2T(out.str().c_str()), _TEXT("Error parsing options"), MB_ICONERROR | MB_OK);
    }
    catch(...)
    {
        std::stringstream out;
        out << "Unknown error in " << __FUNCTION__ << std::endl;
        out << std::endl;
        out << cmdOptions << std::endl;
        MessageBox(AfxGetMainWnd()->GetSafeHwnd(), CA2T(out.str().c_str()), _TEXT("Error parsing options"), MB_ICONERROR | MB_OK);
    }
    return true;
}

void CFmiWorkerCommandLineInfo::ParseParam(const TCHAR* pszParam, BOOL bFlag, BOOL bLast)
{
    // HUOM! Katso WorkerProcessMFCOptions::parse_options -metodista, mitkä optiot pitää tässä ohittaa ja miten
	static bool skipNextParam = false;
	if(skipNextParam)
	{
		skipNextParam = false;
	}
	else
	{
		if(bFlag && pszParam[0] == 'n') // name
		{
			skipNextParam = true;
		}
		else if(bFlag && pszParam[0] == 'l') // log-path
		{
			skipNextParam = true;
		}
		else if(bFlag && pszParam[0] == 'L') // log-level
		{
			skipNextParam = true;
		}
		else if(bFlag && pszParam[0] == 'i') // index in pool
		{
			skipNextParam = true;
		}
		else if(bFlag && pszParam[0] == 'H') // hide
		{
			skipNextParam = false;
		}
		else if(bFlag && pszParam[0] == 'v') // verbose log
		{
			skipNextParam = false;
		}
		else
		{
			if (bFlag)
				CCommandLineInfo::ParseParamFlag(CT2A(pszParam));
			else
				CCommandLineInfo::ParseParamNotFlag(pszParam);
			ParseLast(bLast);
		}
	}
}

void CFmiMasterCommandLineInfo::ParseParam(const TCHAR* pszParam, BOOL bFlag, BOOL bLast)
{
    // HUOM! Katso MasterProcessMFCOptions::parse_options -metodista, mitkä optiot pitää tässä ohittaa ja miten
	static bool skipNextParam = false;
	if(skipNextParam)
	{
		skipNextParam = false;
	}
	else
	{
		if(bFlag && pszParam[0] == 'd') // d niin kuin demo-moodi, jolloin tehdään feikki töitä työlistaan
		{
			skipNextParam = false;
		}
		else if(bFlag && pszParam[0] == 'H') // piiloitus moodi
		{
			skipNextParam = false;
		}
		else if(bFlag && pszParam[0] == 'l') // loki tiedoston polku
		{
			skipNextParam = true;
		}
		else if(bFlag && pszParam[0] == 'L') // log-level
		{
			skipNextParam = true;
		}
		else if(bFlag && pszParam[0] == 'w') // work-queue size
		{
			skipNextParam = true;
		}
		else if(bFlag && pszParam[0] == 'W') // Worker-exen polku
		{
			skipNextParam = true;
		}
		else if(bFlag && pszParam[0] == 'r') // result-queue size
		{
			skipNextParam = true;
		}
		else if(bFlag && pszParam[0] == 'v') // verbose log
		{
			skipNextParam = false;
		}
		else
		{
			if (bFlag)
				CCommandLineInfo::ParseParamFlag(CT2A(pszParam));
			else
				CCommandLineInfo::ParseParamNotFlag(pszParam);
			ParseLast(bLast);
		}
	}
}
