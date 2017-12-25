#include "StdAfx.h"
#include <boost/format.hpp>
#include <boost/filesystem.hpp>
#include "LoggersCommon.h"
#include "StringUtil.h"

using namespace std;
using namespace loggers;
using namespace util;

LoggersCommon::LoggersCommon(const std::wstring &wlogPath, const std::wstring& wlogDir, const std::wstring &wlogPrefix)
{
	// Workaround for boost log v1.0, if file path created loggers will not throw fisrt chance exception
	// which is quite annoying
	_logPath = StringUtil::ToString(wlogPath);
	_logDir = StringUtil::ToString(wlogDir);
	_logPrefix = StringUtil::ToString(wlogPrefix);

	boost::filesystem::path p(_logPath);
	p /= _logDir;
	boost::filesystem::create_directory(p);
}

LoggersCommon::~LoggersCommon()
{
	if (_pSink != nullptr)
	{
		_pSink->stop();
		_pSink->feed_records();
	}
}

string LoggersCommon::CreateLogFileName()
{
	return boost::str(boost::format("%1%\\%2%\\%3%%4%.txt") % _logPath % _logDir % _logPrefix % _fNamePattern);
}

