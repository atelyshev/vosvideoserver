#pragma once
#include "VosVideo.Camera/CameraDeviceManager.h"
#include "VosVideo.UserManagement/UserManager.h"
#include "VosVideo.MediaManagement/MediaWatcher.h"

class Application
{
public:
	Application(const std::vector<std::wstring>& argVect);
	virtual ~Application();

	bool Start();
	bool Main();
	void Wait();

private:
	void Install();
	void UnInstall();
	void SetCurrentPath(const std::wstring& currPath);
	bool CreateLoginRequest(vosvideo::usermanagement::LogInRequest& logInRequest);
	bool CreateConfigurationManager(std::shared_ptr<vosvideo::configuration::ConfigurationManager>& configManager);
	bool WebSocketServerLogin(std::shared_ptr<vosvideo::communication::CommunicationManager> commManager,
		                      std::shared_ptr<vosvideo::usermanagement::UserManager> userManager);
	void KillProcessByName(const std::wstring& filename);

	std::shared_ptr<loggers::SeverityLogger> log_;
	std::shared_ptr<vosvideo::camera::CameraDeviceManager> ipDevManager_;
	std::shared_ptr<vosvideo::archive::MediaWatcher> archiveManager_;

	bool runApplication_;
	bool runAsService_;

	const std::wstring strInstall_   = L"-install";
	const std::wstring strUnInstall_ = L"-uninstall";
	const std::wstring strService_   = L"-service";
	const std::wstring strDevWorker_ = L"deviceworker.exe";
	// For development purposes
	const std::wstring strUname_     = L"-u=";
	const std::wstring strPass_      = L"-p=";
	const std::wstring strMutexName_ = L"rtbcserver";
	// Wakeup timer
	Concurrency::timer<Application*>* wakeupTimer_ = nullptr; 
	static const int wakeupTimeout_ = 60000;
	HANDLE hHandle_;
};

