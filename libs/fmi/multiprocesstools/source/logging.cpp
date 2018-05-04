
#include "logging.h"

#include <fstream>

#ifdef _MSC_VER
#pragma warning (push)
#pragma warning (disable : 4100 4510 4512 4610 4714 )
#endif

#include <boost/log/core.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/sinks/text_file_backend.hpp>
#include <boost/log/sinks/text_ostream_backend.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/sources/record_ostream.hpp>
#include <boost/log/attributes/clock.hpp>
#include <boost/core/null_deleter.hpp>
#include <boost/log/support/date_time.hpp>

#ifdef _MSC_VER
#pragma warning (pop)
#endif

namespace log_src = logging::sources;
namespace log_sinks = logging::sinks;
namespace log_keywords = logging::keywords;
namespace log_attrs = logging::attributes;

#ifdef _MSC_VER
#pragma warning (disable : 4714 )
#endif

namespace
{
    logging::trivial::severity_level g_used_logging_severity_level = logging::trivial::info;
}

logging::trivial::severity_level get_used_severity_level(void)
{
    return g_used_logging_severity_level;
}

void init_logger(const std::string &my_tag, logging::trivial::severity_level min_log_level, const std::string &logging_file_path, bool log_console)
{
    boost::log::register_simple_formatter_factory< boost::log::trivial::severity_level, char >("Severity");
    logging::add_file_log
    (
        log_keywords::file_name = logging_file_path,
        log_keywords::rotation_size = 20 * 1024 * 1024,
        log_keywords::time_based_rotation = log_sinks::file::rotation_at_time_point(0, 0, 0),
        log_keywords::format = "%TimeStamp% [%Severity% %Tag%] %Message%",
        log_keywords::open_mode = (std::ios::out | std::ios::app) // is not rotated
    )->locked_backend()->auto_flush();

    g_used_logging_severity_level = min_log_level;
    logging::core::get()->set_filter
    (
        logging::trivial::severity >= min_log_level
    );

    logging::core::get()->add_global_attribute("TimeStamp", log_attrs::local_clock());
    logging::core::get()->add_global_attribute("Tag", log_attrs::constant< std::string >(my_tag));

    if(log_console)
    {
        boost::shared_ptr< logging::core > core = logging::core::get();

        // Create a backend and attach a streams to it
        boost::shared_ptr< log_sinks::text_ostream_backend > backend =
            boost::make_shared< log_sinks::text_ostream_backend >();
        backend->add_stream(
            boost::shared_ptr< std::ostream >(&std::clog, boost::null_deleter()));

        // Enable auto-flushing after each log record written
        backend->auto_flush(true);

        // Wrap it into the frontend and register in the core.
        // The backend requires synchronization in the frontend.
        typedef log_sinks::synchronous_sink< log_sinks::text_ostream_backend > sink_t;
        boost::shared_ptr< sink_t > sink(new sink_t(backend));
        core->add_sink(sink);
    }

}

void log_message(const std::string &message, logging::trivial::severity_level level)
{
    // En tiedä miten tämän saa tehtyä ilman switch-lauseketta tai ilman BOOST_LOG_TRIVIAL
    // makroja, jotka vaativat juuri oikeita sanoja, eikä parametrien arvoja
    switch (level)
    {
    case logging::trivial::trace:
        BOOST_LOG_TRIVIAL(trace) << message;
        break;
    case logging::trivial::debug:
        BOOST_LOG_TRIVIAL(debug) << message;
        break;
    case logging::trivial::info:
        BOOST_LOG_TRIVIAL(info) << message;
        break;
    case logging::trivial::warning:
        BOOST_LOG_TRIVIAL(warning) << message;
        break;
    case logging::trivial::error:
        BOOST_LOG_TRIVIAL(error) << message;
        break;
    case logging::trivial::fatal:
        BOOST_LOG_TRIVIAL(fatal) << message;
        break;
    default:
        break;
    }
}
