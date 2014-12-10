#include "stdafx.h"
#include <mfapi.h>
#include <mferror.h>
#include <assert.h>
#include <boost/bind.hpp>
#include <vosvideocommon/StringUtil.h>
#include <VosVideoCommon/ComHelper.h>
#include "IpCameraTopology.h"
#include "WebCameraTopology.h"
#include "MFCameraPlayer.h"

using namespace std;
using namespace util;
using namespace vosvideo::data;
using namespace vosvideo::cameraplayer;

//
//  CPlayer constructor - instantiates internal objects and initializes MF
//
MFCameraPlayer::MFCameraPlayer() : 
	pSession_(NULL),
	state_(PlayerState::Closed),
	nRefCount_(1),
	devId_(0)
{
	// create an event that will be fired when the asynchronous IMFMediaSession::Close() 
	// operation is complete
	closeCompleteEvent_ = CreateEvent(NULL, FALSE, FALSE, NULL);
}


MFCameraPlayer::~MFCameraPlayer()
{
	assert(pSession_ == nullptr);  
	Shutdown();
	// close the event
	CloseHandle(closeCompleteEvent_);
}

//
// OpenURL is the main initialization function that triggers bulding of the core
// MF components.
//
HRESULT MFCameraPlayer::OpenURL(CameraConfMsg& conf )
{
	HRESULT hr = S_OK;
	wstring devName;
	conf.GetCameraIds(devId_, devName);

	do
	{
		// Step 1: create a media session if one doesn't exist already
		if(pSession_ == nullptr)
		{
			hr = CreateSession();
			BREAK_ON_FAIL(hr);
		}

		switch (conf.GetCameraType())
		{
		case vosvideo::data::CameraType::UNKNOWN:
			LOG_ERROR("Camera type is UNKNOWN.");
			hr = MF_E_NET_UNSUPPORTED_CONFIGURATION;
			break;
		case vosvideo::data::CameraType::IPCAM:
			LOG_TRACE("Open IP Camera URL");
			camTopology_.reset(new IpCameraTopology());
			break;
		case vosvideo::data::CameraType::WEBCAM:
			LOG_TRACE("Open Web Camera sym link");
			camTopology_.reset(new WebCameraTopology());
			break;
		}
		BREAK_ON_FAIL(hr);

		// Step 2: build the topology.  Here we are using the TopoBuilder helper class.
		hr = camTopology_->RenderUrlAsync(conf, boost::bind(&MFCameraPlayer::OnRenderUrlComplete, this, _1, _2));
		BREAK_ON_FAIL(hr);

		// If we've just initialized a brand new topology in step 1, set the player state 
		// to "open pending" - not playing yet, but ready to begin.
		if(state_ == PlayerState::Ready)
		{
			state_ = PlayerState::OpenPending;
		}
	}
	while(false);

	if (FAILED(hr))
	{
		state_ = PlayerState::Closed;
	}

	LOG_TRACE("Player state: " << static_cast<int>(state_));
	LOG_TRACE("Returning error code: " << hr);
	return hr;
}

uint32_t MFCameraPlayer::GetDeviceId() const
{
	return devId_;
}

void MFCameraPlayer::OnRenderUrlComplete(HRESULT hr, shared_ptr<SendData> errMsg)
{
	if (hr != S_OK)
	{
		state_ = PlayerState::Closed;
		lastErrMsg_ = errMsg;
		return;
	}
	
	do
	{
		CComPtr<IMFTopology> pTopology = nullptr;
		// get the topology from the IpCamTopology
		pTopology = camTopology_->GetTopology();
		BREAK_ON_NULL(pTopology, E_UNEXPECTED);

		// Step 3: add the topology to the internal queue of topologies associated with this
		// media session
		if(pTopology != nullptr)
		{
			hr = pSession_->SetTopology(0, pTopology);
			LOG_TRACE("Set topology returned: " << hr);
			BREAK_ON_FAIL(hr);
		}
	}
	while(false);

	if (FAILED(hr))
	{
		state_ = PlayerState::Closed;
	}
	LOG_TRACE("Player state: " << static_cast<int>(state_));
}

PlayerState MFCameraPlayer::GetState(shared_ptr<SendData>& lastErrMsg) const 
{
	lastErrMsg = lastErrMsg_;
	return state_; 
}

PlayerState MFCameraPlayer::GetState() const 
{
	return state_; 
}

//
// IMFAsyncCallback::Invoke implementation.  This is the function called by media session
// whenever anything of note happens or an asynchronous operation is complete.
//
// pAsyncResult - a pointer to the asynchronous result object which references the event 
// itself in the IMFMediaEventGenerator's event queue.  (The media session is the object
// that implements the IMFMediaEventGenerator interface.)
//
HRESULT MFCameraPlayer::Invoke(IMFAsyncResult* pAsyncResult)
{
	CComPtr<IMFMediaEvent> pEvent;
	HRESULT hr = S_OK;
	MediaEventType eventType;

	do
	{
		// Get the event from the event queue.
		hr = pSession_->EndGetEvent(pAsyncResult, &pEvent);
		LOG_TRACE("EndGetEvent returned: " << hr);
		BREAK_ON_FAIL(hr);

		// Get the event type.
		hr = pEvent->GetType(&eventType);
		LOG_TRACE("GetType returned event type: " << eventType << " and error code: " << hr);
		BREAK_ON_FAIL(hr);

		// MESessionClosed event is guaranteed to be the last event fired by the session. 
		// Fire the m_closeCompleteEvent to let the player know that it can safely shut 
		// down the session and release resources associated with the session.
		if (eventType == MESessionClosed)
		{
			SetEvent(closeCompleteEvent_);
		}
		else
		{
			// If this is not the final event, tell the Media Session that this player is 
			// the object that will handle the next event in the queue.
			hr = pSession_->BeginGetEvent(this, NULL);
			BREAK_ON_FAIL(hr);
		}

		// If we are in a normal state, handle the event by passing it to the HandleEvent()
		// function.  Otherwise, if we are in the closing state, do nothing with the event.
		if (state_ != PlayerState::Closing)
		{
			hr = ProcessEvent(pEvent);
			if (hr != S_OK)
			{
				Stop();
			}
		}
	}
	while(false);

	LOG_TRACE("Returning error code: " << hr);
	return hr;
}

//
//  Creates a new instance of the media session.
//
HRESULT MFCameraPlayer::CreateSession()
{
	// Close the old session, if any.
	HRESULT hr = S_OK;
	MF_TOPOSTATUS topoStatus = MF_TOPOSTATUS_INVALID;
	CComQIPtr<IMFMediaEvent> mfEvent;

	do
	{
		// close the session if one is already created
		hr = CloseSession();
		BREAK_ON_FAIL(hr);

		assert(state_ == PlayerState::Closed);

		// Create the media session.
		hr = MFCreateMediaSession(NULL, &pSession_);
		LOG_TRACE("MFCreateMediaSession returned: " << hr);
		BREAK_ON_FAIL(hr);

		state_ = PlayerState::Ready;

		// designate this class as the one that will be handling events from the media 
		// session
		hr = pSession_->BeginGetEvent((IMFAsyncCallback*)this, NULL);
		LOG_TRACE("BeginGetEvent returned: " << hr);
		BREAK_ON_FAIL(hr);
	}
	while(false);

	LOG_TRACE("Returning error code: " << hr);
	return hr;
}


//
//  Called by Invoke() to do the actual event processing, and determine what, if anything
//  needs to be done.
//
HRESULT MFCameraPlayer::ProcessEvent(CComPtr<IMFMediaEvent>& mediaEvent)
{
	HRESULT hrStatus = S_OK;            // Event status
	HRESULT hr = S_OK;
	MF_TOPOSTATUS TopoStatus = MF_TOPOSTATUS_INVALID; 
	MediaEventType eventType;

	do
	{
		BREAK_ON_NULL( mediaEvent, E_POINTER );

		// Get the event type.
		hr = mediaEvent->GetType(&eventType);
		LOG_TRACE("GetType returned: " << hr);
		BREAK_ON_FAIL(hr);

		// Get the event status. If the operation that triggered the event did
		// not succeed, the status is a failure code.
		hr = mediaEvent->GetStatus(&hrStatus);
		LOG_TRACE("GetStatus returned status : " << hrStatus << " and error code: " << hr);
		BREAK_ON_FAIL(hr);

		// Check if the async operation succeeded.
		if (FAILED(hrStatus))
		{
			hr = hrStatus;
			break;
		}

		// Switch on the event type. Update the internal state of the CPlayer as needed.
		if(eventType == MESessionTopologyStatus)
		{
			// Get the status code.
			hr = mediaEvent->GetUINT32(MF_EVENT_TOPOLOGY_STATUS, (UINT32*)&TopoStatus);
			BREAK_ON_FAIL(hr);

			if (TopoStatus == MF_TOPOSTATUS_READY)
			{
				hr = OnTopologyReady();
			}
		}
		else if(eventType == MEEndOfPresentation)
		{
			hr = OnPresentationEnded();
		}
	}
	while(false);

	if(FAILED(hr))
	{
		string err = StringUtil::IntToHex(hr);
		LOG_ERROR("ProcessEvent(): An error has occured:" << err);
	}

	return hr;
}

//
// IUnknown methods
//
HRESULT MFCameraPlayer::QueryInterface(REFIID riid, void** ppv)
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

ULONG MFCameraPlayer::AddRef()
{
	return ++nRefCount_;
}

ULONG MFCameraPlayer::Release()
{
	ULONG uCount = --nRefCount_;
	if (uCount == 0)
	{
		delete this;
	}
	return uCount;
}

//
//  Starts playback from paused or stopped state.
//
HRESULT MFCameraPlayer::Play()
{
	if (state_ != PlayerState::Paused && state_ != PlayerState::Stopped)
	{
		return MF_E_INVALIDREQUEST;
	}

	if (pSession_ == nullptr)
	{
		return E_UNEXPECTED;
	}

	return StartPlayback();
}

//  Pause playback.
HRESULT MFCameraPlayer::Pause()    
{
	if (state_ != PlayerState::Started)
	{
		return MF_E_INVALIDREQUEST;
	}
	if (pSession_ == nullptr)
	{
		return E_UNEXPECTED;
	}

	HRESULT hr = pSession_->Pause();
	if (SUCCEEDED(hr))
	{
		state_ = PlayerState::Paused;
	}

	LOG_TRACE("Returning error code: " << hr);
	return hr;
}

//
//  Starts playback from the current position.
//
HRESULT MFCameraPlayer::StartPlayback()
{
	assert(pSession_ != nullptr);
	PROPVARIANT varStart;
	PropVariantInit(&varStart);
	varStart.vt = VT_EMPTY;

	// If Start fails later, we will get an MESessionStarted event with an error code, 
	// and will update our state. Passing in GUID_NULL and VT_EMPTY indicates that
	// playback should start from the current position.
	HRESULT hr = pSession_->Start(&GUID_NULL, &varStart);
	if (SUCCEEDED(hr))
	{
		LOG_TRACE("Session started playback: " << hr);
		state_ = PlayerState::Started;
	}

	PropVariantClear(&varStart);

	LOG_TRACE("Returning error code: " << hr);
	return hr;
}

//
// Handler for MESessionTopologyReady event - starts session playback.
//
HRESULT MFCameraPlayer::OnTopologyReady()
{
	// since the topology is ready, start playback
	HRESULT hr = StartPlayback();
	pPresentationClock_ = nullptr;
	pSession_->GetClock(&pPresentationClock_);

	LOG_TRACE("Returning error code: " << hr);
	return hr;
}


//
//  Handler for MEEndOfPresentation event, fired when playback has stopped.
//
HRESULT MFCameraPlayer::OnPresentationEnded()
{
	// The session puts itself into the stopped state automatically.
	state_ = PlayerState::Stopped;
	LOG_TRACE("Change presentation state to Stopped");
	return S_OK;
}

HRESULT MFCameraPlayer::Stop()
{
	if (state_ != PlayerState::Started && state_ != PlayerState::Paused)
	{
		return MF_E_INVALIDREQUEST;
	}

	if (pSession_ == nullptr)
	{
		return E_UNEXPECTED;
	}

	HRESULT hr = pSession_->Stop();

	if (SUCCEEDED(hr))
	{
		state_ = PlayerState::Stopped;
	}

	LOG_TRACE("Returning error code: " << hr);
	return hr;
}

//  Release all resources held by this object.
HRESULT MFCameraPlayer::Shutdown()
{
	// Close the session
	HRESULT hr = CloseSession();

	LOG_TRACE("Returning error code: " << hr);
	return hr;
}

//
//  Closes the media session.
//
//  The IMFMediaSession::Close method is asynchronous, so the CloseSession 
//  method waits for the MESessionClosed event. The MESessionClosed event is 
//  guaranteed to be the last event that the media session fires.
//
HRESULT MFCameraPlayer::CloseSession()
{
	HRESULT hr = S_OK;
	//	state_ = PlayerState::Closing;

	// Call the asynchronous Close() method and then wait for the close
	// operation to complete on another thread
	if (pSession_ != nullptr)
	{
		state_ = PlayerState::Closing;

		hr = pSession_->Close();
		if (SUCCEEDED(hr))
		{
			// Begin waiting for the Win32 close event, fired in CPlayer::Invoke(). The 
			// close event will indicate that the close operation is finished, and the 
			// session can be shut down.
			LOG_TRACE("Begin wait for close event.");
			DWORD dwWaitResult = WaitForSingleObject(closeCompleteEvent_, 15000);
			if (dwWaitResult == WAIT_TIMEOUT)
			{
				LOG_ERROR("Close event timed out.");
				assert(FALSE);
			}
		}
	}

	if (camTopology_)
	{
		hr = camTopology_->ShutdownSource();
	}
	// Shut down the media session. (Synchronous operation, no events.)  Releases all of the
	// internal session resources.
	if (pSession_ != nullptr)
	{
		pSession_->Shutdown();
	}

	// release the session
	pSession_ = nullptr;

	state_ = PlayerState::Closed;

	LOG_TRACE("Returning error code: " << hr);
	return hr;
}

void MFCameraPlayer::GetWebRtcCapability(webrtc::VideoCaptureCapability& webRtcCapability)
{
	camTopology_->GetWebRtcCapability(webRtcCapability);
}

void MFCameraPlayer::SetExternalCapturer(webrtc::VideoCaptureExternal* captureObserver)
{
	camTopology_->SetExternalCapturer(captureObserver);
}

void MFCameraPlayer::RemoveExternalCapturer(webrtc::VideoCaptureExternal* captureObserver)
{
	camTopology_->RemoveExternalCapturer(captureObserver);
}

void MFCameraPlayer::RemoveExternalCapturers()
{
	camTopology_->RemoveExternalCapturers();
}
