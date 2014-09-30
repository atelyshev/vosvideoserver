#pragma once
#include "VosVideo.Communication/InterprocessComm.h"
#include "VosVideo.WebRtc/WebRtcDeviceBroker.h"

class DeviceWorkerApp
{
public:
	DeviceWorkerApp(const std::wstring& wqueueName, bool isLogging);
	~DeviceWorkerApp();

	bool Start();

private:

	std::shared_ptr<vosvideo::communication::InterprocessComm> interprocCommManager_;
	std::shared_ptr<loggers::SeverityLogger> log_;
	std::string queueName_;
	std::shared_ptr<vosvideo::vvwebrtc::WebRtcDeviceBroker> devBroker_;
};

