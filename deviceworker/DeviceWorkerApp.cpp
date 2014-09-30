#include "stdafx.h"
#include <boost/interprocess/ipc/message_queue.hpp>
#include <VosVideoCommon/StringUtil.h>
#include "VosVideo.Communication/PubSubService.h"
#include "VosVideo.Communication.InterprocessQueue/InterprocessQueueEngine.h"
#include "DeviceWorkerApp.h"

using namespace std;
using namespace util;
using namespace loggers;
using namespace vosvideo::communication;
using namespace vosvideo::vvwebrtc;

DeviceWorkerApp::DeviceWorkerApp(const wstring& wqueueName, bool isLogging)
{
	if (isLogging)
	{
		log_.reset(new SeverityLogger(L".", wqueueName));
	}

	std::shared_ptr<PubSubService> communicationPubSub(new PubSubService());
	std::shared_ptr<InterprocessQueueEngine> queueEngine(new InterprocessQueueEngine(communicationPubSub, wqueueName));
	devBroker_.reset(new WebRtcDeviceBroker(communicationPubSub, queueEngine));
	interprocCommManager_.reset(new InterprocessComm(queueEngine));

	HRESULT hr = S_OK;
	if (hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED) != S_OK)
	{
		string errMsg = "DeviceWorker server failed to start, CoInitializeEx returned: " + hr;
		LOG_CRITICAL(errMsg);
		throw runtime_error(errMsg);
	}
	MFStartup(MF_VERSION);
}


DeviceWorkerApp::~DeviceWorkerApp()
{
	MFShutdown();
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