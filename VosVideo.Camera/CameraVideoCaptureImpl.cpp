#include "stdafx.h"
#include "VosVideo.CameraPlayer/CameraPlayerBase.h"
#include "CameraVideoCaptureImpl.h"

using namespace webrtc;
using vosvideo::camera::CameraVideoCaptureImpl;
using vosvideo::cameraplayer::CameraPlayerBase;


CameraVideoCaptureImpl::CameraVideoCaptureImpl(CameraPlayerBase* player) : 
	webrtc::videocapturemodule::VideoCaptureImpl(),
	player_(player),
	startedCapture_(false)
{
}

int32_t CameraVideoCaptureImpl::AddRef() const
{
	return ++ref_count_;
}

int32_t CameraVideoCaptureImpl::Release() const
{
	int count = --ref_count_;
	if (!count)
	{
		delete this;
	}
	return count;
}

CameraVideoCaptureImpl::~CameraVideoCaptureImpl()
{
}

int32_t CameraVideoCaptureImpl::StartCapture(const webrtc::VideoCaptureCapability& capability)
{	
	startedCapture_ = true;
	player_->SetExternalCapturer(this);
	return 0;
}

int32_t CameraVideoCaptureImpl::StopCapture()
{
	startedCapture_ = false;
	// We are done, stop capturer
	player_->RemoveExternalCapturer(this);
	return 0;
}

bool CameraVideoCaptureImpl::CaptureStarted()
{
	return startedCapture_;
}

int32_t CameraVideoCaptureImpl::CaptureSettings(webrtc::VideoCaptureCapability& settings)
{
	settings = _requestedCapability;
	return 0;
}

