/****************************** Module Header ******************************\
* Module Name:  ServiceBase.cpp
* Project:      CppWindowsService
* Copyright (c) Microsoft Corporation.
* 
* Provides a base class for a service that will exist as part of a service 
* application. ServiceBase must be derived from when creating a new service 
* class.
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
#include <assert.h>
#include <strsafe.h>
#include "ServiceBase.h"
#pragma endregion

using namespace loggers;

#pragma region Static Members

// Initialize the singleton service instance.
ServiceBase *ServiceBase::service_ = nullptr;


//
//   FUNCTION: ServiceBase::Run(ServiceBase &)
//
//   PURPOSE: Register the executable for a service with the Service Control 
//   Manager (SCM). After you call Run(ServiceBase), the SCM issues a Start 
//   command, which results in a call to the OnStart method in the service. 
//   This method blocks until the service has stopped.
//
//   PARAMETERS:
//   * service - the reference to a ServiceBase object. It will become the 
//     singleton service instance of this service application.
//
//   RETURN VALUE: If the function succeeds, the return value is TRUE. If the 
//   function fails, the return value is FALSE. To get extended error 
//   information, call GetLastError.
//
BOOL ServiceBase::Run(ServiceBase &service)
{
	service_ = &service;

	SERVICE_TABLE_ENTRY serviceTable[] = 
	{
		{ service.m_name, ServiceMain },
		{ NULL, NULL }
	};

	// Connects the main thread of a service process to the service control 
	// manager, which causes the thread to be the service control dispatcher 
	// thread for the calling process. This call returns when the service has 
	// stopped. The process should simply terminate when the call returns.
	return StartServiceCtrlDispatcher(serviceTable);
}


//
//   FUNCTION: ServiceBase::ServiceMain(DWORD, PWSTR *)
//
//   PURPOSE: Entry point for the service. It registers the handler function 
//   for the service and starts the service.
//
//   PARAMETERS:
//   * dwArgc   - number of command line arguments
//   * lpszArgv - array of command line arguments
//
void WINAPI ServiceBase::ServiceMain(DWORD dwArgc, PWSTR *pszArgv)
{
	assert(service_ != nullptr);

	// Register the handler function for the service
	service_->statusHandle_ = RegisterServiceCtrlHandler(
		service_->m_name, ServiceCtrlHandler);
	if (service_->statusHandle_ == nullptr)
	{
		throw GetLastError();
	}

	// Start the service.
	service_->Start(dwArgc, pszArgv);
}


//
//   FUNCTION: ServiceBase::ServiceCtrlHandler(DWORD)
//
//   PURPOSE: The function is called by the SCM whenever a control code is 
//   sent to the service. 
//
//   PARAMETERS:
//   * dwCtrlCode - the control code. This parameter can be one of the 
//   following values: 
//
//     SERVICE_CONTROL_CONTINUE
//     SERVICE_CONTROL_INTERROGATE
//     SERVICE_CONTROL_NETBINDADD
//     SERVICE_CONTROL_NETBINDDISABLE
//     SERVICE_CONTROL_NETBINDREMOVE
//     SERVICE_CONTROL_PARAMCHANGE
//     SERVICE_CONTROL_PAUSE
//     SERVICE_CONTROL_SHUTDOWN
//     SERVICE_CONTROL_STOP
//
//   This parameter can also be a user-defined control code ranges from 128 
//   to 255.
//
void WINAPI ServiceBase::ServiceCtrlHandler(DWORD dwCtrl)
{
	switch (dwCtrl)
	{
	case SERVICE_CONTROL_STOP: service_->Stop(); break;
	case SERVICE_CONTROL_PAUSE: service_->Pause(); break;
	case SERVICE_CONTROL_CONTINUE: service_->Continue(); break;
	case SERVICE_CONTROL_SHUTDOWN: service_->Shutdown(); break;
	case SERVICE_CONTROL_INTERROGATE: break;
	default: break;
	}
}

#pragma endregion


#pragma region Service Constructor and Destructor

//
//   FUNCTION: ServiceBase::ServiceBase(PWSTR, BOOL, BOOL, BOOL)
//
//   PURPOSE: The constructor of ServiceBase. It initializes a new instance 
//   of the ServiceBase class. The optional parameters (fCanStop, 
///  fCanShutdown and fCanPauseContinue) allow you to specify whether the 
//   service can be stopped, paused and continued, or be notified when system 
//   shutdown occurs.
//
//   PARAMETERS:
//   * pszServiceName - the name of the service
//   * fCanStop - the service can be stopped
//   * fCanShutdown - the service is notified when system shutdown occurs
//   * fCanPauseContinue - the service can be paused and continued
//
ServiceBase::ServiceBase(PWSTR pszServiceName, 
						 BOOL fCanStop, 
						 BOOL fCanShutdown, 
						 BOOL fCanPauseContinue)
{
	// Service name must be a valid string and cannot be NULL.
	m_name = (pszServiceName == nullptr) ? L"" : pszServiceName;

	statusHandle_ = nullptr;

	// The service runs in its own process.
	status_.dwServiceType = SERVICE_WIN32_OWN_PROCESS;

	// The service is starting.
	status_.dwCurrentState = SERVICE_START_PENDING;

	// The accepted commands of the service.
	DWORD dwControlsAccepted = 0;
	if (fCanStop) 
		dwControlsAccepted |= SERVICE_ACCEPT_STOP;
	if (fCanShutdown) 
		dwControlsAccepted |= SERVICE_ACCEPT_SHUTDOWN;
	if (fCanPauseContinue) 
		dwControlsAccepted |= SERVICE_ACCEPT_PAUSE_CONTINUE;
	status_.dwControlsAccepted = dwControlsAccepted;

	status_.dwWin32ExitCode = NO_ERROR;
	status_.dwServiceSpecificExitCode = 0;
	status_.dwCheckPoint = 0;
	status_.dwWaitHint = 0;
}


//
//   FUNCTION: ServiceBase::~ServiceBase()
//
//   PURPOSE: The virtual destructor of ServiceBase. 
//
ServiceBase::~ServiceBase(void)
{
}

#pragma endregion


#pragma region Service Start, Stop, Pause, Continue, and Shutdown

//
//   FUNCTION: ServiceBase::Start(DWORD, PWSTR *)
//
//   PURPOSE: The function starts the service. It calls the OnStart virtual 
//   function in which you can specify the actions to take when the service 
//   starts. If an error occurs during the startup, the error will be logged 
//   in the Application event log, and the service will be stopped.
//
//   PARAMETERS:
//   * dwArgc   - number of command line arguments
//   * lpszArgv - array of command line arguments
//
void ServiceBase::Start(DWORD dwArgc, PWSTR *pszArgv)
{
	try
	{
		// Tell SCM that the service is starting.
		SetServiceStatus(SERVICE_START_PENDING);

		// Perform service-specific initialization.
		OnStart(dwArgc, pszArgv);

		// Tell SCM that the service is started.
		SetServiceStatus(SERVICE_RUNNING);
	}
	catch (DWORD dwError)
	{
		// Log the error.
		EventLogLogger::WriteError(L"Service Start", dwError);

		// Set the service status to be stopped.
		SetServiceStatus(SERVICE_STOPPED, dwError);
	}
	catch (...)
	{
		// Log the error.
		EventLogLogger::WriteError(L"Service failed to start.");

		// Set the service status to be stopped.
		SetServiceStatus(SERVICE_STOPPED);
	}
}


//
//   FUNCTION: ServiceBase::OnStart(DWORD, PWSTR *)
//
//   PURPOSE: When implemented in a derived class, executes when a Start 
//   command is sent to the service by the SCM or when the operating system 
//   starts (for a service that starts automatically). Specifies actions to 
//   take when the service starts. Be sure to periodically call 
//   ServiceBase::SetServiceStatus() with SERVICE_START_PENDING if the 
//   procedure is going to take long time. You may also consider spawning a 
//   new thread in OnStart to perform time-consuming initialization tasks.
//
//   PARAMETERS:
//   * dwArgc   - number of command line arguments
//   * lpszArgv - array of command line arguments
//
void ServiceBase::OnStart(DWORD dwArgc, PWSTR *pszArgv)
{
}


//
//   FUNCTION: ServiceBase::Stop()
//
//   PURPOSE: The function stops the service. It calls the OnStop virtual 
//   function in which you can specify the actions to take when the service 
//   stops. If an error occurs, the error will be logged in the Application 
//   event log, and the service will be restored to the original state.
//
void ServiceBase::Stop()
{
	DWORD dwOriginalState = status_.dwCurrentState;
	try
	{
		// Tell SCM that the service is stopping.
		SetServiceStatus(SERVICE_STOP_PENDING);

		// Perform service-specific stop operations.
		OnStop();

		// Tell SCM that the service is stopped.
		SetServiceStatus(SERVICE_STOPPED);
	}
	catch (DWORD dwError)
	{
		// Log the error.
		EventLogLogger::WriteError(L"Service Stop", dwError);

		// Set the orginal service status.
		SetServiceStatus(dwOriginalState);
	}
	catch (...)
	{
		// Log the error.
		EventLogLogger::WriteError(L"Service failed to stop.");

		// Set the orginal service status.
		SetServiceStatus(dwOriginalState);
	}
}


//
//   FUNCTION: ServiceBase::OnStop()
//
//   PURPOSE: When implemented in a derived class, executes when a Stop 
//   command is sent to the service by the SCM. Specifies actions to take 
//   when a service stops running. Be sure to periodically call 
//   ServiceBase::SetServiceStatus() with SERVICE_STOP_PENDING if the 
//   procedure is going to take long time. 
//
void ServiceBase::OnStop()
{
}


//
//   FUNCTION: ServiceBase::Pause()
//
//   PURPOSE: The function pauses the service if the service supports pause 
//   and continue. It calls the OnPause virtual function in which you can 
//   specify the actions to take when the service pauses. If an error occurs, 
//   the error will be logged in the Application event log, and the service 
//   will become running.
//
void ServiceBase::Pause()
{
	try
	{
		// Tell SCM that the service is pausing.
		SetServiceStatus(SERVICE_PAUSE_PENDING);

		// Perform service-specific pause operations.
		OnPause();

		// Tell SCM that the service is paused.
		SetServiceStatus(SERVICE_PAUSED);
	}
	catch (DWORD dwError)
	{
		// Log the error.
		EventLogLogger::WriteError(L"Service Pause", dwError);

		// Tell SCM that the service is still running.
		SetServiceStatus(SERVICE_RUNNING);
	}
	catch (...)
	{
		// Log the error.
		EventLogLogger::WriteError(L"Service failed to pause.");

		// Tell SCM that the service is still running.
		SetServiceStatus(SERVICE_RUNNING);
	}
}


//
//   FUNCTION: ServiceBase::OnPause()
//
//   PURPOSE: When implemented in a derived class, executes when a Pause 
//   command is sent to the service by the SCM. Specifies actions to take 
//   when a service pauses.
//
void ServiceBase::OnPause()
{
}


//
//   FUNCTION: ServiceBase::Continue()
//
//   PURPOSE: The function resumes normal functioning after being paused if
//   the service supports pause and continue. It calls the OnContinue virtual 
//   function in which you can specify the actions to take when the service 
//   continues. If an error occurs, the error will be logged in the 
//   Application event log, and the service will still be paused.
//
void ServiceBase::Continue()
{
	try
	{
		// Tell SCM that the service is resuming.
		SetServiceStatus(SERVICE_CONTINUE_PENDING);

		// Perform service-specific continue operations.
		OnContinue();

		// Tell SCM that the service is running.
		SetServiceStatus(SERVICE_RUNNING);
	}
	catch (DWORD dwError)
	{
		// Log the error.
		EventLogLogger::WriteError(L"Service Continue", dwError);

		// Tell SCM that the service is still paused.
		SetServiceStatus(SERVICE_PAUSED);
	}
	catch (...)
	{
		// Log the error.
		EventLogLogger::WriteError(L"Service failed to resume.");

		// Tell SCM that the service is still paused.
		SetServiceStatus(SERVICE_PAUSED);
	}
}


//
//   FUNCTION: ServiceBase::OnContinue()
//
//   PURPOSE: When implemented in a derived class, OnContinue runs when a 
//   Continue command is sent to the service by the SCM. Specifies actions to 
//   take when a service resumes normal functioning after being paused.
//
void ServiceBase::OnContinue()
{
}


//
//   FUNCTION: ServiceBase::Shutdown()
//
//   PURPOSE: The function executes when the system is shutting down. It 
//   calls the OnShutdown virtual function in which you can specify what 
//   should occur immediately prior to the system shutting down. If an error 
//   occurs, the error will be logged in the Application event log.
//
void ServiceBase::Shutdown()
{
	try
	{
		// Perform service-specific shutdown operations.
		OnShutdown();

		// Tell SCM that the service is stopped.
		SetServiceStatus(SERVICE_STOPPED);
	}
	catch (DWORD dwError)
	{
		// Log the error.
		EventLogLogger::WriteError(L"Service Shutdown", dwError);
	}
	catch (...)
	{
		// Log the error.
		EventLogLogger::WriteError(L"Service failed to shut down.");
	}
}


//
//   FUNCTION: ServiceBase::OnShutdown()
//
//   PURPOSE: When implemented in a derived class, executes when the system 
//   is shutting down. Specifies what should occur immediately prior to the 
//   system shutting down.
//
void ServiceBase::OnShutdown()
{
}

#pragma endregion


#pragma region Helper Functions

//
//   FUNCTION: ServiceBase::SetServiceStatus(DWORD, DWORD, DWORD)
//
//   PURPOSE: The function sets the service status and reports the status to 
//   the SCM.
//
//   PARAMETERS:
//   * dwCurrentState - the state of the service
//   * dwWin32ExitCode - error code to report
//   * dwWaitHint - estimated time for pending operation, in milliseconds
//
void ServiceBase::SetServiceStatus(DWORD dwCurrentState, 
								   DWORD dwWin32ExitCode, 
								   DWORD dwWaitHint)
{
	static DWORD dwCheckPoint = 1;

	// Fill in the SERVICE_STATUS structure of the service.

	status_.dwCurrentState = dwCurrentState;
	status_.dwWin32ExitCode = dwWin32ExitCode;
	status_.dwWaitHint = dwWaitHint;

	status_.dwCheckPoint = 
		((dwCurrentState == SERVICE_RUNNING) ||
		(dwCurrentState == SERVICE_STOPPED)) ? 
		0 : dwCheckPoint++;

	// Report the status of the service to the SCM.
	::SetServiceStatus(statusHandle_, &status_);
}

#pragma endregion