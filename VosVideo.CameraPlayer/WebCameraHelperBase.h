#pragma once
#include <windows.h>
#include <WTypesbase.h>
#include <string>
#include <vector>
#include <unordered_map>
#include <boost/variant.hpp>

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

			typedef std::vector<WebCameraDescription> WebCamsList;
			typedef boost::variant<int, ULONG, ULARGE_INTEGER, DOUBLE, std::wstring, std::pair<unsigned int, unsigned int>> VariantAttr;
			typedef std::unordered_map <std::wstring, VariantAttr> AttrList;
			typedef std::pair<unsigned int, unsigned int> PairedAttr;
			typedef std::vector<AttrList> CaptureFormats;


			// Enumerates all video devices an returns pair Device_link/VideoSource
			virtual HRESULT CreateVideoCaptureDevices(WebCamsList& webCams) = 0;
		};
	}
}
