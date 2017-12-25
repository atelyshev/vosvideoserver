#pragma once
#include "LoggersCommon.h"

namespace loggers
{
	class StdLogger : public LoggersCommon
	{
	public:
		StdLogger(const std::wstring &wlogPath, const std::wstring& wlogDir, const std::wstring &wfPrefix);
		virtual std::string CreateLogFileName();

	private:
		FILE *_stderr;
		FILE *_stdout;
	};
}
