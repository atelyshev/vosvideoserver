#pragma once
#include <boost/variant.hpp>
#include "VosVideo.CameraPlayer/WebCameraHelperBase.h"

namespace vosvideo
{
	namespace cameraplayer
	{
		class GSWebCameraHelper : public vosvideo::cameraplayer::WebCameraHelperBase
		{
		public:
			// Enumerates all video devices an returns pair Device_link/VideoSource
			void CreateVideoCaptureDevices(WebCamsList& webCams);
		};
	}
}
