#include "stdafx.h"
#include <boost/format.hpp>
#include <vosvideocommon/NativeErrorsManager.h>
#include <vosvideocommon/ComHelper.h>
#include "MfTopologyHelper.h"
#include "WebCameraTopology.h"

using namespace std;
using boost::wformat;
using namespace util;
using namespace vosvideo::camera;
using namespace vosvideo::data;


WebCameraTopology::WebCameraTopology()
{
}


WebCameraTopology::~WebCameraTopology()
{
}

HRESULT WebCameraTopology::RenderUrlAsync(const CameraConfMsg& conf, boost::signal<void(HRESULT, shared_ptr<SendData>)>::slot_function_type subscriber)
{
	HRESULT hr = S_OK;
	conf_ = conf;
	openCompletedSignal_.connect(subscriber);

	wstring audiouri;
	wstring videouri;
	conf_.GetUris(audiouri, videouri);

	wstring username, pass;
	hr = CreateMediaSource(videouri, username, pass);

	return hr;
}

HRESULT WebCameraTopology::CreateMediaSource(wstring& sURL, wstring& username, wstring& pass)
{
	BeginCreateMediaSource(sURL, this, nullptr);
	return S_OK;
}

HRESULT WebCameraTopology::BeginCreateMediaSource(wstring& sURL, IMFAsyncCallback *pCB, IUnknown *pState)
{
	sURL_ = sURL;
	CComPtr<IMFAsyncResult> pResult;
	return MFPutWorkItem(MFASYNC_CALLBACK_QUEUE_STANDARD, this, pResult);
}

HRESULT WebCameraTopology::Invoke(IMFAsyncResult* pAsyncResult)
{	 
	HRESULT hr = S_OK;
	shared_ptr<RtbcDeviceErrorOutMsg> errMsg;

	// In case error will use this info
	int cameraId;
	wstring cameraName;
	wstring failedComponent;
	conf_.GetCameraIds(cameraId, cameraName);

	do 
	{
		hr = webCamHelper.CreateVideoCaptureDeviceFromLink(sURL_, &pSource_);
		BREAK_ON_FAIL(hr);

		WebCameraHelper::CaptureFormats cFormats;
		webCamHelper.EnumerateCaptureFormats(pSource_, cFormats);

		// WebCam has many supported formats
		// Find bes one, we prefer YUY2/640x480/15 frames
		int32_t preferedFormat = -1;

		for (unsigned int i = 0; i < cFormats.size(); i++)
		{
			WebCameraHelper::AttrList aList = cFormats.at(i);
			wstring strGuid;
			webCamHelper.GetGUIDName(MF_MT_SUBTYPE, strGuid);
			WebCameraHelper::AttrList::const_iterator attr;

			if ((attr = aList.find(strGuid)) != aList.end())
			{
				wstring str = boost::get<wstring>(attr->second);
				if (str == L"MFVideoFormat_YUY2" )
				{
					webCamHelper.GetGUIDName(MF_MT_FRAME_SIZE, strGuid);
					if ((attr = aList.find(strGuid)) != aList.end())
					{
						WebCameraHelper::PairedAttr frameSize = boost::get<WebCameraHelper::PairedAttr>(attr->second);
						if (frameSize.first == preferedWidth && frameSize.second == preferedHeight)
						{
							preferedFormat = i;
							break;
						}
					}
				}
			}
		}

		if (preferedFormat == -1)
		{
			hr = MF_E_UNSUPPORTED_FORMAT;
		}
		BREAK_ON_FAIL(hr);
		hr = webCamHelper.SetDeviceFormat(pSource_, preferedFormat);
		BREAK_ON_FAIL(hr);
		hr = webCamHelper.SetFrameRate(pSource_, 0, 20);
		BREAK_ON_FAIL(hr);

		hr = MfTopologyHelper::CreateYuYtoI420ColorTransf(&pYuYtoI420_);
		if (hr != S_OK)
		{
			failedComponent = L"Failed to create color transformer.";
		}
		BREAK_ON_FAIL(hr);

		hr = MfTopologyHelper::NegotiateSourceToMft(pSource_, pYuYtoI420_);
		BREAK_ON_FAIL(hr);

		hr = MfTopologyHelper::CreateVP8Encoder(&pVP8Encoder_);
		if (hr != S_OK)
		{
			failedComponent = L"Failed to create VP8 encoder.";
		}
		BREAK_ON_FAIL(hr);

		hr = MfTopologyHelper::NegotiateMftToVP8(pYuYtoI420_, pVP8Encoder_, webRtcCapability_);
		BREAK_ON_FAIL(hr);

		wstring outFolder;
		int recordLen;
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
			LOG_TRACE("Successfully created Media Foundation topology for Web Camera: " << cameraName);
		}
		// Notify player that we are done
		openCompletedSignal_(hr, errMsg);

		SetEvent(invokeCompleteEvent_);
	} 
	while (false);

	if (hr != S_OK)
	{
		wstring nativeErrMsg = NativeErrorsManager::ToString(hr);
		wstring msg = str(wformat(L"Failed to create media source for camera named: %1%. %2% %3%") % cameraName % failedComponent % nativeErrMsg); 
		errMsg.reset(new RtbcDeviceErrorOutMsg(cameraId, msg, hr));
		LOG_ERROR(msg);
	}

	return hr;
}

HRESULT WebCameraTopology::CreateTopology(CameraVideoRecording recordingType)
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
		UINT32 unDiscardableOutput = 0;
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
		hr = pSourceNode->ConnectOutput(0, pYuYtoI420Node, 0);
		BREAK_ON_FAIL(hr);
		// I420 image to VP8 encoder
		hr = pYuYtoI420Node->ConnectOutput(0, pEncoderNode, 0);
		BREAK_ON_FAIL(hr);
		// VP8 encoder -> TEE 
		hr = pEncoderNode->ConnectOutput(0, pTeeNode, 0);
		BREAK_ON_FAIL(hr);

		if (recordingType != CameraVideoRecording::DISABLED)
		{
			// TEE -> File sink
			hr = pTeeNode->ConnectOutput(0, pFileOutputNode, 0);
			BREAK_ON_FAIL(hr);
			// TEE -> Callback sink
			hr = pTeeNode->ConnectOutput(1, pCallbackOutputNode, 0);
			BREAK_ON_FAIL(hr);
		}
		else
		{
			// TEE -> Callback sink
			hr = pTeeNode->ConnectOutput(0, pCallbackOutputNode, 0);
			BREAK_ON_FAIL(hr);
		}

		// Connect callback for remote requests
		SetFrameCallback();
	}
	while(false);

	return hr;
}

