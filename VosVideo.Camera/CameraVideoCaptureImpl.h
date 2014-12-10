#pragma once

#include <modules/video_capture/video_capture_impl.h>
#include <modules/video_capture/include/video_capture.h>
#include "VosVideo.CameraPlayer/CameraPlayerBase.h"

namespace vosvideo
{
	namespace camera
	{
		class CameraVideoCaptureImpl : public webrtc::videocapturemodule::VideoCaptureImpl
		{
		public:
			CameraVideoCaptureImpl(const int32_t id, vosvideo::cameraplayer::CameraPlayerBase* player);

			static webrtc::VideoCaptureModule* Create(const int32_t id, webrtc::VideoCaptureExternal*& externalCapture, vosvideo::cameraplayer::CameraPlayerBase* player);

			/*************************************************************************
			 *
			 *   Start/Stop
			 *
			 *************************************************************************/
			virtual int32_t StartCapture(const webrtc::VideoCaptureCapability& capability);
			virtual int32_t StopCapture();

			/**************************************************************************
			 *
			 *   Properties of the set device
			 *
			 **************************************************************************/

			virtual bool CaptureStarted();
			virtual int32_t CaptureSettings(webrtc::VideoCaptureCapability& settings);

		protected:
			virtual ~CameraVideoCaptureImpl();
			bool startedCapture_;
			vosvideo::cameraplayer::CameraPlayerBase* player_;
			webrtc::I420VideoFrame captureFrame_;
		};
	}
}
