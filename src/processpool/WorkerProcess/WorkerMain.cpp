
#include "NFmiFastQueryInfo.h"

#include "work_queue.h"
#include "logging.h"
#include "heartbeat_checker.h"
#include "process_helpers.h"

#ifdef _MSC_VER
#pragma warning (disable : 4512)
#endif

#include <boost/thread/thread.hpp> 
#include <boost/program_options.hpp>

typedef process_helpers::worker_process_stuff<task_queue_t, result_queue_t> worker_process_stuff_t;

class Options
{
public:
    Options()
    :name_("Worker")
    ,index_(0)
    {}

    std::string name_;
    int index_;
};

// optiot:
// 1. argumentti työläisen nimi (esim. Worker)
// 2. argumenttina id (esim. 1-n)
bool parse_options(int argc, const char * argv[], Options & options)
{
    namespace po = boost::program_options;
    static const std::string name_string = "name";
    static const std::string index_string = "index";

    po::options_description cmdOptions("Worker process, options");
    try
    {
        cmdOptions.add_options()
            (std::string(name_string+",n").c_str(),po::value(&options.name_),"Process name")
            (std::string(index_string+",i").c_str(),po::value(&options.index_),"Process index")
        ;

    po::variables_map opt;
    po::store(po::command_line_parser(argc,argv)
            .options(cmdOptions)
        .run(),
        opt);

    po::notify(opt);

    }
    catch(std::exception &ex)
    {
        std::cerr << "Error in " << __FUNCTION__ << ": " << ex.what() << std::endl;
        std::cerr << std::endl;
        std::cerr << cmdOptions << std::endl;
    }
    catch(...)
    {
        std::cerr << "Unknown error in " << __FUNCTION__ << std::endl;
        std::cerr << std::endl;
        std::cerr << cmdOptions << std::endl;
    }
    return true;
}

//#include <windows.h>
//SetErrorMode(SetErrorMode(0) | SEM_NOGPFAULTERRORBOX);

int main(int argc, const char *argv[])
{
    int exit_status_code = 0; // 0 = ok, 1 = error
    //for(;;)
    //{
    //    int x = 0;
    //}

    Options options;
    parse_options(argc, argv, options);

    std::string index_string = boost::lexical_cast<std::string>(options.index_);
    std::string workerName = options.name_ + "#" + index_string;
    process_helpers::multi_process_pool_options mpp_options;

    init_logger(workerName, logging::trivial::info, default_logging_file);
    work_queue_verbose_logging(mpp_options.verbose_logging); // laitetaan myös work_queue:n verbose-log tila päälle
    log_message("Started", logging::trivial::info);

    //std::string cmdLine;
    //for(int i=0; i < argc; i++)
    //{
    //    cmdLine += argv[i];
    //    cmdLine += " ";
    //}
    //log_message(std::string("Command line:\n")+cmdLine, logging::trivial::info);

    try
    {
        worker_process_stuff_t worker_process_data(index_string, mpp_options);
        process_helpers::do_work(worker_process_data, process_helpers::do_some_fake_work<worker_process_stuff_t>);

        //log_message("Sleeping for 1 second", logging::trivial::info);
        //boost::this_thread::sleep(boost::posix_time::milliseconds(1000));
    }
    catch(boost::interprocess::interprocess_exception &ex)
    {
        exit_status_code = 1;
        log_message(ex.what(), logging::trivial::error);
    }
    catch(std::exception &ex)
    {
        exit_status_code = 1;
        log_message(ex.what(), logging::trivial::error);
    }
    catch(...)
    {
        exit_status_code = 1;
        log_message("Unknown exception thrown", logging::trivial::error);
    }

    log_message("Stopping", logging::trivial::info);
    return exit_status_code;
}
