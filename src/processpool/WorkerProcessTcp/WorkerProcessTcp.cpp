
#include "worker_to_master_connection.h"
#include "logging.h"


int main(int argc, const char* argv[]) 
{
    std::string worker_name = "default name";
    if(argc > 1)
        worker_name = argv[1];
    init_logger(worker_name, logging::trivial::trace, default_logging_file_tcp);
    log_message("Started", logging::trivial::info);

    // Anna 1. parametriksi joku vakio luku, äläkä anna 2. argumenttia, jos haluat vakio 'random' lukuja.
    tcp_tools::seed_random_generator(tcp_tools::g_missing_random_seed, worker_name);

    worker_to_master_connection::start_worker(worker_name, tcp_tools::make_some_fake_result, true); // 2. parametrina annetaan käytettävä työ-funktio (millä task:ista tehdään result)
    worker_to_master_connection::worker_service().run();
}














