#pragma once
#include "VosVideo.Common/StdLogger.h"
#include "VosVideo.Communication/InterprocessComm.h"
#include "VosVideo.WebRtc/WebRtcManager.h"

class DeviceWorkerApp
{
public:
	DeviceWorkerApp(const std::wstring& wqueueName, bool isLogging);
	virtual ~DeviceWorkerApp();

	bool Start();

private:

	std::shared_ptr<vosvideo::communication::InterprocessComm> interprocCommManager_;
	std::shared_ptr<loggers::SeverityLogger> _log;
	std::unique_ptr<loggers::StdLogger> _stdlog;
	std::string queueName_;
	std::shared_ptr<vosvideo::vvwebrtc::WebRtcManager> devBroker_;
};

