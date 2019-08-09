#include "stdafx.h"
#include "worker_process_info.h"

base_worker_process_start_info::base_worker_process_start_info()
:worker_executable_name_str_()
,worker_project_name_()
,worker_base_name_str_()
,extra_args_()
{}

base_worker_process_start_info::base_worker_process_start_info(const std::string &worker_executable_name_str, const std::string &worker_project_name, const std::string &worker_base_name_str, const std::vector<std::string> &extra_args)
:worker_executable_name_str_(worker_executable_name_str)
,worker_project_name_(worker_project_name)
,worker_base_name_str_(worker_base_name_str)
,extra_args_(extra_args)
{}

worker_process_info::worker_process_info(const std::string &base_shared_name)
:final_name_("undefined")
,child_process_()
,running_info_(base_shared_name, boost::interprocess::open_or_create, 1024)
,heartbeat_checker_(worker_expire_timeout_in_ms)
,has_died_(false)
,base_process_info_()
{}
