#pragma once
#include <mfobjects.h>
#include <mfidl.h>
#include "CameraTopology.h"
#include "VosVideo.CameraPlayer/CameraPlayerBase.h"

namespace vosvideo
{
	namespace cameraplayer
	{
		class MFCameraPlayer : 
			public vosvideo::cameraplayer::CameraPlayerBase, IMFAsyncCallback
		{
		public:
			MFCameraPlayer();
			~MFCameraPlayer();

			// Playback control
			HRESULT OpenURL(vosvideo::data::CameraConfMsg&);
			void GetWebRtcCapability(webrtc::VideoCaptureCapability& webRtcCapability);
			HRESULT Play();
			HRESULT Pause();
			HRESULT Stop();
			HRESULT Shutdown();

			PlayerState GetState(std::shared_ptr<vosvideo::data::SendData>& lastErrMsg) const;
			PlayerState GetState() const;

			// Probably most important method, through it camera communicates to WebRTC
			void SetExternalCapturer(webrtc::VideoCaptureExternal* captureObserver);
			void RemoveExternalCapturers();
			void RemoveExternalCapturer(webrtc::VideoCaptureExternal* captureObserver);
			//
			// IMFAsyncCallback implementation.
			//
			// Skip the optional GetParameters() function - it is used only in advanced players.
			// Returning the E_NOTIMPL error code causes the system to use default parameters.
			STDMETHODIMP GetParameters(DWORD *pdwFlags, DWORD *pdwQueue)   { return E_NOTIMPL; }

			// Main MF event handling function
			STDMETHODIMP Invoke(IMFAsyncResult* pAsyncResult);

			//
			// IUnknown methods
			//
			STDMETHODIMP QueryInterface(REFIID iid, void** ppv);
			STDMETHODIMP_(ULONG) AddRef();
			STDMETHODIMP_(ULONG) Release();

			uint32_t GetDeviceId() const;
		private:
			// MF event handling functionality
			HRESULT ProcessEvent(CComPtr<IMFMediaEvent>& mediaEvent);    
			// private session and playback controlling functions
			HRESULT CreateSession();
			HRESULT CloseSession();
			HRESULT StartPlayback();

			// Media event handlers
			HRESULT OnTopologyReady();
			HRESULT OnPresentationEnded();
			// On Signal reaction
			void OnRenderUrlComplete(HRESULT hr, std::shared_ptr<vosvideo::data::SendData> errMsg);

			CComPtr<IMFMediaSession> pSession_;    
			CComPtr<IMFClock>        pPresentationClock_;

			PlayerState              state_;       // Current state of the media session.
			std::atomic<long>        nRefCount_;   // COM reference count.
			std::shared_ptr<vosvideo::data::SendData> lastErrMsg_;
			// event fired when session close is complete
			HANDLE                   closeCompleteEvent_;
			std::shared_ptr<CameraTopology> camTopology_;
			int devId_;
		};
	}
}