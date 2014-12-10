#include "stdafx.h"
#include <Poco/Process.h>
#include "VosVideo.Communication/InterprocessCommEngine.h"
#include "VosVideo.Communication.InterprocessQueue/InterprocessQueueEngine.h"
#include "VosVideo.Data/ShutdownCameraProcessRequestMsg.h"
#include "CameraPlayerProcess.h"

using namespace std;
using namespace vosvideo::camera;
using namespace vosvideo::communication;

CameraPlayerProcess::CameraPlayerProcess(std::shared_ptr<PubSubService> pubSubService, vosvideo::data::CameraConfMsg& conf, bool isLoggerOn) : 
	conf_(conf), pubSubService_(pubSubService), isLoggerOn_(isLoggerOn)
{
	Init();
}

CameraPlayerProcess::~CameraPlayerProcess()
{
}

void CameraPlayerProcess::Init()
{
	int cameraId; 
	wstring cameraName;
	conf_.GetCameraIds(cameraId, cameraName);
	wstring wstrCamId = to_wstring(cameraId);
	LOG_TRACE("Create camera player process for camera: " << cameraId);

	shared_ptr<InterprocessCommEngine> iqe(new InterprocessQueueEngine(pubSubService_, wstrCamId));
	duplexChannel_.reset(new InterprocessComm(iqe));
	duplexChannel_->OpenAsParent();
	// Pass first message, configuration
	duplexChannel_->Send(conf_.ToString());

	// start process and give it deice Id as starting point
	Poco::Process::Args args;
	args.push_back("-deviceid=" + to_string(cameraId));
	//args.push_back("-debug");
	if (isLoggerOn_)
	{
		args.push_back("-logging=true");
	}
	Poco::ProcessHandle ph = Poco::Process::launch("deviceworker.exe", args);
	pid_ = ph.id();
	LOG_TRACE("Camera player process for camera: " << cameraId << " started. Process Id: " << pid_);
	ReceiveAsync();
}

void CameraPlayerProcess::Reconnect()
{
	if (!IsAlive())
	{
		Shutdown();
		Init();
	}
}

bool CameraPlayerProcess::IsAlive()
{
	HANDLE process = OpenProcess(SYNCHRONIZE, FALSE, pid_);
	DWORD ret = WaitForSingleObject(process, 0);
	CloseHandle(process);
	return ret == WAIT_TIMEOUT;
}

void CameraPlayerProcess::Shutdown()
{
	vosvideo::data::ShutdownCameraProcessRequestMsg shutdownReq;
	duplexChannel_->Send(shutdownReq.ToString());
}

void CameraPlayerProcess::Send(const std::wstring& msg)
{
	duplexChannel_->Send(msg);
}

void CameraPlayerProcess::Receive()
{
	duplexChannel_->Receive();
}

void CameraPlayerProcess::ReceiveAsync()
{
	return duplexChannel_->ReceiveAsync();
}
