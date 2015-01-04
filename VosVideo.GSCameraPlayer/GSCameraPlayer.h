#pragma once
#include <boost\thread\thread.hpp>
#include <gst\gst.h>
#include "VosVideo.CameraPlayer\CameraPlayerBase.h"
#include "GSPipelineBase.h"

namespace vosvideo
{
	namespace cameraplayer
	{
		class GSCameraPlayer : public vosvideo::cameraplayer::CameraPlayerBase
		{
		public:
			GSCameraPlayer();
			~GSCameraPlayer();

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

			uint32_t GetDeviceId() const;			

		private:			
			PlayerState  _state;
			int _deviceId;
			std::wstring _deviceName;
			GSPipelineBase* _pipeline;
		};
	}
}