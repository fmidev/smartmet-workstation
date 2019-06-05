#pragma once

#include "work_queue.h"
#include "heartbeat_checker.h"

#ifdef _MSC_VER
#pragma warning (push)
#pragma warning (disable : 4100 4244 4267 4510 4512 4610 4714 )
#endif

#include <boost/process.hpp> 

#ifdef _MSC_VER
#pragma warning (pop)
#endif

const int worker_expire_timeout_in_ms = 60 * 1000; // annetaan minuutin max työ aika workereille, ennen kuin master-prosessi toteaa sen kuolleeksi

class base_worker_process_start_info
{
public:
    base_worker_process_start_info();
    base_worker_process_start_info(const std::string &worker_executable_name_str, const std::string &worker_project_name, const std::string &worker_base_name_str, const std::vector<std::string> &extra_args = std::vector<std::string>());
    std::string worker_executable_name_str_; // esim. wirker.exe
    std::string worker_project_name_; // esim. WorkerProcess
    std::string worker_base_name_str_; // esim. WorkerProcess
    std::vector<std::string> extra_args_; // eri ympäristöissä pitää ehkä antaa worker prosesseille erilaisia aloitus argumentteja, tähän laitetaan ne (-n name ja -i index ovat kaikille, ja niitä ei laiteta tähän)
};


class worker_process_info
{
public:
    worker_process_info(const std::string &base_shared_name);

    std::string final_name_; // esim. WorkerProcess1
    boost::process::child child_process_;
    worker_running_info running_info_;
    heartbeat_checker heartbeat_checker_;
    bool has_died_;
    base_worker_process_start_info base_process_info_;
};

typedef boost::shared_ptr<worker_process_info> worker_process_info_ptr;
typedef std::vector<worker_process_info_ptr> worker_process_info_vector_t;
