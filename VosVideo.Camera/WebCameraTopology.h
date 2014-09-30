#pragma once
#include "CameraTopology.h"
#include "WebCameraHelper.h"

namespace vosvideo
{
	namespace camera
	{
		class WebCameraTopology : public CameraTopology
		{
		public:
			WebCameraTopology();
			~WebCameraTopology();

			virtual HRESULT RenderUrlAsync(const vosvideo::data::CameraConfMsg& conf, boost::signal<void(HRESULT, std::shared_ptr<vosvideo::data::SendData>)>::slot_function_type subscriber);
			virtual HRESULT CreateMediaSource(std::wstring& sURL, std::wstring& username, std::wstring& pass);
			virtual HRESULT CreateTopology(vosvideo::data::CameraVideoRecording recordingType);

			// Main MF event handling function
			STDMETHODIMP Invoke(IMFAsyncResult* pAsyncResult);

		private:
			HRESULT BeginCreateMediaSource(std::wstring& sURL, IMFAsyncCallback *pCB, IUnknown *pState);
			HRESULT CreateVideoCaptureDeviceFromLink(std::wstring& symLink, IMFMediaSource **ppSource);

			WebCameraHelper webCamHelper;
			std::wstring sURL_;
			const static uint32_t preferedWidth = 640; 
			const static uint32_t preferedHeight = 480; 
			const static uint32_t preferedFrameRate = 15; 
		};
	}
}