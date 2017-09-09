#pragma once

#include <cassert>
#include <iostream>
#include <fstream>
#include <boost/exception/all.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/log/common.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/attributes.hpp>
#include <boost/log/sinks.hpp>
#include <boost/log/sources/logger.hpp>
#include <boost/core/null_deleter.hpp>
#include <boost/log/utility/manipulators/add_value.hpp>
#include <boost/log/attributes/scoped_attribute.hpp>
#include <boost/log/support/date_time.hpp>

namespace logging = boost::log;
namespace sinks = boost::log::sinks;
namespace attrs = boost::log::attributes;
namespace src = boost::log::sources;
namespace keywords = boost::log::keywords;
namespace expr = boost::log::expressions;

// Here we define our application severity levels.
enum severity_level
{
	// Severity logger tags
    vv_debug,
    vv_trace,
    vv_warning,
    vv_error,
    vv_critical,
};

// The formatting logic for the severity level
template< typename CharT, typename TraitsT >
inline std::basic_ostream< CharT, TraitsT >& operator<< (
    std::basic_ostream< CharT, TraitsT >& strm, severity_level lvl)
{
	static const char* const str[] =
	{
		// Severity logger tags
		"debug",
		"trace",
		"warning",
		"error",
		"critical",
	};
    if (static_cast< std::size_t >(lvl) < (sizeof(str) / sizeof(*str)))
        strm << str[lvl];
    else
        strm << static_cast< int >(lvl);
    return strm;
}

namespace loggers
{
	class LoggersCommon
	{
	public:
		LoggersCommon(const std::wstring& wlogPath);
		virtual ~LoggersCommon();

		std::string CreateLogFileName(const std::string &logPath, const std::string &logPrefix);
	protected:
		static const int _minFreeSpaceSize = 50;
		using text_sink = sinks::asynchronous_sink< sinks::text_file_backend >;
		boost::shared_ptr< text_sink > _pSink;
		const std::string _loggerSeparator = ";";
		const std::string _fNamePattern = "%Y-%m-%d_%H-%M-%S.txt";
	};
}

