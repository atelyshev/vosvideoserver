#include "stdafx.h"
#include <boost/format.hpp>
#include "EventLogLogger.h"

using namespace std;
using boost::wformat;
using namespace loggers;

std::wstring EventLogLogger::eventSource_;

EventLogLogger::EventLogLogger()
{
}

EventLogLogger::~EventLogLogger()
{
}

void EventLogLogger::SetEventSource(const std::wstring& eventSource)
{
	eventSource_ = eventSource;
}

void EventLogLogger::WriteSuccess(const std::wstring& errMessage)
{
	WriteEventLogEntry(errMessage, EVENTLOG_SUCCESS);
}

void EventLogLogger::WriteError(const std::wstring& errMessage, unsigned long errCode)
{
	wstring errText = str(wformat(L"%1%, Error code: 0x%2$x") % errMessage % errCode);
	WriteEventLogEntry(errText, EVENTLOG_ERROR_TYPE);
}

void EventLogLogger::WriteWarning(const std::wstring& errMessage)
{
	WriteEventLogEntry(errMessage, EVENTLOG_WARNING_TYPE);
}

void EventLogLogger::WriteInformation(const std::wstring& errMessage)
{
	WriteEventLogEntry(errMessage, EVENTLOG_INFORMATION_TYPE);
}

void EventLogLogger::WriteAuditSuccess(const std::wstring& errMessage)
{
	WriteEventLogEntry(errMessage, EVENTLOG_AUDIT_SUCCESS);
}

void EventLogLogger::WriteAuditFailure(const std::wstring& errMessage)
{
	WriteEventLogEntry(errMessage, EVENTLOG_AUDIT_FAILURE);
}

void EventLogLogger::WriteEventLogEntry(const wstring& errMessage, WORD wType)
{
	HANDLE hEventSource = NULL;
	LPCWSTR lpszStrings[2] = { NULL, NULL };

	hEventSource = RegisterEventSource(NULL, eventSource_.c_str());
	if (hEventSource)
	{
		lpszStrings[0] = eventSource_.c_str();
		lpszStrings[1] = errMessage.c_str();

		ReportEvent(hEventSource,  // Event log handle
			wType,                 // Event type
			0,                     // Event category
			0,                     // Event identifier
			NULL,                  // No security identifier
			2,                     // Size of lpszStrings array
			0,                     // No binary data
			lpszStrings,           // Array of strings
			NULL                   // No binary data
			);

		DeregisterEventSource(hEventSource);
	}
}