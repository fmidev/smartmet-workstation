#pragma once

#ifdef _MSC_VER
#pragma warning (push)
#pragma warning (disable : 4100 4510 4512 4610 4714 )
#endif
#include <boost/log/trivial.hpp>
#ifdef _MSC_VER
#pragma warning (pop)
#endif

namespace logging = boost::log;

const std::string default_logging_file = "D:/data/ProcessPool/processes.log";
const std::string default_logging_file_tcp = "D:/data/ProcessPool/processes_tcp.log";

void init_logger(const std::string &my_tag, logging::trivial::severity_level min_log_level, const std::string &logging_file_path, bool log_console = false);
void log_message(const std::string &message, logging::trivial::severity_level level);
logging::trivial::severity_level get_used_severity_level(void);
