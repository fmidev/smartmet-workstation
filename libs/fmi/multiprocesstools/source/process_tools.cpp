
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
        bp::args final_args;
        for(size_t i= 0; i < args1.size(); i++)
            final_args(args1[i]);
        return bp::make_child(
            bp::paths(executable),
            final_args
            );
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
