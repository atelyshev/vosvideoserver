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

			int32_t OpenURL(vosvideo::data::CameraConfMsg&) override;

			void GetWebRtcCapability(webrtc::VideoCaptureCapability& webRtcCapability) override;
			int32_t Play() override;
			int32_t Pause() override;
			int32_t Stop() override;
			int32_t Shutdown() override;

			PlayerState GetState(std::shared_ptr<vosvideo::data::SendData>& lastErrMsg) const  override;
			PlayerState GetState() const override;

			// Probably most important method, through it camera communicates to WebRTC
			void SetExternalCapturer(webrtc::VideoCaptureExternal* captureObserver) override;
			void RemoveExternalCapturers() override;
			void RemoveExternalCapturer(webrtc::VideoCaptureExternal* captureObserver) override;

			uint32_t GetDeviceId() const override;

		private:			
			PlayerState  _state;
			int _deviceId;
			std::wstring _deviceName;
			GSPipelineBase* _pipeline;
		};
	}
}