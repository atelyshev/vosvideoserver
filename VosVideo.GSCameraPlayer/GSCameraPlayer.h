#pragma once
#include <boost/thread/thread.hpp>
#include <gst/gst.h>
#include "VosVideo.CameraPlayer/CameraPlayerBase.h"
#include "GSPipelineBase.h"

namespace vosvideo
{
	namespace cameraplayer
	{
		class GSCameraPlayer : public vosvideo::cameraplayer::CameraPlayerBase
		{
		public:
			GSCameraPlayer();
			virtual ~GSCameraPlayer();

			int32_t OpenURL(vosvideo::data::CameraConfMsg&);

			void GetWebRtcCapability(webrtc::VideoCaptureCapability& webRtcCapability);
			int32_t Play();
			int32_t Pause();
			int32_t Stop();
			int32_t Shutdown();

			PlayerState GetState(std::shared_ptr<vosvideo::data::SendData>& lastErrMsg) const;
			PlayerState GetState() const;

			// Probably most important method, through it camera communicates to WebRTC
			void SetExternalCapturer(webrtc::VideoCaptureExternal* captureObserver);
			void RemoveExternalCapturers();
			void RemoveExternalCapturer(webrtc::VideoCaptureExternal* captureObserver);

			uint32_t GetDeviceId() const;			

		private:			
			PlayerState  _state;
			int _deviceId;
			std::wstring _deviceName;
			GSPipelineBase* _pipeline;
		};
	}
}