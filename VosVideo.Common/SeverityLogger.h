#pragma once

#include "SeverityLoggerMacros.h"


namespace loggers
{
	class SeverityLogger : public LoggersCommon
	{
	public:
		// logFolder - folder where log file should be created. If folder doesnt exists it will be created
		// fPrefix - log file prefix for instance events, general
		// nLogFileSizeInMb - max log file in Mb, when this size reached old one should be closed and new one will be created
		// nMaxRollBackFileLimit - Maximum number of files before which the Frist most created file is deleted.
		SeverityLogger(const std::wstring &logFolder, 
			          const std::wstring &fPrefix, 
					  int nLogFileSizeInMb = 50, 
					  int nMaxRollBackFileLimit = 50);
	};
}
