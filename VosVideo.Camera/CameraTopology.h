#pragma once
// Media Foundation headers
#include <mfapi.h>
#include <modules/video_capture/include/video_capture_defines.h>
#include <boost/signals2.hpp>
#include <ppltasks.h>
#include <agents.h>
#include <VP8CallbackSink/VP8CallbackSink.h>
#include "VosVideo.Data/RtbcDeviceErrorOutMsg.h"
#include "VosVideo.Data/CameraConfMsg.h"

namespace vosvideo
{
	namespace camera
	{
		// This class should be able to build next topologies:
		// 1. IP CAM MJPEG->YUY2->I420->VP8->[FILE or Callback sink] 
		// This is non CPU intensive topology
		// 2. IP CAM H264->YUY2->I420->VP8->[FILE or Callback sink]
		// very CPU intensive topology
		// 2. IP CAM MPEG4->YUY2->I420->VP8->[FILE or Callback sink]
		// very CPU intensive topology

		class FrameCallback;

		class CameraTopology : public IMFAsyncCallback
		{
		public:
			CameraTopology();
			~CameraTopology();

			virtual HRESULT RenderUrlAsync(const vosvideo::data::CameraConfMsg& conf, boost::signals2::signal<void(HRESULT, std::shared_ptr<vosvideo::data::SendData>)>::slot_function_type subscriber) = 0;
			IMFTopology* GetTopology();

			void GetWebRtcCapability(webrtc::VideoCaptureCapability& webRtcCapability);
			void SetExternalCapturer(webrtc::VideoCaptureExternal* captureObserver);
			void RemoveExternalCapturer(webrtc::VideoCaptureExternal* captureObserver);
			void RemoveExternalCapturers();

			void VP8BufCallback(std::vector<unsigned char>& vFrame, std::vector<int>& vCapabilities);
			HRESULT ShutdownSource();

			//
			// IMFAsyncCallback implementation.
			//
			// Skip the optional GetParameters() function - it is used only in advanced players.
			// Returning the E_NOTIMPL error code causes the system to use default parameters.
			STDMETHODIMP GetParameters(DWORD *pdwFlags, DWORD *pdwQueue)   { return E_NOTIMPL; }

			//
			// IUnknown methods
			//
			STDMETHODIMP QueryInterface(REFIID iid, void** ppv);
			STDMETHODIMP_(ULONG) AddRef();
			STDMETHODIMP_(ULONG) Release();

		protected:
			// Creators
			virtual HRESULT CreateMediaSource(std::wstring& sURL, std::wstring& username, std::wstring& pass) = 0;
			virtual HRESULT CreateTopology(vosvideo::data::CameraVideoRecording recordingType) = 0;

			HRESULT SetFileSinkParameters(vosvideo::data::CameraConfMsg& conf);
			HRESULT SetFrameCallback();

			// variables
			CComQIPtr<IMFMediaSource> pSource_;       // the MF source
			CComPtr<IMFTransform>     pDecoder_;
			CComPtr<IMFTransform>     pYuYtoI420_;
			CComPtr<IMFTransform>     pVP8Encoder_;
			CComPtr<IMFMediaSink>     pVP8FileSink_;
			CComPtr<IMFMediaSink>     pVP8CallbackSink_;

			CComQIPtr<IMFTopology>    pTopology_;     // the topology itself
			CComPtr<IVP8Callback>     pVP8Callback_;
			CComPtr<IUnknown>		  pCancelCookie_;
			CComPtr<IMFSourceResolver> pSourceResolver_;
			std::atomic<long>         nRefCount_;   // COM reference count.
			// For simplicity just use webrtc capability
			webrtc::VideoCaptureCapability webRtcCapability_;
			std::unordered_map<uint32_t, webrtc::VideoCaptureExternal*> captureObservers_;
			FrameCallback            * newFrameCallback_;
			vosvideo::data::CameraConfMsg              conf_;
			// Signal to complete build player
			boost::signals2::signal<void (HRESULT, std::shared_ptr<vosvideo::data::SendData>)> openCompletedSignal_;
			Concurrency::timer<CameraTopology*>* fireCancelTimer_; 
			HANDLE           invokeCompleteEvent_;
			static const int openConnTimeout_ = 20000;

			std::mutex mutex_;
		};

		class FrameCallback : public INewFrameCallback
		{
		public: 
			FrameCallback(CameraTopology* topoPtr);
			virtual void IncomingVp8Frame(std::vector<byte>&, std::vector<int>&);

		private:
			CameraTopology* topoPtr_;
		};
	}
}

