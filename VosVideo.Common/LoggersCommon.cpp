#include "StdAfx.h"
#include <boost/format.hpp>
#include <boost/filesystem.hpp>
#include "LoggersCommon.h"
#include "StringUtil.h"

using namespace std;
using namespace loggers;
using namespace util;

//const string LoggersCommon::_fNamePattern = "_%Y-%m-%d_%H-%M-%S.txt";
//const string LoggersCommon::_loggerSeparator = ";";

LoggersCommon::LoggersCommon(const wstring& wlogPath)
{
	// Workaround for boost log v1.0, if file path created loggers will not throw fisrt chance exception
	// which is quite annoying
	string logPath = StringUtil::ToString(wlogPath);
	boost::filesystem::create_directory(logPath);
}

LoggersCommon::~LoggersCommon()
{
	if (_pSink != NULL)
	{
		_pSink->stop();
		_pSink->feed_records();
	}
}

string LoggersCommon::CreateLogFileName(const string& logPath, const string& logPrefix)
{
	string name = str(boost::format("%1%\\%2%\\%3%") % logPath % logPrefix % _fNamePattern);
	return name;
}

