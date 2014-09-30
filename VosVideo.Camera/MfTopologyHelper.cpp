#include "stdafx.h"
#include <Wmcodecdsp.h>
#include <vosvideocommon/ComHelper.h>
#include <webm/include/webmtypes.h>
#include <VP8CallbackSink/VP8CallbackSinkGuid.h>
#include <VP8FileSink/VP8FileSinkGuid.h>
#include <VP8FileSink/VP8FileSink.h>
#include <VP8Encoder/VP8EncoderGuid.h>
#include "VosVideo.Data/CameraConfMsg.h"
#include "MfTopologyHelper.h"

using namespace vosvideo::camera;

MfTopologyHelper::MfTopologyHelper()
{
}


MfTopologyHelper::~MfTopologyHelper()
{
}

HRESULT MfTopologyHelper::CreateMpeg4Decoder(IMFTransform** ppTransf)
{
	HRESULT hr = S_OK;
	MFT_REGISTER_TYPE_INFO inputFilter = { MFMediaType_Video, MFVideoFormat_MP4V};
	hr = CreateDecoder(ppTransf, inputFilter);
	return hr;
}

HRESULT MfTopologyHelper::CreateMjpegDecoder(IMFTransform** ppTransf)
{
	HRESULT hr = S_OK;
	MFT_REGISTER_TYPE_INFO inputFilter = { MFMediaType_Video, MFVideoFormat_MJPG};
	hr = CreateDecoder(ppTransf, inputFilter);
	return hr;
}

HRESULT MfTopologyHelper::CreateH264Decoder(IMFTransform** ppTransf)
{
	HRESULT hr = S_OK;
	MFT_REGISTER_TYPE_INFO inputFilter = { MFMediaType_Video, MFVideoFormat_H264};
	hr = CreateDecoder(ppTransf, inputFilter);
	return hr;
}

HRESULT MfTopologyHelper::CreateDecoder(IMFTransform** ppTransf, MFT_REGISTER_TYPE_INFO& inputFilter)
{
	HRESULT hr = S_OK;

	UINT32 unFlags = MFT_ENUM_FLAG_SYNCMFT | MFT_ENUM_FLAG_LOCALMFT | MFT_ENUM_FLAG_SORTANDFILTER;
	IMFActivate **ppActivate = nullptr;

	UINT32 numMft = 0;

	do 
	{
		hr = MFTEnumEx(MFT_CATEGORY_VIDEO_DECODER, unFlags, &inputFilter, NULL, &ppActivate, &numMft);
		BREAK_ON_FAIL(hr);

		if (numMft > 0)
		{
			// Activate transform
			hr = ppActivate[0]->ActivateObject(IID_PPV_ARGS(ppTransf));
		}

		// cleanup
		for (UINT32 i = 0; i < numMft; i++)
		{
			ppActivate[i]->Release();
		}
		CoTaskMemFree(ppActivate);
	}
	while(false);

	return hr;
}

HRESULT MfTopologyHelper::NegotiateSourceToMft(CComQIPtr<IMFMediaSource> pSource, CComPtr<IMFTransform> pMft)
{
	HRESULT hr = S_OK;
	CComPtr<IMFPresentationDescriptor> pPresDescriptor;
	CComPtr<IMFStreamDescriptor> pStreamDescriptor;
	CComPtr<IMFMediaTypeHandler> pMediaTypeHandler;
	CComPtr<IMFMediaType> pMediaType;
	BOOL streamSelected = FALSE;
	DWORD decoderInputStreamId = 0;

	DWORD sourceTypesCount = 0;

	do
	{
		BREAK_ON_NULL(pMft, E_UNEXPECTED);

		// get the presentation descriptor for the source
		hr = pSource->CreatePresentationDescriptor(&pPresDescriptor);
		BREAK_ON_FAIL(hr);

		// get the stream descriptor for the first stream
		hr = pPresDescriptor->GetStreamDescriptorByIndex(0, &streamSelected, 
			&pStreamDescriptor);
		BREAK_ON_FAIL(hr);

		// get the media type handler for the source
		hr = pStreamDescriptor->GetMediaTypeHandler(&pMediaTypeHandler);
		BREAK_ON_FAIL(hr);

		// get the number of media types that are exposed by the source stream
		hr = pMediaTypeHandler->GetMediaTypeCount(&sourceTypesCount);
		BREAK_ON_FAIL(hr);        

		// go through every media type exposed by the source, and try each one with the sink
		for(DWORD x = 0; x < sourceTypesCount; x++)
		{
			pMediaType = nullptr;

			// get a media type from the source by index
			hr = pMediaTypeHandler->GetMediaTypeByIndex(x, &pMediaType);
			BREAK_ON_FAIL(hr);

			// try to set the input media type on the decoder - assume that the input stream
			// ID is 0, since this is a well-known MFT
			hr = pMft->SetInputType(0, pMediaType, 0);
			if(SUCCEEDED(hr))
			{
				// if the decoder accepted the input media type, set it on the source
				hr = pMediaTypeHandler->SetCurrentMediaType(pMediaType);
				BREAK_ON_FAIL(hr);
				break;
			}
		}

		// if the type was found, hr will be S_OK - otherwise hr will indicate a failure to
		// either get the media type by index, set it on the decoder, or set it on the 
		// media type handler
		BREAK_ON_FAIL(hr);

		// if the source stream is not activated, activate it
		if(!streamSelected)
		{
			hr = pPresDescriptor->SelectStream(0);
		}
	}
	while(false);

	return hr;
}


HRESULT MfTopologyHelper::NegotiateMftToVP8(CComPtr<IMFTransform> pMft1, CComPtr<IMFTransform> pMft2, webrtc::VideoCaptureCapability& webRtcCapability)
{
	HRESULT hr = S_OK;
	CComPtr<IMFMediaType> pInputMediaType;
	DWORD mft1OutputStreamId = 0;
	DWORD mft2InputStreamId = 0;

	DWORD mft1TypeIndex = 0;

	do
	{
		BREAK_ON_NULL(pMft1, E_UNEXPECTED);
		BREAK_ON_NULL(pMft2, E_UNEXPECTED);

		// loop through all of the available output types exposed by the upstream MFT, and
		// try each of them as the input type of the downstream MFT.
		while(true)
		{
			pInputMediaType = nullptr;

			// get the type with the mftTypeIndex index from the upstream MFT
			hr = pMft1->GetOutputAvailableType(mft1OutputStreamId, mft1TypeIndex++, &pInputMediaType);
			BREAK_ON_FAIL(hr);

			// if we succeeded, set the output type on the upstream component
			hr = pInputMediaType->SetUINT32(MF_MT_INTERLACE_MODE, MFVideoInterlace_Progressive);   
			BREAK_ON_FAIL(hr);
			hr = pInputMediaType->SetUINT32(MF_MT_AVG_BITRATE, 500000);   			
			BREAK_ON_FAIL(hr);
			hr = pMft1->SetOutputType(mft1OutputStreamId, pInputMediaType, 0);
			BREAK_ON_FAIL(hr);

			GUID g;
			pInputMediaType->GetGUID(MF_MT_SUBTYPE, &g);
			BREAK_ON_FAIL(hr);
			// Configure encoder input
			hr = pMft2->SetInputType(mft2InputStreamId, pInputMediaType, 0);
			BREAK_ON_FAIL(hr);

			UINT32 w,h;
			hr = MFGetAttributeSize(pInputMediaType, MF_MT_FRAME_SIZE, &w, &h);
			BREAK_ON_FAIL(hr);
			webRtcCapability.width = w;
			webRtcCapability.height = h;

			UINT32 br=0;
			hr = pInputMediaType->GetUINT32(MF_MT_INTERLACE_MODE, &br);
			BREAK_ON_FAIL(hr);
			if (br == MFVideoInterlace_Progressive)
			{
				webRtcCapability.interlaced = true;
			}
			hr = pMft2->SetInputType(mft2InputStreamId, pInputMediaType, 0);
			BREAK_ON_FAIL(hr);

			// Configure encoder output
			CComPtr<IMFMediaType> pOutMediaType;
			MFCreateMediaType(&pOutMediaType);
			hr = pOutMediaType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
			BREAK_ON_FAIL(hr);
			hr = pOutMediaType->SetGUID(MF_MT_SUBTYPE, MEDIASUBTYPE_VP80);
			BREAK_ON_FAIL(hr);
			webRtcCapability.codecType = webrtc::VideoCodecType::kVideoCodecVP8;
			hr = pOutMediaType->SetUINT32(MF_MT_AVG_BITRATE, 500000);   
			BREAK_ON_FAIL(hr);
			hr = MFSetAttributeSize(pOutMediaType, MF_MT_FRAME_SIZE, w, h);   
			BREAK_ON_FAIL(hr);
			hr = pOutMediaType->SetUINT32(MF_MT_INTERLACE_MODE, MFVideoInterlace_Progressive);   
			BREAK_ON_FAIL(hr);
			hr = MFSetAttributeRatio(pOutMediaType, MF_MT_FRAME_RATE, 20, 1);   
			BREAK_ON_FAIL(hr);
			webRtcCapability.maxFPS = 20;

			hr = pMft2->SetOutputType(mft2InputStreamId, pOutMediaType, 0);

			if(SUCCEEDED(hr))
			{
				break;
			}
		}
		BREAK_ON_FAIL(hr);
	}
	while(false);

	return hr;
}

// Find matching media type for two MFTs, and set it
HRESULT MfTopologyHelper::NegotiateYUY2ToI420(CComPtr<IMFTransform> pMft1, CComPtr<IMFTransform> pMft2)
{
	HRESULT hr = S_OK;
	CComPtr<IMFMediaType> pMediaType;
	DWORD mft1OutputStreamId = 0;
	DWORD mft2InputStreamId = 0;

	DWORD mft1TypeIndex = 0;

	do
	{
		BREAK_ON_NULL(pMft1, E_UNEXPECTED);
		BREAK_ON_NULL(pMft2, E_UNEXPECTED);

		// loop through all of the available output types exposed by the upstream MFT, and
		// try each of them as the input type of the downstream MFT.
		while(true)
		{
			pMediaType = nullptr;

			// get the type with the mftTypeIndex index from the upstream MFT
			hr = pMft1->GetOutputAvailableType(mft1OutputStreamId, mft1TypeIndex++, &pMediaType);
			BREAK_ON_FAIL(hr);

			// if we succeeded, set the output type on the upstream component
			hr = pMft1->SetOutputType(mft1OutputStreamId, pMediaType, 0);
			BREAK_ON_FAIL(hr);

			CComPtr<IMFMediaType> pOutType;
			MFCreateMediaType(&pOutType);
			hr = pOutType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
			hr = pOutType->SetGUID(MF_MT_SUBTYPE, MEDIASUBTYPE_I420);

			hr = pMft2->SetOutputType(mft2InputStreamId, pOutType, 0);
			hr = pMft2->SetInputType(mft2InputStreamId, pMediaType, 0);

			if(SUCCEEDED(hr))
			{
				break;
			}
		}
		BREAK_ON_FAIL(hr);
	}
	while(false);

	return hr;
}

HRESULT MfTopologyHelper::CreateYuYtoI420ColorTransf(IMFTransform** ppTransf)
{
	HRESULT hr = S_OK;
	CComPtr<IMFTransform> pTransf;

	hr = pTransf.CoCreateInstance(CLSID_CColorConvertDMO);
	*ppTransf = pTransf.Detach();

	return hr;
}

HRESULT MfTopologyHelper::CreateVP8Encoder(IMFTransform** ppTransf)
{
	HRESULT hr = S_OK;
	CComPtr<IMFTransform> pTransf;

	hr = pTransf.CoCreateInstance(CLSID_VP8EncoderMFT);
	*ppTransf = pTransf.Detach();

	return hr;
}


HRESULT MfTopologyHelper::CreateVP8CallbackSink(IMFMediaSink** ppSink)
{
	HRESULT hr = S_OK;
	CComPtr<IMFMediaSink> pSink;

	hr = pSink.CoCreateInstance(CLSID_VP8CallbackSink);	

	*ppSink = pSink.Detach();
	return hr;
}

HRESULT MfTopologyHelper::CreateVP8FileSink(IMFMediaSink** ppSink)
{
	HRESULT hr = S_OK;
	CComPtr<IMFMediaSink> pSink;

	hr = pSink.CoCreateInstance(CLSID_VP8FileSink);

	*ppSink = pSink.Detach();

	return hr;
}

//
//  Create a source node for the specified stream
//
//  pPresDescriptor: Presentation descriptor for the media source.
//  pStreamDescriptor: Stream descriptor for the stream.
//  pNode: Reference to a pointer to the new node - returns the new node.
//
HRESULT MfTopologyHelper::CreateSourceStreamNode(
	CComQIPtr<IMFMediaSource> pSource,
	CComPtr<IMFPresentationDescriptor> pPresDescriptor,
	CComPtr<IMFStreamDescriptor> pStreamDescriptor,
	CComPtr<IMFTopologyNode> &pNode)
{
	HRESULT hr = S_OK;

	do
	{
		BREAK_ON_NULL(pPresDescriptor, E_POINTER);
		BREAK_ON_NULL(pStreamDescriptor, E_POINTER);

		// Create the topology node, indicating that it must be a source node.
		hr = MFCreateTopologyNode(MF_TOPOLOGY_SOURCESTREAM_NODE, &pNode);
		BREAK_ON_FAIL(hr);

		// Associate the node with the source by passing in a pointer to the media source,
		// and indicating that it is the source
		hr = pNode->SetUnknown(MF_TOPONODE_SOURCE, pSource);
		BREAK_ON_FAIL(hr);

		// Set the node presentation descriptor attribute of the node by passing 
		// in a pointer to the presentation descriptor
		hr = pNode->SetUnknown(MF_TOPONODE_PRESENTATION_DESCRIPTOR, pPresDescriptor);
		BREAK_ON_FAIL(hr);

		// Set the node stream descriptor attribute by passing in a pointer to the stream
		// descriptor
		hr = pNode->SetUnknown(MF_TOPONODE_STREAM_DESCRIPTOR, pStreamDescriptor);
		BREAK_ON_FAIL(hr);
	}
	while(false);

	return hr;
}

