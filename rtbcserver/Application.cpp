#include "stdafx.h"
#include <process.h>
#include <Tlhelp32.h>
#include <ppltasks.h>
#include <boost/bind.hpp>
#include <boost/signal.hpp>
#include <talk/base/win32socketinit.h>
#include <talk/base/win32socketserver.h>
#include <vosvideocommon/StringUtil.h>

#include "VosVideo.Camera/CameraPlayer.h"
#include "VosVideo.WebRtc/PeerConnectionClientManager.h"
#include "VosVideo.Configuration/ConfigurationManager.h"
#include "VosVideo.Communication/HttpClientException.h"
#include "VosVideo.Communication/CommunicationManager.h"
#include "VosVideo.Configuration/ConfigurationParserException.h"
#include "VosVideo.Configuration/ServiceCredentialsManager.h"
#include "VosVideo.Configuration/CredentialsException.h"
#include "VosVideo.Communication.Casablanca/CbHttpClientEngine.h"
#include "VosVideo.Communication.Websocketpp/WsppWebsocketClientEngine.h"
#include "VosVideo.WebRtc/WebRtcException.h"
#include "VosVideo.DeviceManagement/DeviceConfigurationManager.h"
#include "ServiceInstaller.h"
#include "Application.h"
#include "RtbcService.h"

using namespace std;
using namespace concurrency;
using namespace boost::filesystem;
using namespace loggers;
using namespace util;

using namespace vosvideo::communication;
using namespace vosvideo::usermanagement;
using namespace vosvideo::configuration;
using namespace vosvideo::communication::casablanca;
using namespace vosvideo::communication::wspp;
using namespace vosvideo::vvwebrtc;
using namespace vosvideo::camera;
using namespace vosvideo::devicemanagement;
using namespace vosvideo::archive;

std::wstring Application::strMutexName_ = L"rtbcserver";
std::wstring Application::strInstall_   = L"-install";
std::wstring Application::strUnInstall_ = L"-uninstall";
std::wstring Application::strService_   = L"-service";
std::wstring Application::strDevWorker_ = L"deviceworker.exe";
// this parameters for development only, in real life they will never be passed
std::wstring Application::strUname_   = L"-u=";
std::wstring Application::strPass_   = L"-p=";

// Settings of the service
// Internal name of the service
#define SERVICE_NAME             L"RtbcService"
// Displayed name of the service
#define SERVICE_DISPLAY_NAME     L"Vos Video RTBC video service"
// Service start options.
#define SERVICE_START_TYPE       SERVICE_DEMAND_START
// List of service dependencies - "dep1\0dep2\0\0"
#define SERVICE_DEPENDENCIES     L""
// The name of the account under which the service should run
#define SERVICE_ACCOUNT          NULL
// The password to the service account name
#define SERVICE_PASSWORD         NULL

Application::Application(const std::vector<std::wstring>& argVect) : 
	runApplication_(true), 
	runAsService_(false)
{
	// Make initial step, change current dir to the same as executable.
	SetCurrentPath(argVect[0]);
	// Activate event source
	EventLogLogger::SetEventSource(L"RTBCServer");

	wstring userName, userPass;

	for(unsigned int i = 0 ; i < argVect.size(); i++)
	{
		if (argVect[i] == strInstall_)
		{
			runApplication_ = false;
			Install();
		}
		else if (argVect[i] == strUnInstall_)
		{
			runApplication_ = false;
			UnInstall();
		}
		else if (argVect[i] == strService_)
		{
			runAsService_ = true;
		}
		else if (argVect[i].substr(0, strUname_.length()) == strUname_)
		{
			userName = argVect[i].substr(strUname_.length(), argVect[i].length());
		}
		else if (argVect[i].substr(0, strPass_.length()) == strPass_)
		{
			userPass = argVect[i].substr(strPass_.length(), argVect[i].length());
		}
	}

	// Check if developer wanted to set credentials
	if (userName.length() > 0 && userPass.length() > 0)
	{
		ServiceCredentialsManager credMgr;
		credMgr.SetCredentials(userName, userPass);	
	}

	// Activate non-sleep mode
	auto callback = new call<Application*>([this](Application*)
	{
		// Enable away mode and prevent the sleep idle time-out.
		SetThreadExecutionState(ES_CONTINUOUS | ES_SYSTEM_REQUIRED | ES_AWAYMODE_REQUIRED);
	});
	// This timer prevents computer go to sleep
	wakeupTimer_ = new Concurrency::timer<Application*>(wakeupTimeout_, 0, callback, true);
	wakeupTimer_->start();
}


Application::~Application()
{
	wakeupTimer_->stop();
	// Clear EXECUTION_STATE flags to disable away mode and allow the system to idle to sleep normally.
	SetThreadExecutionState(ES_CONTINUOUS);

	if (!runApplication_)
	{
		return; // Nothing to stop
	}

	LOG_TRACE("Shutdown peer connections manager.");
	//if (webRtcManager_)
	//{
	//	webRtcManager_->Shutdown();
	//}

	LOG_TRACE("Shutdown runnig devices");
	if (ipDevManager_)
	{
		ipDevManager_->Shutdown();
	}

	// Stop Media foundation
	LOG_TRACE("Shutdown Media Foundation");
	MFShutdown();
	LOG_TRACE("Logging finished");
	// Finishing with COM
	CoUninitialize();
}

// Change current dir to the same as executable.
void Application::SetCurrentPath(const wstring& currPath)
{
	path p(currPath);
	path parent = p.parent_path();
	if (!parent.empty())
	{
		current_path(parent);
	}
}

void Application::Install()
{
	// Install the service when the command is "-install" 
	InstallService( 
		SERVICE_NAME,               // Name of service 
		SERVICE_DISPLAY_NAME,       // Name to display 
		SERVICE_START_TYPE,         // Service start type 
		SERVICE_DEPENDENCIES,       // Dependencies 
		SERVICE_ACCOUNT,            // Service running account 
		SERVICE_PASSWORD);          // Password of the account 
}

void Application::UnInstall()
{
	UninstallService(SERVICE_NAME);
}

bool Application::Start()
{
	// Prevent multiple copies of server to run
	HANDLE hHandle = CreateMutex( NULL, TRUE, strMutexName_.c_str());
	if( ERROR_ALREADY_EXISTS == GetLastError() )
	{
		wstring werr = L"RTBC server already running. Only one instance of server is allowed";
		EventLogLogger::WriteError(werr);
		LOG_ERROR(werr);
		return false;
	}

	KillProcessByName(strDevWorker_);
	// If service functionality requested, install service 
	// or un-install don't start server
	if (!runApplication_)
	{
		return false;
	}

	if (runAsService_) // Requested service start
	{
		RtbcService service(this, SERVICE_NAME);
		if (!ServiceBase::Run(service))
		{
			string err = "Service failed to run with err " + GetLastError();
			LOG_CRITICAL(err);
			wstring werr = L"Service failed to run";
			EventLogLogger::WriteError(werr);
			return false;
		}
		return true;
	}
	else // Requested application start
	{
		return Main();
	}
}

// Common method, executed by Application and Service
bool Application::Main()
{
	LOG_DEBUG("Rtbc server started");
	// initialize COM
	HRESULT hr = S_OK;
	if (hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED) != S_OK)
	{
		LOG_DEBUG("Rtbc server failed to start, CoInitializeEx returned: " << hr);
		return false;
	}

	// Start up Media Foundation platform.
	MFStartup(MF_VERSION);

	std::shared_ptr<ConfigurationManager> configManager;

	if (!CreateConfigurationManager(configManager))
	{
		return false;
	}

	// Activate logger 
	if (configManager->IsLoggerOn())
	{
		log_.reset(new SeverityLogger(L".", L"rtbc"));
	}
	
	std::wstring webSiteUri = configManager->GetWebSiteUri();
	if (webSiteUri.length() == 0)
	{
		string err = "Rest Service URI not found in configuration file";
		LOG_CRITICAL(err);
		wstring werr;
		StringUtil::ToWstring(err, werr);
		EventLogLogger::WriteError(werr);
		std::cin.get();
		return false;
	}

	std::shared_ptr<CbHttpClientEngine> httpClientEngine(new CbHttpClientEngine(webSiteUri));
	std::shared_ptr<HttpClient> httpClient(new HttpClient(httpClientEngine));
	std::shared_ptr<PubSubService> communicationPubSub(new PubSubService());
	std::shared_ptr<WsppWebsocketClientEngine> websocketClientEngine(new WsppWebsocketClientEngine(communicationPubSub));
	std::shared_ptr<WebsocketClient> websocketClient(new WebsocketClient(websocketClientEngine));
	std::shared_ptr<CommunicationManager> commManager = std::shared_ptr<CommunicationManager>(new CommunicationManager(httpClient, websocketClient));	 	

	// Now we need to speak to services to take user properties and configuration
	std::shared_ptr<vosvideo::usermanagement::UserManager> userManager(new UserManager(commManager, configManager, communicationPubSub));

	if (!WebSocketServerLogin(commManager, userManager))
	{
		return false;
	}

	// Now get all known cameras description
	wstring accountId = userManager->GetAccountId();
	wstring siteId = configManager->GetSiteId();
	std::shared_ptr<DeviceConfigurationManager> devManager(new DeviceConfigurationManager(commManager, communicationPubSub, accountId, siteId));
	ipDevManager_.reset(new CameraDeviceManager(commManager, devManager, communicationPubSub, userManager, configManager));
	try
	{
		auto devRespAsync = devManager->RequestDeviceConfigurationAsync();
		devRespAsync.wait();
	}
	catch(exception& e)
	{
		LOG_CRITICAL(e.what());
		wstring werr;
		StringUtil::ToWstring(e.what(), werr);
		EventLogLogger::WriteError(werr);
		std::cin.get();
		return false;
	}

	archiveManager_.reset(new ArchiveManager(configManager, communicationPubSub));	 	

	LOG_TRACE("RTBC server is ready.");
	return true;
}

bool Application::CreateConfigurationManager(std::shared_ptr<ConfigurationManager>& configManager)
{
	try
	{
		configManager.reset(new ConfigurationManager());
	}
	catch(ConfigurationParserException& e)
	{
		LOG_CRITICAL(e.what());
		wstring werr;
		StringUtil::ToWstring(e.what(), werr);
		EventLogLogger::WriteError(werr);
		std::cin.get();
		return false;
	}

	return true;
}

bool Application::WebSocketServerLogin(shared_ptr<CommunicationManager> commManager,
									   shared_ptr<UserManager> userManager)
{
	LogInRequest logInRequest;
	if (!CreateLoginRequest(logInRequest))
	{
		return false;
	}

	try
	{
		auto userRespAsync = userManager->LogInAsync(logInRequest);

		auto userContinuation = userRespAsync.then([&](LogInResponse& resp)
		{
			commManager->WebsocketSend("RTBC is Ready");
			LOG_TRACE("RTBC successfully logged to WebSocket server.");
		});
		userContinuation.wait();
	}
	catch(exception& e)
	{
		LOG_CRITICAL(e.what());
		wstring werr;
		StringUtil::ToWstring(e.what(), werr);
		EventLogLogger::WriteError(werr);
		std::cin.get();
		return false;
	}
	return true;
}

bool Application::CreateLoginRequest(LogInRequest& logInRequest)
{
	wstring userName;
	wstring userPass;
	try
	{
		ServiceCredentialsManager credMgr;
		credMgr.GetCredentials(userName, userPass);	
		LogInRequest tmpLogInRequest(userName, userPass);
		logInRequest = tmpLogInRequest;
	}
	catch(CredentialsException ex)
	{
		string err = ex.what();
		wstring werr;
		StringUtil::ToWstring(err, werr);
		EventLogLogger::WriteInformation(werr);
		std::cin.get();
		return false;
	}

	return true;
}

void Application::Wait()
{
	if (!runAsService_)
	{
		for(;;)
		{
			string resp;
			cin >> resp;
			if (resp == "stop")
			{
				break;
			}
		}
	}
}

void Application::KillProcessByName(const wstring& filename)
{
	HANDLE hSnapShot = CreateToolhelp32Snapshot(TH32CS_SNAPALL, NULL);
	PROCESSENTRY32 pEntry;
	pEntry.dwSize = sizeof (pEntry);
	BOOL hRes = Process32First(hSnapShot, &pEntry);
	while (hRes)
	{
		wstring found(pEntry.szExeFile);

		if (found == filename)
		{
			HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, 0,
				(DWORD) pEntry.th32ProcessID);
			if (hProcess != NULL)
			{
				TerminateProcess(hProcess, 9);
				CloseHandle(hProcess);
			}
		}
		hRes = Process32Next(hSnapShot, &pEntry);
	}
	CloseHandle(hSnapShot);
}