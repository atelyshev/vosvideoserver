#include "stdafx.h"
#include "CameraVideoCaptureImpl.h"
#include "VosVideo.CameraPlayer/CameraPlayerBase.h"
#include "ref_count.h"
#include <Unknwn.h>

using namespace webrtc;

using vosvideo::camera::CameraVideoCaptureImpl;
using vosvideo::cameraplayer::CameraPlayerBase;


VideoCaptureModule* CameraVideoCaptureImpl::Create(const int32_t id, VideoCaptureExternal*& externalCapture, vosvideo::cameraplayer::CameraPlayerBase* player)
{
	RefCountImpl<CameraVideoCaptureImpl>* implementation = new RefCountImpl<CameraVideoCaptureImpl>(id, player);
	externalCapture = implementation;
	return implementation;
}

CameraVideoCaptureImpl::CameraVideoCaptureImpl(const int32_t id, CameraPlayerBase* player) : 
	webrtc::videocapturemodule::VideoCaptureImpl(id),
	startedCapture_(false),
	player_(player)
{
	//Check if it derives from IUnknown
	IUnknown* iUnknownPlayer = dynamic_cast<IUnknown*>(player_);
	if(iUnknownPlayer)
		iUnknownPlayer->AddRef();
}


CameraVideoCaptureImpl::~CameraVideoCaptureImpl()
{
	//Check if it derives from IUnknown
	IUnknown* iUnknownPlayer = dynamic_cast<IUnknown*>(player_);
	if(iUnknownPlayer)
		iUnknownPlayer->Release();
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

