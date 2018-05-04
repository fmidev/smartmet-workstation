#pragma once

#include <vector>

#ifdef _MSC_VER
#pragma warning (push)
#pragma warning (disable : 4100 4244 4267 4510 4512 4610 4714 )
#endif

#include <boost/process/process.hpp> 

#ifdef _MSC_VER
#pragma warning (pop)
#endif

boost::process::child start_process(const std::string &executable, const std::vector<std::string> &args);
