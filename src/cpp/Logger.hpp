#pragma once

#include <string>
#include <iomanip>

#include <boost/thread/thread.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>

#include <boost/log/attributes/current_thread_id.hpp>
#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/sinks/text_file_backend.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/support/date_time.hpp>


namespace logging = boost::log;
namespace keywords = boost::log::keywords;
namespace sinks = boost::log::sinks;
namespace expr = boost::log::expressions;
namespace attrs = boost::log::attributes;


void init_logger(std::string filename)
{
    logging::add_common_attributes();

    auto format = expr::format("[%1%] [%2%] (%3%) - %4%")
                % expr::format_date_time< boost::posix_time::ptime >("TimeStamp", "%Y-%m-%d %H:%M:%S")
                % expr::attr<attrs::current_thread_id::value_type>("ThreadID")
                % logging::trivial::severity
                % expr::smessage;
                
    logging::add_console_log
    (
        std::clog,
        keywords::filter = logging::trivial::severity >= logging::trivial::info,
        keywords::format = format
    );

    logging::add_file_log
    (
        keywords::file_name = filename + "_%Y_%m_%d.%N.log",                                        
        keywords::time_based_rotation = sinks::file::rotation_at_time_point(0, 0, 0),
        keywords::filter = logging::trivial::severity >= logging::trivial::debug,
        keywords::target = "logs",
        keywords::format = format
    );
}
