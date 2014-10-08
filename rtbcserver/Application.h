#pragma once
#include "VosVideo.Camera/CameraDeviceManager.h"
#include "VosVideo.UserManagement/UserManager.h"
#include "VosVideo.VideoArchiveManagement/ArchiveManager.h"

class Application
{
public:
	Application(const std::vector<std::wstring>& argVect);
	~Application();

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
	std::shared_ptr<vosvideo::archive::ArchiveManager> archiveManager_;

	bool runApplication_;
	bool runAsService_;

	static std::wstring strInstall_;
	static std::wstring strUnInstall_;
	static std::wstring strService_;
	static std::wstring strDevWorker_;
	// For development purposes
	static std::wstring strUname_;
	static std::wstring strPass_;
	static std::wstring strMutexName_;
	// Wakeup timer
	Concurrency::timer<Application*>* wakeupTimer_; 
	const static int wakeupTimeout_ = 60000;
	HANDLE hHandle_;
};

