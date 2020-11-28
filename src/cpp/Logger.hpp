#pragma once

#include <string>
#include <iomanip>

#include <boost/thread/thread.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/format.hpp>

#include <boost/log/attributes/current_thread_id.hpp>
#include <boost/log/attributes/mutable_constant.hpp>
#include <boost/log/core.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/sinks/text_file_backend.hpp>
#include <boost/log/sources/global_logger_storage.hpp>
#include <boost/log/sources/severity_feature.hpp>
#include <boost/log/sources/severity_channel_logger.hpp>
#include <boost/log/support/date_time.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/utility/manipulators/add_value.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/utility/setup/file.hpp>

#include <boost/algorithm/string/predicate.hpp>
#include <boost/log/common.hpp>
#include <boost/log/sinks.hpp>
#include <boost/log/expressions/keyword.hpp>
#include <boost/log/attributes.hpp>
#include <boost/log/utility/exception_handler.hpp>
#include <boost/log/utility/setup/filter_parser.hpp>
#include <boost/log/utility/setup/formatter_parser.hpp>
#include <boost/log/utility/setup/from_stream.hpp>
#include <boost/log/utility/setup/settings.hpp>
#include <boost/log/sinks/sync_frontend.hpp>
#include <boost/log/sinks/text_ostream_backend.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>

namespace logging = boost::log;
namespace keywords = boost::log::keywords;
namespace sinks = boost::log::sinks;
namespace expr = boost::log::expressions;
namespace attrs = boost::log::attributes;

// Convert file path to only the filename
std::string path_to_filename(std::string path) {
   return path.substr(path.find_last_of("/\\")+1);
}

template<typename... Arguments>
std::string format(std::string&& fmt, Arguments&&... args)
{
    boost::format f(fmt);
    return (f % ... % std::forward<Arguments>(args));
}


BOOST_LOG_GLOBAL_LOGGER(sysLogger,
  logging::sources::severity_channel_logger_mt<logging::trivial::severity_level>);

BOOST_LOG_GLOBAL_LOGGER_CTOR_ARGS(sysLogger,
  boost::log::sources::severity_channel_logger_mt<logging::trivial::severity_level>,
  (keywords::channel = "SYSLF"));

#define LOG_LOG_LOCATION(LOGGER, LEVEL, ARG, ARGS)              \
  BOOST_LOG_SEV(LOGGER, boost::log::trivial::LEVEL)              \
    << boost::log::add_value("Line", __LINE__)                    \
    << boost::log::add_value("File", path_to_filename(__FILE__))   \
    << boost::log::add_value("Function", __FUNCTION__) << ARG;

#define LOG_TRACE(ARG, ...) LOG_LOG_LOCATION(sysLogger::get(), trace, ARG, __VA_ARGS__);
#define LOG_DEBUG(ARG, ...) LOG_LOG_LOCATION(sysLogger::get(), debug, ARG, __VA_ARGS__);
#define LOG_INFO(ARG, ...)  LOG_LOG_LOCATION(sysLogger::get(), info, ARG, __VA_ARGS__);
#define LOG_WARN(ARG, ...)  LOG_LOG_LOCATION(sysLogger::get(), warning, ARG, __VA_ARGS__);
#define LOG_ERROR(ARG, ...) LOG_LOG_LOCATION(sysLogger::get(), error, ARG, __VA_ARGS__);
#define LOG_FATAL(ARG, ...) LOG_LOG_LOCATION(sysLogger::get(), fatal, ARG, __VA_ARGS__);

/*!
 * Initialises the logging.
 *
 * @param filename The name of the binary running the logger
 */
void init_logger(std::string filename)
{
    logging::add_common_attributes();

    auto format = expr::format("[%1%] {%2%} <%3%> (%4%:%5%) - %6%")
                % expr::format_date_time< boost::posix_time::ptime >("TimeStamp", "%Y-%m-%d %H:%M:%S")
                % expr::attr<attrs::current_thread_id::value_type>("ThreadID")
                % logging::trivial::severity
                % expr::attr<std::string>("File")
                % expr::attr<int>("Line")
                % expr::smessage;
                
    logging::add_console_log
    (
        std::clog,
        keywords::filter = logging::trivial::severity >= logging::trivial::info,
        keywords::format = format
    );

    logging::add_file_log
    (
        keywords::file_name = "logs/" + filename + "_%Y_%m_%d.log",                                        
        keywords::time_based_rotation = sinks::file::rotation_at_time_interval(boost::posix_time::hours(1)),
        keywords::filter = logging::trivial::severity >= logging::trivial::debug,
        keywords::target = "logs",
        keywords::format = format,
        keywords::auto_flush = true,
        keywords::open_mode = std::ios::out | std::ios::app
    );
}
