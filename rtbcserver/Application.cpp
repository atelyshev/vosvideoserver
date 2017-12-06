#include "stdafx.h"
#include <process.h>
#include <Tlhelp32.h>
#include <ppltasks.h>
#include <boost/bind.hpp>
#include <boost/signals2.hpp>
#include <webrtc/base/win32socketinit.h>
#include <webrtc/base/win32socketserver.h>

#include "VosVideo.WebRtc/PeerConnectionClientManager.h"
#include "VosVideo.Configuration/ConfigurationManager.h"
#include "VosVideo.Communication/HttpClientException.h"
#include "VosVideo.Communication/CommunicationManager.h"
#include "VosVideo.Configuration/ConfigurationParserException.h"
#include "VosVideo.Configuration/ServiceCredentialsManager.h"
#include "VosVideo.Configuration/CredentialsException.h"
#include "VosVideo.Communication.Casablanca/CbHttpClientEngine.h"
#include "VosVideo.Communication.Casablanca/CbWebsocketClientEngine.h"
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
using namespace vosvideo::vvwebrtc;
using namespace vosvideo::camera;
using namespace vosvideo::devicemanagement;
using namespace vosvideo::archive;

// Settings of the service
// Internal name of the service
#define SERVICE_NAME             L"RtbcService"
// Displayed name of the service
#define SERVICE_DISPLAY_NAME     L"Vos Video media service"
// Service start options.
#define SERVICE_START_TYPE       SERVICE_DEMAND_START
// List of service dependencies - "dep1\0dep2\0\0"
#define SERVICE_DEPENDENCIES     L""
// The name of the account under which the service should run
#define SERVICE_ACCOUNT          nullptr
// The password to the service account name
#define SERVICE_PASSWORD         nullptr

Application::Application(const std::vector<std::wstring>& argVect) : 
	runApplication_(true), 
	runAsService_(false)
{
	// Make initial step, change current dir to the same as executable.
	SetCurrentPath(argVect[0]);
	// Activate event source
	EventLogLogger::SetEventSource(L"RTBCServer");

	wstring userName, userPass;

	for(const auto& arg : argVect)
	{
		if (arg == strInstall_)
		{
			runApplication_ = false;
			Install();
		}
		else if (arg == strUnInstall_)
		{
			runApplication_ = false;
			UnInstall();
		}
		else if (arg == strService_)
		{
			runAsService_ = true;
		}
		else if (arg.substr(0, strUname_.length()) == strUname_)
		{
			userName = arg.substr(strUname_.length(), arg.length());
		}
		else if (arg.substr(0, strPass_.length()) == strPass_)
		{
			userPass = arg.substr(strPass_.length(), arg.length());
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

	LOG_TRACE("Shutdown runnig devices");
	if (ipDevManager_)
	{
		ipDevManager_->Shutdown();
	}

	CloseHandle(hHandle_);
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
	hHandle_ = CreateMutex(nullptr, TRUE, strMutexName_.c_str());
	if( ERROR_ALREADY_EXISTS == GetLastError() )
	{
		wstring werr = L"RTBC server already running. Only one instance of server is allowed";
		EventLogLogger::WriteError(werr);
		LOG_ERROR(StringUtil::ToString(werr));
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
	if (hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED) != S_OK)
	{
		LOG_DEBUG("Rtbc server failed to start, CoInitializeEx returned: " << hr);
		return false;
	}

	// Initialize GStreamer 
	gst_init(nullptr, nullptr);

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
	
	std::wstring restServiceUri = configManager->GetRestServiceUri();
	if (restServiceUri.length() == 0)
	{
		string err = "Rest Service URI not found in configuration file";
		LOG_CRITICAL(err);
		wstring werr = StringUtil::ToWstring(err);
		EventLogLogger::WriteError(werr);
		std::cin.get();
		return false;
	}

	std::shared_ptr<CbHttpClientEngine> httpClientEngine(new CbHttpClientEngine(restServiceUri));
	std::shared_ptr<HttpClient> httpClient(new HttpClient(httpClientEngine));
	std::shared_ptr<PubSubService> communicationPubSub(new PubSubService());
	std::shared_ptr<CbWebsocketClientEngine> websocketClientEngine(new CbWebsocketClientEngine(communicationPubSub));
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
		wstring werr = StringUtil::ToWstring(e.what());
		EventLogLogger::WriteError(werr);
		std::cin.get();
		return false;
	}

	archiveManager_.reset(new MediaWatcher(configManager, communicationPubSub));	 	

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
		wstring werr = StringUtil::ToWstring(e.what());
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
		userRespAsync.then([=](task<LogInResponse> end_task)
		{
			try
			{
				end_task.get();
			}
			catch (...)
			{
				throw;
			}
		}).wait();			
	}
	catch(exception& e)
	{
		LOG_CRITICAL(e.what());
		wstring werr = StringUtil::ToWstring(e.what());
		EventLogLogger::WriteError(werr);
		std::cin.get();
		return false;
	}
	LOG_TRACE("RTBC successfully logged to WebSocket server.");
	return true;
}

bool Application::CreateLoginRequest(LogInRequest& logInRequest)
{
	wstring userName;
	wstring userPass;
	try
	{
		ServiceCredentialsManager credMgr;
		tie(userName, userPass) = credMgr.GetCredentials();
		LogInRequest tmpLogInRequest(userName, userPass);
		logInRequest = tmpLogInRequest;
	}
	catch(CredentialsException& ex)
	{
		string err = ex.what();
		wstring werr = StringUtil::ToWstring(err);
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
	HANDLE hSnapShot = CreateToolhelp32Snapshot(TH32CS_SNAPALL, 0);
	PROCESSENTRY32 pEntry;
	pEntry.dwSize = sizeof (pEntry);
	int32_t hRes = Process32First(hSnapShot, &pEntry);
	while (hRes)
	{
		wstring found(pEntry.szExeFile);

		if (found == filename)
		{
			HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, 0,
				(uint32_t) pEntry.th32ProcessID);
			if (hProcess != nullptr)
			{
				TerminateProcess(hProcess, 9);
				CloseHandle(hProcess);
			}
		}
		hRes = Process32Next(hSnapShot, &pEntry);
	}
	CloseHandle(hSnapShot);
}