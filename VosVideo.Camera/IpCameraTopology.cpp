#include "stdafx.h"
#include <boost/format.hpp>
#include <vosvideocommon/ComHelper.h>
#include <vosvideocommon/NativeErrorsManager.h>
#include <Codecapi.h>
#include "NetCredentialManager.h"
#include "MfTopologyHelper.h"
#include "IpCameraTopology.h"

using namespace std;
using namespace concurrency;
using boost::wformat;
using namespace util;
using namespace vosvideo::camera;
using namespace vosvideo::data;

IpCameraTopology::IpCameraTopology()
{
}

IpCameraTopology::~IpCameraTopology()
{
}

HRESULT IpCameraTopology::RenderUrlAsync(const CameraConfMsg& conf, boost::signal<void(HRESULT, shared_ptr<SendData>)>::slot_function_type subscriber)
{
	conf_ = conf;
	openCompletedSignal_.connect(subscriber);

	wstring audiouri;
	wstring videouri;
	conf_.GetUris(audiouri, videouri);

	wstring username, pass;
	conf_.GetCredentials(username, pass);

	HRESULT hr = CreateMediaSource(videouri, username, pass);

	return hr;
}

HRESULT IpCameraTopology::CreateMediaSource(wstring& sURL, wstring& username, wstring& pass)
{
	HRESULT hr = S_OK;
	MF_OBJECT_TYPE objectType = MF_OBJECT_INVALID;

	do
	{
		// Create the source resolver.
		if (pSourceResolver_ == nullptr)
		{
			hr = MFCreateSourceResolver(&pSourceResolver_);
			BREAK_ON_FAIL(hr);
		}

		CComPtr<IPropertyStore> pPropStore;
		NetCredentialManager *pCredentials = new (std::nothrow) NetCredentialManager(username, pass);
		// Configure the property store.
		hr = PSCreateMemoryPropertyStore(IID_PPV_ARGS(&pPropStore));
		if (SUCCEEDED(hr))
		{
			PROPERTYKEY key;
			key.fmtid =  MFNETSOURCE_CREDENTIAL_MANAGER;
			key.pid = 0;

			PROPVARIANT var;
			var.vt = VT_UNKNOWN;
			pCredentials->QueryInterface(IID_PPV_ARGS(&var.punkVal));

			hr = pPropStore->SetValue(key, var);

			PropVariantClear(&var);
		}

		CComPtr<IUnknown> pCancelCookie;
		LOG_TRACE("Try to open URL: " << sURL);

		hr = pSourceResolver_->BeginCreateObjectFromURL(
			sURL.c_str(),               // URL of the source.
			MF_RESOLUTION_MEDIASOURCE | 
			MF_RESOLUTION_CONTENT_DOES_NOT_HAVE_TO_MATCH_EXTENSION_OR_MIME_TYPE,  
			pPropStore,                 // Optional property store for extra parameters
			&pCancelCookie,
			this,
			NULL);
		BREAK_ON_FAIL(hr);

		pCancelCookie_ = pCancelCookie.Detach();

		// Open source cancellation, PPL async timer executes Cancel routine after timeout
		auto callback = new call<CameraTopology*>([this](CameraTopology*)
		{
			// If after 5 sec camera source was not created cancel it
			if (this->pSource_ == nullptr) 
			{
				int cameraId = -1;
				wstring cameraName;
				conf_.GetCameraIds(cameraId, cameraName);
				wstring errMsg = str(wformat(L"Timeout %1% ms for camera %2%. Cancel connection creation.") % openConnTimeout_ % cameraName); 
				LOG_ERROR(errMsg);
				HRESULT hr = this->pSourceResolver_->CancelObjectCreation(this->pCancelCookie_);
				LOG_DEBUG("CancelObjectCreation returned HR=" << hr);
			}
		});
		// Connect the timer to the callback and start the timer.
		fireCancelTimer_ = new Concurrency::timer<CameraTopology*>(openConnTimeout_, 0, nullptr, false);
		fireCancelTimer_->link_target(callback);
		fireCancelTimer_->start();
	}
	while(false);

	return hr;
}

HRESULT IpCameraTopology::Invoke(IMFAsyncResult* pAsyncResult)
{
	HRESULT hr = S_OK;
	shared_ptr<RtbcDeviceErrorOutMsg> errMsg;
	MF_OBJECT_TYPE objType;

	// In case error will use this info
	int cameraId;
	wstring cameraName;
	wstring failedComponent;
	conf_.GetCameraIds(cameraId, cameraName);

	do
	{
		CComPtr<IUnknown> pSource;
		hr = pSourceResolver_->EndCreateObjectFromURL(pAsyncResult, &objType, &pSource);
		// Get the IMFMediaSource interface from the media source.
		pSource_ = pSource.Detach();

		if (hr != S_OK || pSource_ == nullptr)
		{
			// Too generic, here is timeout I know for sure
			if (hr == E_ABORT)
			{
				hr = WAIT_TIMEOUT;
			}
			break;
		}

		if (conf_.GetVideoFormat() == CameraVideoFormat::MPEG4)
		{
			hr = MfTopologyHelper::CreateMpeg4Decoder(&pDecoder_);
			if (hr != S_OK)
			{
				failedComponent = L"Failed to create Mpeg4 decoder.";
			}
			BREAK_ON_FAIL(hr);
		}
		else if (conf_.GetVideoFormat() == CameraVideoFormat::MJPEG)
		{
			hr = MfTopologyHelper::CreateMjpegDecoder(&pDecoder_);
			if (hr != S_OK)
			{
				failedComponent = L"Failed to create MJPEG decoder.";
			}
			BREAK_ON_FAIL(hr);
		}
		else if (conf_.GetVideoFormat() == CameraVideoFormat::H264)
		{
			hr = MfTopologyHelper::CreateH264Decoder(&pDecoder_);
			if (hr != S_OK)
			{
				failedComponent = L"Failed to create h264 decoder.";
			}
			// Make sure that acceleration is ON
			CComPtr<IMFAttributes> attr;
			pDecoder_->GetAttributes(&attr);
			hr = attr->SetUINT32(CODECAPI_AVDecVideoAcceleration_H264, TRUE);
			BREAK_ON_FAIL(hr);
		}
		else 
		{
			hr = S_FALSE;
			break;
		}

		hr = MfTopologyHelper::NegotiateSourceToMft(pSource_, pDecoder_);
		BREAK_ON_FAIL(hr);

		hr = MfTopologyHelper::CreateVP8Encoder(&pVP8Encoder_);
		if (hr != S_OK)
		{
			failedComponent = L"Failed to create VP8 encoder.";
		}
		BREAK_ON_FAIL(hr);

		hr = MfTopologyHelper::CreateYuYtoI420ColorTransf(&pYuYtoI420_);
		if (hr != S_OK)
		{
			failedComponent = L"Failed to create color transformer.";
		}
		BREAK_ON_FAIL(hr);

		hr = MfTopologyHelper::NegotiateYUY2ToI420(pDecoder_, pYuYtoI420_);
		BREAK_ON_FAIL(hr);

		hr = MfTopologyHelper::NegotiateMftToVP8(pYuYtoI420_, pVP8Encoder_, webRtcCapability_);
		BREAK_ON_FAIL(hr);

		wstring outFolder;
		uint32_t recordLen;
		CameraVideoRecording recordingType;
		conf_.GetFileSinkParameters(outFolder, recordLen, recordingType);

		if (recordingType != CameraVideoRecording::DISABLED)
		{
			hr = MfTopologyHelper::CreateVP8FileSink(&pVP8FileSink_);
			if (hr != S_OK)
			{
				failedComponent = L"Failed to create VP8 file sink.";
			}
			BREAK_ON_FAIL(hr);

			if (hr = SetFileSinkParameters(conf_) != S_OK)
			{
				if (hr != S_OK)
				{
					failedComponent = L"Failed to set file sink parameters.";
				}
				BREAK_ON_FAIL(hr);
			}
		}

		hr = MfTopologyHelper::CreateVP8CallbackSink(&pVP8CallbackSink_);
		if (hr != S_OK)
		{
			failedComponent = L"Failed to create VP8 callback sink.";
		}
		BREAK_ON_FAIL(hr);

		hr = CreateTopology(recordingType);

		if (hr == S_OK)
		{
			LOG_TRACE("Successfully created Media Foundation topology for IP Camera: " << cameraName);
		}
	}
	while(false);

	if (hr != S_OK)
	{
		wstring nativeErrMsg = NativeErrorsManager::ToString(hr);
		wstring msg = str(wformat(L"Failed to create media source for camera named: %1%. %2% %3%") % cameraName % failedComponent % nativeErrMsg); 
		errMsg.reset(new RtbcDeviceErrorOutMsg(cameraId, msg, hr));
		LOG_ERROR(msg);
	}

	// Notify player that we are done
	openCompletedSignal_(hr, errMsg);

	SetEvent(invokeCompleteEvent_);
	return hr;
}

//
//  Creates a playback topology from the media source by extracting presentation
//  and stream descriptors from the source, and creating a sink for each of them.
//
HRESULT IpCameraTopology::CreateTopology(CameraVideoRecording recordingType)
{
	HRESULT hr = S_OK;
	CComQIPtr<IMFPresentationDescriptor> pPresDescriptor;
	DWORD nSourceStreams = 0;

	do
	{
		// release the old topology if there was one        
		pTopology_.Release();

		// Create a new topology.
		hr = MFCreateTopology(&pTopology_);
		BREAK_ON_FAIL(hr);

		// Create the presentation descriptor for the media source - a container object that
		// holds a list of the streams and allows selection of streams that will be used.
		hr = pSource_->CreatePresentationDescriptor(&pPresDescriptor);
		BREAK_ON_FAIL(hr);

		// Get the number of streams in the media source
		hr = pPresDescriptor->GetStreamDescriptorCount(&nSourceStreams);
		BREAK_ON_FAIL(hr);

		// Get the stream descriptor for this stream (information about stream).
		BOOL streamSelected = FALSE;
		CComPtr<IMFStreamDescriptor> pStreamDescriptor;
		hr = pPresDescriptor->GetStreamDescriptorByIndex(0, &streamSelected, &pStreamDescriptor);
		BREAK_ON_FAIL(hr);
		CComPtr<IMFTopologyNode> pSourceNode;
		hr = MfTopologyHelper::CreateSourceStreamNode(pSource_, pPresDescriptor, pStreamDescriptor, pSourceNode);
		BREAK_ON_FAIL(hr);
		pTopology_->AddNode(pSourceNode);

		// Create node for MJPEG/MPEG4/H264 decoder
		CComPtr<IMFTopologyNode> pDecoderNode;
		hr = MFCreateTopologyNode(MF_TOPOLOGY_TRANSFORM_NODE, &pDecoderNode);
		BREAK_ON_FAIL(hr);
		// set the output stream ID on the stream sink topology node
		hr = pDecoderNode->SetObject(pDecoder_);
		BREAK_ON_FAIL(hr);
		pTopology_->AddNode(pDecoderNode);

		// Create node for color converter
		CComPtr<IMFTopologyNode> pYuYtoI420Node;
		hr = MFCreateTopologyNode(MF_TOPOLOGY_TRANSFORM_NODE, &pYuYtoI420Node);
		BREAK_ON_FAIL(hr);
		// set the output stream ID on the stream sink topology node
		hr = pYuYtoI420Node->SetObject(pYuYtoI420_);
		BREAK_ON_FAIL(hr);
		pTopology_->AddNode(pYuYtoI420Node);

		// Create node for VP8 encoder
		CComPtr<IMFTopologyNode> pEncoderNode;
		hr = MFCreateTopologyNode(MF_TOPOLOGY_TRANSFORM_NODE, &pEncoderNode);
		BREAK_ON_FAIL(hr);
		// set the output stream ID on the stream sink topology node
		hr = pEncoderNode->SetObject(pVP8Encoder_);
		BREAK_ON_FAIL(hr);
		pTopology_->AddNode(pEncoderNode);

		// Create the topology Tee node
		CComPtr<IMFTopologyNode> pTeeNode;
		hr = MFCreateTopologyNode(MF_TOPOLOGY_TEE_NODE, &pTeeNode);
		BREAK_ON_FAIL(hr);
		pTopology_->AddNode(pTeeNode);

		DWORD streamId = 0;
		// get the stream ID
		hr = pStreamDescriptor->GetStreamIdentifier(&streamId);
		BREAK_ON_FAIL(hr);

		// create the output topology node for one of the streams on the file sink
		CComPtr<IMFTopologyNode> pFileOutputNode;

		if (recordingType != CameraVideoRecording::DISABLED)
		{
			hr = MFCreateTopologyNode(MF_TOPOLOGY_OUTPUT_NODE, &pFileOutputNode);
			// set the output stream ID on the stream sink topology node
			hr = pFileOutputNode->SetUINT32(MF_TOPONODE_STREAMID, streamId);
			CComPtr<IMFStreamSink> pStream1;
			hr = pVP8FileSink_->GetStreamSinkById(0, &pStream1);
			BREAK_ON_FAIL(hr);
			hr = pFileOutputNode->SetObject(pStream1);
			BREAK_ON_FAIL(hr);
			// add the network output topology node to the topology
			pTopology_->AddNode(pFileOutputNode);
			BREAK_ON_FAIL(hr);
		}

		// create the output topology node for one of the streams on the callback sink
		CComPtr<IMFTopologyNode> pCallbackOutputNode;
		hr = MFCreateTopologyNode(MF_TOPOLOGY_OUTPUT_NODE, &pCallbackOutputNode);
		// set the output stream ID on the stream sink topology node
		hr = pCallbackOutputNode->SetUINT32(MF_TOPONODE_STREAMID, streamId);
		CComPtr<IMFStreamSink> pStream2;
		hr = pVP8CallbackSink_->GetStreamSinkById(0, &pStream2);
		BREAK_ON_FAIL(hr);
		hr = pCallbackOutputNode->SetObject(pStream2);
		BREAK_ON_FAIL(hr);
		// add the network output topology node to the topology
		pTopology_->AddNode(pCallbackOutputNode);
		BREAK_ON_FAIL(hr);

		// Connect components
		// CameraSource->MJPEG/MPEG4/H264 decoder
		pSourceNode->ConnectOutput(0, pDecoderNode, 0);
		// MPEG4 decoder -> YuYtoI420 color converter
		pDecoderNode->ConnectOutput(0, pYuYtoI420Node, 0);
		// I420 image to VP8 encoder
		pYuYtoI420Node->ConnectOutput(0, pEncoderNode, 0);
		// VP8 encoder -> TEE 
		pEncoderNode->ConnectOutput(0, pTeeNode, 0);
		if (recordingType != CameraVideoRecording::DISABLED)
		{
			// TEE -> File sink
			pTeeNode->ConnectOutput(0, pFileOutputNode, 0);
			// TEE -> Callback sink
			pTeeNode->ConnectOutput(1, pCallbackOutputNode, 0);
		}
		else
		{
			// TEE -> Callback sink
			pTeeNode->ConnectOutput(0, pCallbackOutputNode, 0);
		}

		// Connect callback for remote requests
		SetFrameCallback();
	}
	while(false);

	return hr;
}
