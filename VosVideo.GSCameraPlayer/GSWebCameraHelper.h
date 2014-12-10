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
			GSWebCameraHelper();
			~GSWebCameraHelper();

			// Enumerates all video devices an returns pair Device_link/VideoSource
			HRESULT CreateVideoCaptureDevices(WebCamsList& webCams);
		};
	}
}
