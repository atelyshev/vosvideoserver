#pragma once

#include <webrtc/base/messagehandler.h>
#include <webrtc/media/base/videocapturer.h>
#include <webrtc/modules/video_capture/video_capture.h>
#include "webrtc/media/base/videosinkinterface.h"
#include <webrtc/media/base/device.h>
#include "VosVideo.CameraPlayer/CameraPlayerBase.h"
#include "VosVideo.Camera/CameraVideoCaptureImpl.h"


namespace vosvideo
{
	namespace camera
	{
		class CameraVideoCapturer : public cricket::VideoCapturer, 
			public rtc::VideoSinkInterface<webrtc::VideoFrame>
		{
		public:
			virtual ~CameraVideoCapturer();

			bool Init(int camId, vosvideo::cameraplayer::CameraPlayerBase* device);
			bool Init(const cricket::Device& device);
			bool Init(webrtc::VideoCaptureModule* module);

			// Override virtual methods of the parent class VideoCapturer.
			virtual cricket::CaptureState Start(const cricket::VideoFormat& capture_format) override;
			virtual void Stop() override;
			virtual bool IsRunning() override;
			virtual bool IsScreencast() const override { return false; }
			virtual bool GetPreferredFourccs(std::vector<uint32_t>* fourccs) override;
			// Override virtual methods of the parent class VideoSinkInterface
			virtual void OnFrame(const webrtc::VideoFrame& frame) override;

		private:
			// Callback when a frame is captured by camera.
			vosvideo::camera::CameraVideoCaptureImpl* videoCapturerImpl_ = nullptr;
			int captured_frames_ = 0;
		};
	}
}
