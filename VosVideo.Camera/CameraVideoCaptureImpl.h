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
			virtual ~CameraVideoCaptureImpl();


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

		private:
			vosvideo::cameraplayer::CameraPlayerBase* player_;

		protected:
			bool startedCapture_;
			webrtc::I420VideoFrame captureFrame_;
		};
	}
}
