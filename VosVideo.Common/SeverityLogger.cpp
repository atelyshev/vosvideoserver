#include "StdAfx.h"
#include <process.h>
#include <boost/thread.hpp>
#include <boost/log/sinks/sync_frontend.hpp>
#include "SeverityLogger.h"
#include "StringUtil.h"

using namespace std;
using boost::shared_ptr;
using namespace loggers;
using namespace util;

SeverityLogger::SeverityLogger(const wstring& wlogPath, 
								const wstring& wfPrefix, 
								int nLogFileSizeInMb /* 50 */, 
								int nMaxRollBackFileLimit /* 50 */) : 
								LoggersCommon(wlogPath)
{	
	string logPath = StringUtil::ToString(wlogPath);
	string fPrefix= StringUtil::ToString(wfPrefix);

	string logFile = CreateLogFileName(logPath, fPrefix);
	auto backend = boost::make_shared< sinks::text_file_backend >(keywords::file_name = logFile,
		keywords::rotation_size = nLogFileSizeInMb * 1024 * 1024);
	_pSink.reset(new text_sink(backend));

	// Set up the file collector
	backend->set_file_collector(sinks::file::make_collector(
		// rotated logs will be moved here
		keywords::target = logPath,
		// oldest log files will be removed if the total size reaches 100 MB...
		keywords::max_size = nLogFileSizeInMb * nMaxRollBackFileLimit * 1024 * 1024,
		// ...or the free space in the target directory comes down to 50 MB
		keywords::min_free_space = _minFreeSpaceSize * 1024 * 1024
	));

	// Ok, we're ready to add the sink to the logging library
	logging::core::get()->add_sink(_pSink);
	_pSink->locked_backend()->auto_flush(true);

	// Each logging record may have a number of attributes in addition to the
	// message body itself. By setting up formatter we define which of them
	// will be written to log and in what way they will look there.
	_pSink->set_formatter
	(
		expr::stream
		<< expr::format_date_time< boost::posix_time::ptime >("TimeStamp", "%Y-%m-%d %H:%M:%S.%f") << _loggerSeparator 
		<< "[" << expr::attr< severity_level >("Severity") << "]"<< _loggerSeparator 
		<< "[" << boost::this_thread::get_id() << "]" << _loggerSeparator
		<< "[" << expr::format_named_scope("Scope", keywords::format = "%n", keywords::iteration = expr::reverse) << "]" << _loggerSeparator
		<< expr::smessage
	); // here goes the log record text

	// And similarly add a time stamp
	logging::core::get()->add_global_attribute("TimeStamp", attrs::local_clock());

	// Let's also track the execution scope from which the records are made
	logging::core::get()->add_global_attribute("Scope", attrs::named_scope());

	// Now we can set the filter. A filter is essentially a functor that returns
	// boolean value that tells whether to write the record or not.
	_pSink->set_filter(expr::attr< severity_level >("Severity") <= vv_critical); 

#ifndef _DEBUG // filter out debug level
	_pSink->set_filter(expr::attr< severity_level >("Severity") > vv_debug); // Write records with "debug" severity also
#endif
}

