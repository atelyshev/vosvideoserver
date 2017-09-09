#include "stdafx.h"
#include <boost/interprocess/ipc/message_queue.hpp>

#include "VosVideo.Communication/PubSubService.h"
#include "VosVideo.Communication.InterprocessQueue/InterprocessQueueEngine.h"
#include "VosVideo.Camera/CameraPlayerFactory.h"

#include "DeviceWorkerApp.h"

using namespace std;
using namespace util;
using namespace loggers;
using namespace vosvideo::communication;
using namespace vosvideo::vvwebrtc;
using namespace vosvideo::camera;

DeviceWorkerApp::DeviceWorkerApp(const wstring& wqueueName, bool isLogging)
{	
	if (isLogging)
	{
		wstring prefix = L"deviceworker_" + wqueueName;
		log_.reset(new SeverityLogger(L".", prefix));
	}

	std::shared_ptr<PubSubService> communicationPubSub(new PubSubService());
	std::shared_ptr<InterprocessQueueEngine> queueEngine(new InterprocessQueueEngine(communicationPubSub, wqueueName));
	devBroker_.reset(new WebRtcManager(communicationPubSub, queueEngine));
	interprocCommManager_.reset(new InterprocessComm(queueEngine));

	HRESULT hr = S_OK;
	if (hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED) != S_OK)
	{
		string errMsg = "DeviceWorker server failed to start, CoInitializeEx returned: " + hr;
		LOG_CRITICAL(errMsg);
		throw runtime_error(errMsg);
	}
	CameraPlayerFactory::Init(nullptr, nullptr);
}


DeviceWorkerApp::~DeviceWorkerApp()
{
	CameraPlayerFactory::Shutdown();
	CoUninitialize();
}

bool DeviceWorkerApp::Start()
{
	try
	{
		interprocCommManager_->OpenAsChild();
	}
	catch(exception &ex)
	{
		LOG_CRITICAL(ex.what());
		return false;
	}
	interprocCommManager_->Receive();
	return true;
}