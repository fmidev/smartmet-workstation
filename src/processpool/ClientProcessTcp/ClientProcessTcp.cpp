// Client -> server -> client -yhteyksien testaamista varten.
// Tämä on client_to_master_connection -luokan käyttöä ja se käynnistää MasterProcess -ohjelman.
// Tämä on vähän kuin SmartMetissa oleva client osio dos-ohjelmassa.

#ifdef WIN32
#pragma warning (disable : 4505)
#endif

#include "tcp_tools.h"
#include "logging.h"
#include "client_to_master_connection.h"
#include "process_helpers.h"

#include "boost/thread.hpp"

static void generate_starting_test_tasks(void)
{
    size_t running_task_index = 1;
    tcp_tools::fill_work_queue(client_to_master_connection::task_queue(), running_task_index);
    log_message("Writing generated workqueue", logging::trivial::info);
    tcp_tools::log_concurrent_queue(client_to_master_connection::task_queue(), logging::trivial::info);
}

static void test_task_generation(bool just_generate_tasks)
{
    generate_starting_test_tasks();

    if(just_generate_tasks)
        return ;

    std::stringstream stream;
    log_message("Writing taskqueue to stream", logging::trivial::info);
    tcp_tools::write_concurrent_queue(stream, client_to_master_connection::task_queue());
    tcp_tools::concurrent_queue<tcp_tools::task_structure> workqueue2;
    log_message("Reading taskqueue from stream", logging::trivial::info);
    tcp_tools::read_queue(stream, workqueue2);
    tcp_tools::log_concurrent_queue(workqueue2, logging::trivial::info);

    tcp_tools::make_some_fake_work(client_to_master_connection::task_queue(), client_to_master_connection::result_queue(), true);
    log_message("Writing fake workresult queue", logging::trivial::info);
    tcp_tools::log_concurrent_queue(client_to_master_connection::result_queue(), logging::trivial::info);

    std::stringstream stream_result;
    log_message("Writing workresult queue to stream", logging::trivial::info);
    tcp_tools::write_concurrent_queue(stream_result, client_to_master_connection::result_queue());
    tcp_tools::concurrent_queue<tcp_tools::work_result_structure> workresult_queue2;
    log_message("Reading workresult queue from stream", logging::trivial::info);
    tcp_tools::read_queue(stream_result, workresult_queue2);
    tcp_tools::log_concurrent_queue(workresult_queue2, logging::trivial::info);
}

const std::string client_name = tcp_tools::g_client_login_base_string + tcp_tools::g_sample_guid;
const std::string master_name = "MasterTcp";
const std::string master_project_name = "MasterProcessTcp";
const std::string master_executable_name = master_project_name + ".exe";
const std::string master_executable_absolute_path = ""; // tätä ei tiedetä, mutta se on annettava process_helpers::make_worker_executable_full_path -funktiolle

int main(int /* argc */ , char* /* argv */ []) 
{
    init_logger(client_name, logging::trivial::trace, default_logging_file_tcp, true);
    log_message("Started", logging::trivial::info);

    // Anna parametriksi joku ei "g_missing_random_seed" vakio luku, jos haluat vakio 'random' lukuja.
    tcp_tools::seed_random_generator(tcp_tools::g_missing_random_seed);

    bool create_work = true;
    if(create_work)
        ::generate_starting_test_tasks();

    bool start_master_process = false;
    if(start_master_process)
    {
        std::vector<std::string> extra_args;
        tcp_tools::create_worker_process(process_helpers::make_worker_executable_full_path(master_executable_absolute_path, master_executable_name, master_project_name), master_name, "Creating master Tcp process: ", extra_args, false);
    }

    bool sleep_and_wait = true;
    if(sleep_and_wait)
        boost::this_thread::sleep(boost::posix_time::milliseconds(1000 * 30));

    client_to_master_connection::start_client(client_name, true);

    client_to_master_connection::client_service().run();

}
