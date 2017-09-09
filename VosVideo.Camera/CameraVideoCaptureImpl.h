#pragma once

#include <webrtc/modules/video_capture/video_capture_impl.h>
#include <webrtc/modules/video_capture/video_capture.h>
#include "VosVideo.CameraPlayer/CameraPlayerBase.h"

namespace vosvideo
{
	namespace camera
	{
		class CameraVideoCaptureImpl : public webrtc::videocapturemodule::VideoCaptureImpl
		{
		public:
			CameraVideoCaptureImpl(vosvideo::cameraplayer::CameraPlayerBase* player);
			virtual ~CameraVideoCaptureImpl();
			// Start/Stop
			virtual int32_t StartCapture(const webrtc::VideoCaptureCapability& capability);
			virtual int32_t StopCapture();

			//  Properties of the set device
			virtual bool CaptureStarted();
			virtual int32_t CaptureSettings(webrtc::VideoCaptureCapability& settings);

		private:
			vosvideo::cameraplayer::CameraPlayerBase* player_ = nullptr;
			// The creator must call AddRef() after construction and use Release()
			// to release the reference and delete this object.
			int32_t AddRef() const override;
			int32_t Release() const override;
			mutable std::atomic<int32_t> ref_count_;

		protected:
			bool startedCapture_;
		};
	}
}
