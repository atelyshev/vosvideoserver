#include "stdafx.h"
#include <stdio.h>
#include <stdlib.h>
#include <boost/format.hpp>
#include "StringUtil.h"
#include "StdLogger.h"

using namespace loggers;
using namespace util;

StdLogger::StdLogger(const std::wstring &wlogPath, const std::wstring& wlogDir, const std::wstring &wlogPrefix) :
	LoggersCommon(wlogPath, wlogDir, wlogPrefix)
{
	std::string logFile = CreateLogFileName();

	if ((_stderr = freopen(logFile.c_str(), "w", stderr)) == nullptr)
		std::cout << "Failed to redirect stderr to file" << std::endl;
	if ((_stdout = freopen(logFile.c_str(), "w", stdout)) == nullptr)
		std::cout << "Failed to redirect stderr to file" << std::endl;
}

std::string StdLogger::CreateLogFileName()
{
	std::time_t t = std::time(nullptr);
	tm timeinfo;
	localtime_s(&timeinfo, &t);
	char mbstr[100];
	std::strftime(mbstr, sizeof(mbstr), "%Y-%m-%d_%H-%M-%S", &timeinfo);
	return boost::str(boost::format("%1%\\%2%\\%3%%4%.txt") % _logPath % _logDir % _logPrefix % mbstr);
}
