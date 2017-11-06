/****************************** Module Header ******************************\
* Module Name:  SampleService.cpp
* Project:      CppWindowsService
* Copyright (c) Microsoft Corporation.
* 
* Provides a sample service class that derives from the service base class - 
* CServiceBase. The sample service logs the service start and stop 
* information to the Application event log, and shows how to run the main 
* function of the service in a thread pool worker thread.
* 
* This source is subject to the Microsoft Public License.
* See http://www.microsoft.com/en-us/openness/resources/licenses.aspx#MPL.
* All other rights reserved.
* 
* THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND, 
* EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED 
* WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR PURPOSE.
\***************************************************************************/

#pragma region Includes
#include "stdafx.h"
#include <boost/format.hpp>
#include "VosVideo.Configuration/ServiceCredentialsManager.h"
#include "VosVideo.Configuration/CredentialsException.h"
#include "RtbcService.h"
#include "ThreadPool.h"
#pragma endregion

using namespace std;
using namespace util;
using boost::wformat;
using namespace loggers;
using namespace vosvideo::configuration;

RtbcService::RtbcService(Application* appPtr,
						 PWSTR pszServiceName, 
                         BOOL fCanStop, 
                         BOOL fCanShutdown, 
                         BOOL fCanPauseContinue)
: ServiceBase(pszServiceName, fCanStop, fCanShutdown, fCanPauseContinue), appPtr_(appPtr)
{
    // Create a manual-reset event that is not signaled at first to indicate 
    // the stopped signal of the service.
    m_hStoppedEvent = CreateEvent(nullptr, TRUE, FALSE, nullptr);
    if (m_hStoppedEvent == nullptr)
    {
        throw GetLastError();
    }
}


RtbcService::~RtbcService()
{
    if (m_hStoppedEvent)
    {
        CloseHandle(m_hStoppedEvent);
        m_hStoppedEvent = nullptr;
    }
}


//
//   FUNCTION: RtbcService::OnStart(DWORD, LPWSTR *)
//
//   PURPOSE: The function is executed when a Start command is sent to the 
//   service by the SCM or when the operating system starts (for a service 
//   that starts automatically). It specifies actions to take when the 
//   service starts. In this code sample, OnStart logs a service-start 
//   message to the Application log, and queues the main service function for 
//   execution in a thread pool worker thread.
//
//   PARAMETERS:
//   * dwArgc   - number of command line arguments
//   * lpszArgv - array of command line arguments
//
//   NOTE: A service application is designed to be long running. Therefore, 
//   it usually polls or monitors something in the system. The monitoring is 
//   set up in the OnStart method. However, OnStart does not actually do the 
//   monitoring. The OnStart method must return to the operating system after 
//   the service's operation has begun. It must not loop forever or block. To 
//   set up a simple monitoring mechanism, one general solution is to create 
//   a timer in OnStart. The timer would then raise events in your code 
//   periodically, at which time your service could do its monitoring. The 
//   other solution is to spawn a new thread to perform the main service 
//   functions, which is demonstrated in this code sample.
//
void RtbcService::OnStart(DWORD dwArgc, PWSTR *lpszArgv)
{
    // Log a service start message to the Application log.
    EventLogLogger::WriteInformation(L"RtbcService in OnStart");

	if (dwArgc == 1) // No parameters, check if we already have them in Vault
	{
		ServiceCredentialsManager credMgr;
		if(!credMgr.HasCredentials())
		{
			wstring msg = L"No credentials found. Please run configuration utility and provide valid credentials."; 
			EventLogLogger::WriteError(msg);
			Stop();
			return;
		}
	}
	else if (dwArgc == 3) // Number of parameters as we expected, just need to update Vault
	{
		// First parameter must be username
		wstring userName(lpszArgv[1]);
		// Second parameter must be password
		wstring userPass(lpszArgv[2]);
		try
		{
			ServiceCredentialsManager credMgr;
			credMgr.SetCredentials(userName, userPass);	
		}
		catch(CredentialsException& ex)
		{
			string err = ex.what();
			wstring werr = StringUtil::ToWstring(err);
			EventLogLogger::WriteInformation(werr);
			Stop();
			return;
		}
	}
	else
	{
		wstring msg = str(wformat(L"Expected 1 or 3 parameters but received: %1%, stopping service.") %dwArgc); 
		EventLogLogger::WriteError(msg);
		Stop();
		return;
	}

	// Queue the main service function for execution in a worker thread.
	ThreadPool::QueueUserWorkItem(&RtbcService::ServiceWorkerThread, this);
}


//
//   FUNCTION: RtbcService::ServiceWorkerThread(void)
//
//   PURPOSE: The method performs the main function of the service. It runs 
//   on a thread pool worker thread.
//
void RtbcService::ServiceWorkerThread()
{
	// Perform main service function here...
	if (!appPtr_->Main())
	{
		return; // Something wrong
	}

	// Wait if server stop signaled
	if (WaitForSingleObject(m_hStoppedEvent, INFINITE) != WAIT_OBJECT_0)
	{
		throw GetLastError();
	}
}


//
//   FUNCTION: RtbcService::OnStop(void)
//
//   PURPOSE: The function is executed when a Stop command is sent to the 
//   service by SCM. It specifies actions to take when a service stops 
//   running. In this code sample, OnStop logs a service-stop message to the 
//   Application log, and waits for the finish of the main service function.
//
//   COMMENTS:
//   Be sure to periodically call ReportServiceStatus() with 
//   SERVICE_STOP_PENDING if the procedure is going to take long time. 
//
void RtbcService::OnStop()
{
    // Log a service stop message to the Application log.
    EventLogLogger::WriteInformation(L"RTBC service in OnStop");

	// Signal the stopped event.
	SetEvent(m_hStoppedEvent);
}
