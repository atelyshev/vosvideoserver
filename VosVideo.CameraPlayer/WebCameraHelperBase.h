#pragma once
#include <vector>

namespace vosvideo
{
	namespace cameraplayer
	{
		class WebCameraDescription
		{
		public:
			std::wstring SymLink;
			std::wstring FriendlyName;
		};

		class WebCameraHelperBase
		{
		public:
			WebCameraHelperBase();
			virtual ~WebCameraHelperBase();

			using WebCamsList = std::vector<WebCameraDescription>;

			// Enumerates all video devices an returns pair Device_link/VideoSource
			virtual void CreateVideoCaptureDevices(WebCamsList& webCams) = 0;
		};
	}
}
