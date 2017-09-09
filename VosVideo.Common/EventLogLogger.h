#pragma once

namespace loggers
{
	class EventLogLogger
	{
	public:
		EventLogLogger();
		virtual ~EventLogLogger();

		static void SetEventSource(const std::wstring& eventSource);
		static void WriteSuccess(const std::wstring& errMessage);
		static void WriteError(const std::wstring& errMessage, unsigned long errCode = GetLastError());
		static void WriteWarning(const std::wstring& errMessage);
		static void WriteInformation(const std::wstring& errMessage);
		static void WriteAuditSuccess(const std::wstring& errMessage);
		static void WriteAuditFailure(const std::wstring& errMessage);

	private: 
		static void WriteEventLogEntry(const std::wstring& errMessage, WORD wType);

		static std::wstring eventSource_;
	};
}
