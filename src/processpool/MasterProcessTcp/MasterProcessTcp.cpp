#ifdef WIN32
#pragma warning (disable : 4505)
#endif

#include "tcp_tools.h"
#include "logging.h"
#include "master_to_worker_connection.h"
#include "process_helpers.h"

#include "boost/thread.hpp"

static void generate_starting_test_tasks(void)
{
    master_to_worker_connection::set_last_working_action_time();
    size_t running_task_index = 1;
    tcp_tools::fill_work_queue(master_to_worker_connection::work_queue(), running_task_index);
    log_message("Writing generated workqueue", logging::trivial::info);
    tcp_tools::log_concurrent_queue(master_to_worker_connection::work_queue(), logging::trivial::info);
}

static void test_task_generation(bool just_generate_tasks)
{
    generate_starting_test_tasks();

    if(just_generate_tasks)
        return ;

    std::stringstream stream;
    log_message("Writing workqueue to stream", logging::trivial::info);
    tcp_tools::write_concurrent_queue(stream, master_to_worker_connection::work_queue());
    tcp_tools::concurrent_queue<tcp_tools::task_structure> workqueue2;
    log_message("Reading workqueue from stream", logging::trivial::info);
    tcp_tools::read_queue(stream, workqueue2);
    tcp_tools::log_concurrent_queue(workqueue2, logging::trivial::info);

    tcp_tools::make_some_fake_work(master_to_worker_connection::work_queue(), master_to_worker_connection::work_result_queue(), true);
    log_message("Writing fake workresult queue", logging::trivial::info);
    tcp_tools::log_concurrent_queue(master_to_worker_connection::work_result_queue(), logging::trivial::info);

    std::stringstream stream_result;
    log_message("Writing workresult queue to stream", logging::trivial::info);
    tcp_tools::write_concurrent_queue(stream_result, master_to_worker_connection::work_result_queue());
    tcp_tools::concurrent_queue<tcp_tools::work_result_structure> workresult_queue2;
    log_message("Reading workresult queue from stream", logging::trivial::info);
    tcp_tools::read_queue(stream_result, workresult_queue2);
    tcp_tools::log_concurrent_queue(workresult_queue2, logging::trivial::info);
}

static void test_binary_write_and_read()
{
    tcp_tools::make_some_fake_work(master_to_worker_connection::work_queue(), master_to_worker_connection::work_result_queue(), true);
    log_message("Writing fake workresult queue", logging::trivial::info);
    tcp_tools::log_concurrent_queue(master_to_worker_connection::work_result_queue(), logging::trivial::info);

    tcp_tools::concurrent_queue<tcp_tools::work_result_structure> result_queue2;
    tcp_tools::work_result_structure result;
    for( ; master_to_worker_connection::work_result_queue().try_pop(result); )
    {
        std::string result_binary_str = result.to_binary_string("");
        tcp_tools::work_result_structure result2;
        result2.from_binary_string(result_binary_str);
        result_queue2.push(result2);
    }

    log_message("Writing binary generated workresult queue", logging::trivial::info);
    tcp_tools::log_concurrent_queue(result_queue2, logging::trivial::info);

}

const std::string master_name = "MasterTcp";
const std::string worker_base_name = "workerTcp";
const std::string worker_project_name = "WorkerProcessTcp";
const std::string worker_executable_name = worker_project_name + ".exe";
const std::string worker_executable_absolute_path = ""; // tätä ei tiedetä, mutta se on annettava process_helpers::make_worker_executable_full_path -funktiolle

int main(int /* argc */ , char* /* argv */ []) 
{
    init_logger(master_name, logging::trivial::trace, default_logging_file_tcp, true);
    log_message("Started", logging::trivial::info);

    // Anna parametriksi joku ei "g_missing_random_seed" vakio luku, jos haluat vakio 'random' lukuja.
    tcp_tools::seed_random_generator(tcp_tools::g_missing_random_seed);

    bool create_work = true;
    if(create_work)
    {
        ::generate_starting_test_tasks();

//        ::test_binary_write_and_read();
    }

    master_to_worker_connection::initialize_master(true);
//    master_to_worker_connection::do_time_out_checks(false); // ota kommentti pois tästä, jos et halua ping timeout testejä

    bool create_workers = true;
    if(create_workers)
    {
        std::vector<std::string> extra_args;
        int worker_count = static_cast<int>(boost::thread::hardware_concurrency());
        master_to_worker_connection::startup_workers(worker_count, worker_base_name, worker_executable_absolute_path, worker_executable_name, worker_project_name, extra_args, false);
    }

    master_to_worker_connection::master_service().run();

}
