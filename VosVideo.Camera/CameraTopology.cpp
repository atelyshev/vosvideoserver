#include "stdafx.h"

#include <webm/include/webmtypes.h>
#include <VP8CallbackSink/VP8CallbackSinkGuid.h>
#include <VP8FileSink/VP8FileSinkGuid.h>
#include <VP8FileSink/VP8FileSink.h>
#include <VP8Encoder/VP8EncoderGuid.h>
#include <vosvideocommon/StringUtil.h>
#include <vosvideocommon/NativeErrorsManager.h>
#include <vosvideocommon/ComHelper.h>
#include "NetCredentialManager.h"
#include "MfTopologyHelper.h"
#include "CameraTopology.h"

using namespace std;
using namespace util;
using namespace concurrency;
using namespace vosvideo::camera;
using namespace vosvideo::data;

FrameCallback::FrameCallback(CameraTopology* topoPtr) : topoPtr_(topoPtr)
{
}

void FrameCallback::IncomingVp8Frame(vector<byte>& vFrame, vector<int>& vCapabilities)
{
	ATLTRACE("Frame size %d bytes, Is key frame=%d\n", vFrame.size(), vCapabilities[7]);
	topoPtr_->VP8BufCallback(vFrame, vCapabilities);
}

void CameraTopology::VP8BufCallback(std::vector<unsigned char>& vFrame, std::vector<int>& vCapabilities)
{
	webrtc::VideoCaptureCapability cap;
	// param - capabilities 
	// width - int
	// height - int
	// maxFPS - int
	// expectedCaptureDelay - int
	// RawVideoType = 99 /*unknown*/
	// VideoCodecType = 0 /*VP8*/
	// interlaced - BOOL
	int i = 0;
	cap.width = vCapabilities[i++];
	cap.height = vCapabilities[i++];
	cap.maxFPS = vCapabilities[i++];
	cap.expectedCaptureDelay = vCapabilities[i++];
	cap.rawType = webrtc::RawVideoType::kVideoI420;//vCapabilities[i++];
	cap.codecType = webrtc::VideoCodecType::kVideoCodecUnknown;//)vCapabilities[i++];
	cap.interlaced = vCapabilities[i++] ? true:false;
	vFrame.resize((cap.width * cap.height * 3)/2);

	lock_guard<std::mutex> lock(mutex_);

	for(auto iter = captureObservers_.begin(); iter != captureObservers_.end(); ++iter)
	{
		// Regarding API buffer should be exact size of raw image, have to resize it here
		vFrame.resize((cap.width * cap.height * 3)/2);
		iter->second->IncomingFrame(&vFrame[0], vFrame.size(), cap);
	}
}

CameraTopology::CameraTopology() : 
	newFrameCallback_(nullptr), 
	fireCancelTimer_(nullptr),
	nRefCount_(1)
{
	invokeCompleteEvent_ = CreateEvent(NULL, FALSE, FALSE, NULL);
}


CameraTopology::~CameraTopology()
{
	// have to stop because destructor can be called while timer waiting
	if (fireCancelTimer_ != nullptr)
	{
		fireCancelTimer_->stop();
		delete fireCancelTimer_;
	}

	// Now prevent Invoke to be called
	if (pSource_ == nullptr) 
	{
		if (pSourceResolver_!= nullptr)
		{
			pSourceResolver_->CancelObjectCreation(pCancelCookie_);
			WaitForSingleObject(invokeCompleteEvent_, 15000);
		}
	}

	CloseHandle(invokeCompleteEvent_);
}

IMFTopology* CameraTopology::GetTopology()
{
	return pTopology_; 
}

void CameraTopology::GetWebRtcCapability(webrtc::VideoCaptureCapability& webRtcCapability)
{
	webRtcCapability = webRtcCapability_;
}

//
// IUnknown methods
//
HRESULT CameraTopology::QueryInterface(REFIID riid, void** ppv)
{
	HRESULT hr = S_OK;

	if(ppv == nullptr)
	{
		return E_POINTER;
	}

	if(riid == IID_IMFAsyncCallback)
	{
		*ppv = static_cast<IMFAsyncCallback*>(this);
	}
	else if(riid == IID_IUnknown)
	{
		*ppv = static_cast<IUnknown*>(this);
	}
	else
	{
		*ppv = nullptr;
		hr = E_NOINTERFACE;
	}

	if(SUCCEEDED(hr))
	{
		AddRef();
	}

	return hr;
}

ULONG CameraTopology::AddRef()
{
	return ++nRefCount_;
}

ULONG CameraTopology::Release()
{
	ULONG uCount = --nRefCount_;
	if (uCount == 0)
	{
		delete this;
	}
	return uCount;
}

HRESULT CameraTopology::ShutdownSource()
{
	HRESULT hr = S_OK;
	if (pSource_ != nullptr)
	{
		hr = pSource_->Shutdown();
	}

	LOG_TRACE("Returning error code: " << hr);
	return hr;
}


HRESULT CameraTopology::SetFileSinkParameters(CameraConfMsg& conf)
{
	HRESULT hr = S_OK;
	wstring outFolder;
	uint32_t recordLen;
	CameraVideoRecording recordingType;
	conf.GetFileSinkParameters(outFolder, recordLen, recordingType);

	int cameraId;
	wstring cameraName;
	conf.GetCameraIds(cameraId, cameraName);

	if (recordingType == CameraVideoRecording::DISABLED)
	{
		LOG_TRACE("Exit because recording desabled for " << StringUtil::ToString(cameraName));
		return hr;
	}

	do
	{
		IVP8FileSinkConfiguration* pVP8FileSinkConf;
		HRESULT hr = pVP8FileSink_->QueryInterface(IID_IVP8FileSinkConfiguration, (void**)&pVP8FileSinkConf);
		BREAK_ON_FAIL(hr);

		string scameraName = StringUtil::ToString(cameraName);
		string soutFolder = StringUtil::ToString(outFolder);

		pVP8FileSinkConf->SetFileSinkParameters(soutFolder, scameraName, recordLen);
	}
	while(false);

	LOG_TRACE("Returning error code: " << hr);
	return hr;
}

void CameraTopology::RemoveExternalCapturer(webrtc::VideoCaptureExternal* captureObserver)
{
	lock_guard<std::mutex> lock(mutex_);
	captureObservers_.erase(reinterpret_cast<uint32_t>(captureObserver));
	LOG_TRACE("External capturer removed.");
}

void CameraTopology::RemoveExternalCapturers()
{
	lock_guard<std::mutex> lock(mutex_);
	captureObservers_.clear();
	LOG_TRACE("All external capturers removed.");
}

void CameraTopology::SetExternalCapturer(webrtc::VideoCaptureExternal* captureObserver)
{
	lock_guard<std::mutex> lock(mutex_);
	captureObservers_.insert(make_pair(reinterpret_cast<uint32_t>(captureObserver), captureObserver));
	LOG_TRACE("External capturer added.");
}

HRESULT CameraTopology::SetFrameCallback()
{
	HRESULT hr = S_OK;

	do
	{
		if (pVP8CallbackSink_ == nullptr)
		{
			hr = S_FALSE;
			LOG_TRACE("Callback pointer is NULL. Exiting.");
			break;
		}

		IVP8Callback* pVP8Callback;

		hr = pVP8CallbackSink_->QueryInterface(IID_IVP8Callback, (void**)&pVP8Callback);
		BREAK_ON_FAIL(hr);
		// Remove previous if any
		pVP8Callback->SetFrameCallback(NULL);
		delete newFrameCallback_;
		// Set new callback
		newFrameCallback_ = new FrameCallback(this);
		pVP8Callback->SetFrameCallback(newFrameCallback_);
	}
	while(false);

	LOG_TRACE("Returning error code: " << hr);
	return hr;
}
