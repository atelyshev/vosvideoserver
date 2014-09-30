#include "stdafx.h"
#include <vosvideocommon/ComHelper.h>
#include <mferror.h>
#include "WebCameraHelper.h"

using namespace std;
using namespace vosvideo::camera;

WebCameraHelper::WebCameraHelper()
{
}


WebCameraHelper::~WebCameraHelper()
{
}

HRESULT WebCameraHelper::CreateVideoCaptureDevices(WebCamsList& webCams)
{
	IMFMediaSource* pSource = NULL;
	UINT32 count = 0;
	IMFAttributes *pConfig = NULL;
	IMFActivate **ppDevices = NULL;

	// Create an attribute store to hold the search criteria.
	HRESULT hr = MFCreateAttributes(&pConfig, 1);

	// Request video capture devices.
	if (SUCCEEDED(hr))
	{
		hr = pConfig->SetGUID(
			MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE, 
			MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID
			);
	}

	// Enumerate the devices,
	if (SUCCEEDED(hr))
	{
		hr = MFEnumDeviceSources(pConfig, &ppDevices, &count);
	}

	DebugShowDeviceNames(ppDevices, count);

	// Create a media source for the first device in the list.
	if (SUCCEEDED(hr))
	{
		for (unsigned int i = 0; i < count; i++)
		{
			if (count > 0)
			{
				hr = ppDevices[i]->ActivateObject(IID_PPV_ARGS(&pSource));
			}
			else
			{
				hr = MF_E_NOT_FOUND;
			}
			if (hr == S_OK)
			{
				WebCameraDescription descr;
				GetDeviceSymLink(ppDevices[i], descr.friendlyName_, descr.symLink_);		
				webCams.push_back(descr);
			}
		}
	}

	for (DWORD i = 0; i < count; i++)
	{
		ppDevices[i]->Release();
	}
	CoTaskMemFree(ppDevices);
	return hr;
}

HRESULT WebCameraHelper::CreateVideoCaptureDeviceFromLink(wstring& symLink, IMFMediaSource **ppSource)
{
	*ppSource = NULL;

	CComPtr<IMFAttributes> pAttributes;
	IMFMediaSource *pSource = NULL;

	HRESULT hr = MFCreateAttributes(&pAttributes, 2);

	// Set the device type to video.
	if (SUCCEEDED(hr))
	{
		hr = pAttributes->SetGUID(
			MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE,
			MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID);
	}

	// Set the symbolic link.
	if (SUCCEEDED(hr))
	{
		hr = pAttributes->SetString(
			MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_SYMBOLIC_LINK,
			symLink.c_str());            
	}

	if (SUCCEEDED(hr))
	{
		hr = MFCreateDeviceSource(pAttributes, ppSource);
	}

	return hr;    
}

HRESULT WebCameraHelper::GetDeviceSymLink(IMFActivate *pDevice, wstring& friendlyName, wstring& symLink)
{
	HRESULT hr = S_OK;
	WCHAR *szAttrName = NULL;
	UINT32 cchName;

	hr = pDevice->GetAllocatedString(MF_DEVSOURCE_ATTRIBUTE_FRIENDLY_NAME, &szAttrName, &cchName);

	if (SUCCEEDED(hr))
	{
		friendlyName = wstring(szAttrName);
	}

	CoTaskMemFree(szAttrName);

	// Try to get the display name.
	hr = pDevice->GetAllocatedString(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_SYMBOLIC_LINK, &szAttrName, &cchName);

	if (SUCCEEDED(hr))
	{
		symLink = wstring(szAttrName);
	}

	CoTaskMemFree(szAttrName);	

	return hr;
}

void WebCameraHelper::DebugShowDeviceNames(IMFActivate **ppDevices, UINT count)
{
	for (DWORD i = 0; i < count; i++)
	{
		HRESULT hr = S_OK;
		WCHAR *szFriendlyName = NULL;

		// Try to get the display name.
		UINT32 cchName;
		hr = ppDevices[i]->GetAllocatedString(
			MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_SYMBOLIC_LINK,
			//			MF_DEVSOURCE_ATTRIBUTE_FRIENDLY_NAME,
			&szFriendlyName, &cchName);

		if (SUCCEEDED(hr))
		{
			OutputDebugString(szFriendlyName);
			OutputDebugString(L"\n");
		}
		CoTaskMemFree(szFriendlyName);
	}
}

HRESULT WebCameraHelper::EnumerateCaptureFormats(IMFMediaSource *pSource, CaptureFormats& formats)
{
	CComPtr<IMFPresentationDescriptor> pPD;
	CComPtr<IMFStreamDescriptor> pSD;
	CComPtr<IMFMediaTypeHandler> pHandler;
	HRESULT hr = S_OK;

	do
	{
		hr = pSource->CreatePresentationDescriptor(&pPD);
		BREAK_ON_FAIL(hr);

		BOOL fSelected;
		hr = pPD->GetStreamDescriptorByIndex(0, &fSelected, &pSD);
		BREAK_ON_FAIL(hr);

		hr = pSD->GetMediaTypeHandler(&pHandler);
		BREAK_ON_FAIL(hr);

		DWORD cTypes = 0;
		hr = pHandler->GetMediaTypeCount(&cTypes);
		BREAK_ON_FAIL(hr);

		for (DWORD i = 0; i < cTypes; i++)
		{
			IMFMediaType* pType;
			hr = pHandler->GetMediaTypeByIndex(i, &pType);
			BREAK_ON_FAIL(hr);

			AttrList attrList;
			LogMediaType(pType, attrList);
			formats.push_back(attrList);
			OutputDebugString(L"\n");
			SafeRelease(pType);
		}

	}
	while (false);

	return hr;
}

HRESULT WebCameraHelper::SetDeviceFormat(IMFMediaSource *pSource, DWORD dwFormatIndex)
{
	CComPtr<IMFPresentationDescriptor> pPD;
	CComPtr<IMFStreamDescriptor> pSD;
	CComPtr<IMFMediaTypeHandler> pHandler;
	CComPtr<IMFMediaType> pType;
	HRESULT hr = S_OK;

	do 
	{
		hr = pSource->CreatePresentationDescriptor(&pPD);
		BREAK_ON_FAIL(hr);

		BOOL fSelected;
		hr = pPD->GetStreamDescriptorByIndex(0, &fSelected, &pSD);
		BREAK_ON_FAIL(hr);

		hr = pSD->GetMediaTypeHandler(&pHandler);
		BREAK_ON_FAIL(hr);

		hr = pHandler->GetMediaTypeByIndex(dwFormatIndex, &pType);
		BREAK_ON_FAIL(hr);

		hr = pHandler->SetCurrentMediaType(pType);

	} while (false);

	return hr;
}

HRESULT WebCameraHelper::SetFrameRate(IMFMediaSource *pSource, DWORD dwTypeIndex, unsigned int fRate)
{
	CComPtr<IMFPresentationDescriptor> pPD;
	CComPtr<IMFStreamDescriptor> pSD;
	CComPtr<IMFMediaTypeHandler> pHandler;
	CComPtr<IMFMediaType> pType;
	HRESULT hr = S_OK;

	do 
	{
		hr = pSource->CreatePresentationDescriptor(&pPD);
		BREAK_ON_FAIL(hr);

		BOOL fSelected;
		hr = pPD->GetStreamDescriptorByIndex(dwTypeIndex, &fSelected, &pSD);
		BREAK_ON_FAIL(hr);

		hr = pSD->GetMediaTypeHandler(&pHandler);
		BREAK_ON_FAIL(hr);

		hr = pHandler->GetCurrentMediaType(&pType);
		BREAK_ON_FAIL(hr);

		// Get the maximum frame rate for the selected capture format.
		// Note: To get the minimum frame rate, use the 
		// MF_MT_FRAME_RATE_RANGE_MIN attribute instead.

		PROPVARIANT var;
		if (SUCCEEDED(pType->GetItem(MF_MT_FRAME_RATE_RANGE_MAX, &var)))
		{
			// If requested frame rate is less then MAX set it or just set MAX
			if (var.uhVal.HighPart > fRate)
			{
				var.uhVal.HighPart = fRate;
			}

			hr = pType->SetItem(MF_MT_FRAME_RATE, var);

			PropVariantClear(&var);
			BREAK_ON_FAIL(hr);

//			hr = MFSetAttributeRatio(pType, MF_MT_FRAME_RATE, 1, fRate);
			BREAK_ON_FAIL(hr);
			hr = pHandler->SetCurrentMediaType(pType);
		}
	} while (false);

	return hr;
}

HRESULT WebCameraHelper::LogMediaType(IMFMediaType *pType, AttrList& attrList)
{
	UINT32 count = 0;
	HRESULT hr = S_OK;

	do
	{
		hr = pType->GetCount(&count);
		BREAK_ON_FAIL(hr);

		if (count == 0)
		{
			DBGMSG(L"Empty media type.\n");
		}

		for (UINT32 i = 0; i < count; i++)
		{
			hr = LogAttributeValueByIndex(pType, attrList, i);
			BREAK_ON_FAIL(hr);
		}
	}
	while(false);

	return hr;
}

HRESULT WebCameraHelper::LogAttributeValueByIndex(IMFAttributes *pAttr, AttrList& attrList, unsigned int index)
{
	wstring pGuidName;
	wstring pGuidValName;
	HRESULT hr = S_OK;
	GUID guid = { 0 };

	PROPVARIANT var;
	PropVariantInit(&var);

	do 
	{
		bool shouldAdd = false;
		hr = pAttr->GetItemByIndex(index, &guid, &var);
		BREAK_ON_FAIL(hr);

		hr = GetGUIDName(guid, pGuidName);
		BREAK_ON_FAIL(hr);

		DBGMSG(L"\t%s\t", pGuidName.c_str());

		PairedAttr valPair;
		hr = SpecialCaseAttributeValue(guid, var, valPair);
		BREAK_ON_FAIL(hr);

		VariantAttr varVal;  

		if (hr == S_FALSE)
		{
			switch (var.vt)
			{
			case VT_UI4:
				DBGMSG(L"%d", var.ulVal);
				varVal = var.ulVal;
				shouldAdd = true;
				break;

			case VT_UI8:
				DBGMSG(L"%I64d", var.uhVal);
				varVal = var.uhVal;
				shouldAdd = true;
				break;

			case VT_R8:
				DBGMSG(L"%f", var.dblVal);
				varVal = var.dblVal;
				shouldAdd = true;
				break;

			case VT_CLSID:
				hr = GetGUIDName(*var.puuid, pGuidValName);
				if (SUCCEEDED(hr))
				{
					varVal = pGuidValName;
					shouldAdd = true;
					DBGMSG(pGuidValName.c_str());
				}
				break;

			case VT_LPWSTR:
				DBGMSG(var.pwszVal);
				break;

			case VT_VECTOR | VT_UI1:
				DBGMSG(L"<<byte array>>");
				break;

			case VT_UNKNOWN:
				DBGMSG(L"IUnknown");
				break;

			default:
				DBGMSG(L"Unexpected attribute type (vt = %d)", var.vt);
				break;
			}
		}
		else
		{
			varVal = valPair;
			shouldAdd = true;
		}

		if (shouldAdd)
		{
			auto newPair = pair<wstring, VariantAttr>(pGuidName, varVal);
			attrList.insert(newPair);
		}
	} 
	while (false);


	DBGMSG(L"\n");
	PropVariantClear(&var);
	return hr;
}

HRESULT WebCameraHelper::GetGUIDName(const GUID& guid, wstring& guidName)
{
	HRESULT hr = S_OK;
	WCHAR *pName = NULL;

	LPCWSTR pcwsz = GetGUIDNameConst(guid);
	if (pcwsz)
	{
		size_t cchLength = 0;

		hr = StringCchLength(pcwsz, STRSAFE_MAX_CCH, &cchLength);
		if (FAILED(hr))
		{
			goto done;
		}

		pName = (WCHAR*)CoTaskMemAlloc((cchLength + 1) * sizeof(WCHAR));

		if (pName == NULL)
		{
			hr = E_OUTOFMEMORY;
			goto done;
		}

		hr = StringCchCopy(pName, cchLength + 1, pcwsz);
		if (FAILED(hr))
		{
			goto done;
		}
	}
	else
	{
		hr = StringFromCLSID(guid, &pName);
	}

done:
	if (!FAILED(hr))
	{
		guidName = pName;
	}
	CoTaskMemFree(pName);

	return hr;
}

void WebCameraHelper::LogUINT32AsUINT64(const PROPVARIANT& var, PairedAttr& valPair)
{
	UINT32 uHigh = 0, uLow = 0;
	Unpack2UINT32AsUINT64(var.uhVal.QuadPart, &uHigh, &uLow);
	valPair = make_pair(uHigh, uLow);
	DBGMSG(L"%d x %d", uHigh, uLow);
}

float WebCameraHelper::OffsetToFloat(const MFOffset& offset)
{
	return offset.value + (static_cast<float>(offset.fract) / 65536.0f);
}

HRESULT WebCameraHelper::LogVideoArea(const PROPVARIANT& var)
{
	if (var.caub.cElems < sizeof(MFVideoArea))
	{
		return MF_E_BUFFERTOOSMALL;
	}

	MFVideoArea *pArea = (MFVideoArea*)var.caub.pElems;

	DBGMSG(L"(%f,%f) (%d,%d)", OffsetToFloat(pArea->OffsetX), OffsetToFloat(pArea->OffsetY), 
		pArea->Area.cx, pArea->Area.cy);
	return S_OK;
}

// Handle certain known special cases.
HRESULT WebCameraHelper::SpecialCaseAttributeValue(GUID guid, const PROPVARIANT& var, PairedAttr& valPair)
{
	if ((guid == MF_MT_FRAME_RATE) || (guid == MF_MT_FRAME_RATE_RANGE_MAX) ||
		(guid == MF_MT_FRAME_RATE_RANGE_MIN) || (guid == MF_MT_FRAME_SIZE) ||
		(guid == MF_MT_PIXEL_ASPECT_RATIO))
	{
		// Attributes that contain two packed 32-bit values.
		LogUINT32AsUINT64(var, valPair);
	}
	else if ((guid == MF_MT_GEOMETRIC_APERTURE) || 
		(guid == MF_MT_MINIMUM_DISPLAY_APERTURE) || 
		(guid == MF_MT_PAN_SCAN_APERTURE))
	{
		// Attributes that an MFVideoArea structure.
		return LogVideoArea(var);
	}
	else
	{
		return S_FALSE;
	}
	return S_OK;
}

void WebCameraHelper::DBGMSG(PCWSTR format, ...)
{
	va_list args;
	va_start(args, format);

	WCHAR msg[MAX_PATH];

	if (SUCCEEDED(StringCbVPrintf(msg, sizeof(msg), format, args)))
	{
		OutputDebugString(msg);
	}
}

#ifndef IF_EQUAL_RETURN
#define IF_EQUAL_RETURN(param, val) if(val == param) return L#val
#endif

LPCWSTR WebCameraHelper::GetGUIDNameConst(const GUID& guid)
{
	IF_EQUAL_RETURN(guid, MF_MT_MAJOR_TYPE);
	IF_EQUAL_RETURN(guid, MF_MT_MAJOR_TYPE);
	IF_EQUAL_RETURN(guid, MF_MT_SUBTYPE);
	IF_EQUAL_RETURN(guid, MF_MT_ALL_SAMPLES_INDEPENDENT);
	IF_EQUAL_RETURN(guid, MF_MT_FIXED_SIZE_SAMPLES);
	IF_EQUAL_RETURN(guid, MF_MT_COMPRESSED);
	IF_EQUAL_RETURN(guid, MF_MT_SAMPLE_SIZE);
	IF_EQUAL_RETURN(guid, MF_MT_WRAPPED_TYPE);
	IF_EQUAL_RETURN(guid, MF_MT_AUDIO_NUM_CHANNELS);
	IF_EQUAL_RETURN(guid, MF_MT_AUDIO_SAMPLES_PER_SECOND);
	IF_EQUAL_RETURN(guid, MF_MT_AUDIO_FLOAT_SAMPLES_PER_SECOND);
	IF_EQUAL_RETURN(guid, MF_MT_AUDIO_AVG_BYTES_PER_SECOND);
	IF_EQUAL_RETURN(guid, MF_MT_AUDIO_BLOCK_ALIGNMENT);
	IF_EQUAL_RETURN(guid, MF_MT_AUDIO_BITS_PER_SAMPLE);
	IF_EQUAL_RETURN(guid, MF_MT_AUDIO_VALID_BITS_PER_SAMPLE);
	IF_EQUAL_RETURN(guid, MF_MT_AUDIO_SAMPLES_PER_BLOCK);
	IF_EQUAL_RETURN(guid, MF_MT_AUDIO_CHANNEL_MASK);
	IF_EQUAL_RETURN(guid, MF_MT_AUDIO_FOLDDOWN_MATRIX);
	IF_EQUAL_RETURN(guid, MF_MT_AUDIO_WMADRC_PEAKREF);
	IF_EQUAL_RETURN(guid, MF_MT_AUDIO_WMADRC_PEAKTARGET);
	IF_EQUAL_RETURN(guid, MF_MT_AUDIO_WMADRC_AVGREF);
	IF_EQUAL_RETURN(guid, MF_MT_AUDIO_WMADRC_AVGTARGET);
	IF_EQUAL_RETURN(guid, MF_MT_AUDIO_PREFER_WAVEFORMATEX);
	IF_EQUAL_RETURN(guid, MF_MT_AAC_PAYLOAD_TYPE);
	IF_EQUAL_RETURN(guid, MF_MT_AAC_AUDIO_PROFILE_LEVEL_INDICATION);
	IF_EQUAL_RETURN(guid, MF_MT_FRAME_SIZE);
	IF_EQUAL_RETURN(guid, MF_MT_FRAME_RATE);
	IF_EQUAL_RETURN(guid, MF_MT_FRAME_RATE_RANGE_MAX);
	IF_EQUAL_RETURN(guid, MF_MT_FRAME_RATE_RANGE_MIN);
	IF_EQUAL_RETURN(guid, MF_MT_PIXEL_ASPECT_RATIO);
	IF_EQUAL_RETURN(guid, MF_MT_DRM_FLAGS);
	IF_EQUAL_RETURN(guid, MF_MT_PAD_CONTROL_FLAGS);
	IF_EQUAL_RETURN(guid, MF_MT_SOURCE_CONTENT_HINT);
	IF_EQUAL_RETURN(guid, MF_MT_VIDEO_CHROMA_SITING);
	IF_EQUAL_RETURN(guid, MF_MT_INTERLACE_MODE);
	IF_EQUAL_RETURN(guid, MF_MT_TRANSFER_FUNCTION);
	IF_EQUAL_RETURN(guid, MF_MT_VIDEO_PRIMARIES);
	IF_EQUAL_RETURN(guid, MF_MT_CUSTOM_VIDEO_PRIMARIES);
	IF_EQUAL_RETURN(guid, MF_MT_YUV_MATRIX);
	IF_EQUAL_RETURN(guid, MF_MT_VIDEO_LIGHTING);
	IF_EQUAL_RETURN(guid, MF_MT_VIDEO_NOMINAL_RANGE);
	IF_EQUAL_RETURN(guid, MF_MT_GEOMETRIC_APERTURE);
	IF_EQUAL_RETURN(guid, MF_MT_MINIMUM_DISPLAY_APERTURE);
	IF_EQUAL_RETURN(guid, MF_MT_PAN_SCAN_APERTURE);
	IF_EQUAL_RETURN(guid, MF_MT_PAN_SCAN_ENABLED);
	IF_EQUAL_RETURN(guid, MF_MT_AVG_BITRATE);
	IF_EQUAL_RETURN(guid, MF_MT_AVG_BIT_ERROR_RATE);
	IF_EQUAL_RETURN(guid, MF_MT_MAX_KEYFRAME_SPACING);
	IF_EQUAL_RETURN(guid, MF_MT_DEFAULT_STRIDE);
	IF_EQUAL_RETURN(guid, MF_MT_PALETTE);
	IF_EQUAL_RETURN(guid, MF_MT_USER_DATA);
	IF_EQUAL_RETURN(guid, MF_MT_AM_FORMAT_TYPE);
	IF_EQUAL_RETURN(guid, MF_MT_MPEG_START_TIME_CODE);
	IF_EQUAL_RETURN(guid, MF_MT_MPEG2_PROFILE);
	IF_EQUAL_RETURN(guid, MF_MT_MPEG2_LEVEL);
	IF_EQUAL_RETURN(guid, MF_MT_MPEG2_FLAGS);
	IF_EQUAL_RETURN(guid, MF_MT_MPEG_SEQUENCE_HEADER);
	IF_EQUAL_RETURN(guid, MF_MT_DV_AAUX_SRC_PACK_0);
	IF_EQUAL_RETURN(guid, MF_MT_DV_AAUX_CTRL_PACK_0);
	IF_EQUAL_RETURN(guid, MF_MT_DV_AAUX_SRC_PACK_1);
	IF_EQUAL_RETURN(guid, MF_MT_DV_AAUX_CTRL_PACK_1);
	IF_EQUAL_RETURN(guid, MF_MT_DV_VAUX_SRC_PACK);
	IF_EQUAL_RETURN(guid, MF_MT_DV_VAUX_CTRL_PACK);
	IF_EQUAL_RETURN(guid, MF_MT_ARBITRARY_HEADER);
	IF_EQUAL_RETURN(guid, MF_MT_ARBITRARY_FORMAT);
	IF_EQUAL_RETURN(guid, MF_MT_IMAGE_LOSS_TOLERANT); 
	IF_EQUAL_RETURN(guid, MF_MT_MPEG4_SAMPLE_DESCRIPTION);
	IF_EQUAL_RETURN(guid, MF_MT_MPEG4_CURRENT_SAMPLE_ENTRY);
	IF_EQUAL_RETURN(guid, MF_MT_ORIGINAL_4CC); 
	IF_EQUAL_RETURN(guid, MF_MT_ORIGINAL_WAVE_FORMAT_TAG);

	// Media types

	IF_EQUAL_RETURN(guid, MFMediaType_Audio);
	IF_EQUAL_RETURN(guid, MFMediaType_Video);
	IF_EQUAL_RETURN(guid, MFMediaType_Protected);
	IF_EQUAL_RETURN(guid, MFMediaType_SAMI);
	IF_EQUAL_RETURN(guid, MFMediaType_Script);
	IF_EQUAL_RETURN(guid, MFMediaType_Image);
	IF_EQUAL_RETURN(guid, MFMediaType_HTML);
	IF_EQUAL_RETURN(guid, MFMediaType_Binary);
	IF_EQUAL_RETURN(guid, MFMediaType_FileTransfer);

	IF_EQUAL_RETURN(guid, MFVideoFormat_AI44); //     FCC('AI44')
	IF_EQUAL_RETURN(guid, MFVideoFormat_ARGB32); //   D3DFMT_A8R8G8B8 
	IF_EQUAL_RETURN(guid, MFVideoFormat_AYUV); //     FCC('AYUV')
	IF_EQUAL_RETURN(guid, MFVideoFormat_DV25); //     FCC('dv25')
	IF_EQUAL_RETURN(guid, MFVideoFormat_DV50); //     FCC('dv50')
	IF_EQUAL_RETURN(guid, MFVideoFormat_DVH1); //     FCC('dvh1')
	IF_EQUAL_RETURN(guid, MFVideoFormat_DVSD); //     FCC('dvsd')
	IF_EQUAL_RETURN(guid, MFVideoFormat_DVSL); //     FCC('dvsl')
	IF_EQUAL_RETURN(guid, MFVideoFormat_H264); //     FCC('H264')
	IF_EQUAL_RETURN(guid, MFVideoFormat_I420); //     FCC('I420')
	IF_EQUAL_RETURN(guid, MFVideoFormat_IYUV); //     FCC('IYUV')
	IF_EQUAL_RETURN(guid, MFVideoFormat_M4S2); //     FCC('M4S2')
	IF_EQUAL_RETURN(guid, MFVideoFormat_MJPG);
	IF_EQUAL_RETURN(guid, MFVideoFormat_MP43); //     FCC('MP43')
	IF_EQUAL_RETURN(guid, MFVideoFormat_MP4S); //     FCC('MP4S')
	IF_EQUAL_RETURN(guid, MFVideoFormat_MP4V); //     FCC('MP4V')
	IF_EQUAL_RETURN(guid, MFVideoFormat_MPG1); //     FCC('MPG1')
	IF_EQUAL_RETURN(guid, MFVideoFormat_MSS1); //     FCC('MSS1')
	IF_EQUAL_RETURN(guid, MFVideoFormat_MSS2); //     FCC('MSS2')
	IF_EQUAL_RETURN(guid, MFVideoFormat_NV11); //     FCC('NV11')
	IF_EQUAL_RETURN(guid, MFVideoFormat_NV12); //     FCC('NV12')
	IF_EQUAL_RETURN(guid, MFVideoFormat_P010); //     FCC('P010')
	IF_EQUAL_RETURN(guid, MFVideoFormat_P016); //     FCC('P016')
	IF_EQUAL_RETURN(guid, MFVideoFormat_P210); //     FCC('P210')
	IF_EQUAL_RETURN(guid, MFVideoFormat_P216); //     FCC('P216')
	IF_EQUAL_RETURN(guid, MFVideoFormat_RGB24); //    D3DFMT_R8G8B8 
	IF_EQUAL_RETURN(guid, MFVideoFormat_RGB32); //    D3DFMT_X8R8G8B8 
	IF_EQUAL_RETURN(guid, MFVideoFormat_RGB555); //   D3DFMT_X1R5G5B5 
	IF_EQUAL_RETURN(guid, MFVideoFormat_RGB565); //   D3DFMT_R5G6B5 
	IF_EQUAL_RETURN(guid, MFVideoFormat_RGB8);
	IF_EQUAL_RETURN(guid, MFVideoFormat_UYVY); //     FCC('UYVY')
	IF_EQUAL_RETURN(guid, MFVideoFormat_v210); //     FCC('v210')
	IF_EQUAL_RETURN(guid, MFVideoFormat_v410); //     FCC('v410')
	IF_EQUAL_RETURN(guid, MFVideoFormat_WMV1); //     FCC('WMV1')
	IF_EQUAL_RETURN(guid, MFVideoFormat_WMV2); //     FCC('WMV2')
	IF_EQUAL_RETURN(guid, MFVideoFormat_WMV3); //     FCC('WMV3')
	IF_EQUAL_RETURN(guid, MFVideoFormat_WVC1); //     FCC('WVC1')
	IF_EQUAL_RETURN(guid, MFVideoFormat_Y210); //     FCC('Y210')
	IF_EQUAL_RETURN(guid, MFVideoFormat_Y216); //     FCC('Y216')
	IF_EQUAL_RETURN(guid, MFVideoFormat_Y410); //     FCC('Y410')
	IF_EQUAL_RETURN(guid, MFVideoFormat_Y416); //     FCC('Y416')
	IF_EQUAL_RETURN(guid, MFVideoFormat_Y41P);
	IF_EQUAL_RETURN(guid, MFVideoFormat_Y41T);
	IF_EQUAL_RETURN(guid, MFVideoFormat_YUY2); //     FCC('YUY2')
	IF_EQUAL_RETURN(guid, MFVideoFormat_YV12); //     FCC('YV12')
	IF_EQUAL_RETURN(guid, MFVideoFormat_YVYU);

	IF_EQUAL_RETURN(guid, MFAudioFormat_PCM); //              WAVE_FORMAT_PCM 
	IF_EQUAL_RETURN(guid, MFAudioFormat_Float); //            WAVE_FORMAT_IEEE_FLOAT 
	IF_EQUAL_RETURN(guid, MFAudioFormat_DTS); //              WAVE_FORMAT_DTS 
	IF_EQUAL_RETURN(guid, MFAudioFormat_Dolby_AC3_SPDIF); //  WAVE_FORMAT_DOLBY_AC3_SPDIF 
	IF_EQUAL_RETURN(guid, MFAudioFormat_DRM); //              WAVE_FORMAT_DRM 
	IF_EQUAL_RETURN(guid, MFAudioFormat_WMAudioV8); //        WAVE_FORMAT_WMAUDIO2 
	IF_EQUAL_RETURN(guid, MFAudioFormat_WMAudioV9); //        WAVE_FORMAT_WMAUDIO3 
	IF_EQUAL_RETURN(guid, MFAudioFormat_WMAudio_Lossless); // WAVE_FORMAT_WMAUDIO_LOSSLESS 
	IF_EQUAL_RETURN(guid, MFAudioFormat_WMASPDIF); //         WAVE_FORMAT_WMASPDIF 
	IF_EQUAL_RETURN(guid, MFAudioFormat_MSP1); //             WAVE_FORMAT_WMAVOICE9 
	IF_EQUAL_RETURN(guid, MFAudioFormat_MP3); //              WAVE_FORMAT_MPEGLAYER3 
	IF_EQUAL_RETURN(guid, MFAudioFormat_MPEG); //             WAVE_FORMAT_MPEG 
	IF_EQUAL_RETURN(guid, MFAudioFormat_AAC); //              WAVE_FORMAT_MPEG_HEAAC 
	IF_EQUAL_RETURN(guid, MFAudioFormat_ADTS); //             WAVE_FORMAT_MPEG_ADTS_AAC 

	return NULL;
}

