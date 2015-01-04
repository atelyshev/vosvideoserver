#pragma once
#include "VosVideo.Data/CameraConfMsg.h"
#include "VosVideo.Data/SendData.h"
#include <modules/video_capture/include/video_capture_defines.h>
#include <Windows.h>

namespace vosvideo
{
	namespace cameraplayer
	{
		enum class PlayerState	
		{
			Closed,         // No session.
			Ready,          // Session was created, ready to open a file.
			OpenPending,    // Session is opening a file.
			Started,        // Session is playing a file.
			Paused,         // Session is paused.
			Stopped,        // Session is stopped (ready to play).
			Closing         // Application is waiting for MESessionClosed.
		};


		class CameraPlayerBase
		{
		public:
			CameraPlayerBase();
			virtual ~CameraPlayerBase();

			// Playback control
			virtual HRESULT OpenURL(vosvideo::data::CameraConfMsg&) = 0;
			virtual void GetWebRtcCapability(webrtc::VideoCaptureCapability& webRtcCapability) = 0;
			virtual HRESULT Play() = 0;
			virtual HRESULT Pause() = 0;
			virtual HRESULT Stop() = 0;
			virtual HRESULT Shutdown() = 0;

			virtual PlayerState GetState(std::shared_ptr<vosvideo::data::SendData>& lastErrMsg) const = 0;
			virtual PlayerState GetState() const = 0;

			// Probably most important method, through it camera communicates to WebRTC
			virtual void SetExternalCapturer(webrtc::VideoCaptureExternal* captureObserver) = 0;
			virtual void RemoveExternalCapturers() = 0;
			virtual void RemoveExternalCapturer(webrtc::VideoCaptureExternal* captureObserver) = 0;

			virtual uint32_t GetDeviceId() const = 0;
		};
	}
}

