#pragma once

#include <talk/base/messagehandler.h>
#include <talk/media/base/videocapturer.h>
#include <modules/video_capture/include/video_capture.h>
#include <talk/media/webrtc/webrtcvideoframe.h>
#include "VosVideo.CameraPlayer/CameraPlayerBase.h"


namespace vosvideo
{
	namespace camera
	{
		// Factory to allow injection of a VCM impl into WebRtcVideoCapturer.
		// DeviceInfos do not have a Release() and therefore need an explicit Destroy().
		class CameraVcmFactoryInterface 
		{
		public:
			virtual ~CameraVcmFactoryInterface() {}
			virtual webrtc::VideoCaptureModule* Create(int id, const char* device) = 0;
			virtual webrtc::VideoCaptureModule* Create(const int32_t id, webrtc::VideoCaptureExternal*& externalCapture, vosvideo::cameraplayer::CameraPlayerBase*) = 0;
			virtual webrtc::VideoCaptureModule::DeviceInfo* CreateDeviceInfo(int id) = 0;
			virtual void DestroyDeviceInfo(webrtc::VideoCaptureModule::DeviceInfo* info) = 0;
		};

		// MediaFoundation-based implementation of VideoCapturer.
		class CameraVideoCapturer : public cricket::VideoCapturer,
			public webrtc::VideoCaptureDataCallback 
		{
		public:
			CameraVideoCapturer();
			explicit CameraVideoCapturer(CameraVcmFactoryInterface* factory);
			virtual ~CameraVideoCapturer();

			bool Init(int camId, vosvideo::cameraplayer::CameraPlayerBase* device);
			bool Init(const cricket::Device& device);
			bool Init(webrtc::VideoCaptureModule* module);

			// Override virtual methods of the parent class VideoCapturer.
			virtual bool GetBestCaptureFormat(const cricket::VideoFormat& desired,
				cricket::VideoFormat* best_format);
			virtual cricket::CaptureState Start(const cricket::VideoFormat& capture_format);
			virtual void Stop();
			virtual bool IsRunning();
			virtual bool IsScreencast() const { return false; }

		protected:
			// Override virtual methods of the parent class VideoCapturer.
			virtual bool GetPreferredFourccs(std::vector<uint32>* fourccs);

		private:
			// Callback when a frame is captured by camera.
			virtual void OnCaptureDelayChanged(const int32_t id,
				const int32_t delay);
			virtual void OnIncomingCapturedFrame(const int32_t id,
				webrtc::I420VideoFrame& videoFrame);

			talk_base::scoped_ptr<CameraVcmFactoryInterface> factory_;
			webrtc::VideoCaptureModule* module_;
			int captured_frames_;
			std::vector<uint8_t> capture_buffer_;
		};
	}
}
