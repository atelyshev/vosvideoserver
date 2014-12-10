#pragma once
#include <mfapi.h>
#include <mfidl.h>
#pragma once
#include <boost/variant.hpp>
#include "VosVideo.CameraPlayer/WebCameraHelperBase.h"

namespace vosvideo
{
	namespace cameraplayer
	{
		class MFWebCameraHelper : public vosvideo::cameraplayer::WebCameraHelperBase
		{
		public:
			MFWebCameraHelper();
			~MFWebCameraHelper();

			// Enumerates all video devices an returns pair Device_link/VideoSource
			HRESULT CreateVideoCaptureDevices(WebCamsList& webCams);
			// Uses device link found in CreateVideoCaptureDevices before and creates VideoSource
			HRESULT CreateVideoCaptureDeviceFromLink(std::wstring& symLink, IMFMediaSource **ppSource);
			// Allows to find appropriate video mode for targeted device
			HRESULT EnumerateCaptureFormats(IMFMediaSource *pSource, CaptureFormats& formats);
			// Allows to set most appropriate video format for targeted device
			HRESULT SetDeviceFormat(IMFMediaSource *pSource, DWORD dwFormatIndex);
			// Allows to set most appropriate frame rate for targeted device
			HRESULT SetFrameRate(IMFMediaSource *pSource, DWORD dwTypeIndex, unsigned int fRate);
			HRESULT GetGUIDName(const GUID& guid, std::wstring& guidName);

		protected:
			HRESULT GetDeviceSymLink(IMFActivate *pDevice, std::wstring& friendlyName, std::wstring& symLink);
			void DebugShowDeviceNames(IMFActivate **ppDevices, unsigned int count);
			HRESULT LogMediaType(IMFMediaType *pType, AttrList& attrList);
			void LogUINT32AsUINT64(const PROPVARIANT& var, PairedAttr& valPair);
			HRESULT LogAttributeValueByIndex(IMFAttributes *pAttr, AttrList& attr, unsigned int index);
			HRESULT LogVideoArea(const PROPVARIANT& var);

			LPCWSTR GetGUIDNameConst(const GUID& guid);
			void DBGMSG(PCWSTR format, ...);
			HRESULT SpecialCaseAttributeValue(GUID guid, const PROPVARIANT& var, PairedAttr& valPair);
			float OffsetToFloat(const MFOffset& offset);
		};
	}
}
