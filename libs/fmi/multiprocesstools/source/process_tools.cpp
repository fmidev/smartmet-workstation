
#include "process_tools.h"
#include "logging.h"

namespace bp = boost::process; 

#ifdef _MSC_VER
#pragma warning (disable : 4286 )
#endif

boost::process::child start_process(const std::string &executable, const std::vector<std::string> &args1)
{
    try 
    { 
        return bp::child(boost::filesystem::path(executable), args1);
    } 
    catch (boost::system::system_error &ex) 
    { 
        log_message(ex.what(), logging::trivial::error);
        throw;
    } 
    catch (std::runtime_error &ex) 
    { 
        log_message(ex.what(), logging::trivial::error);
        throw;
    } 
}
