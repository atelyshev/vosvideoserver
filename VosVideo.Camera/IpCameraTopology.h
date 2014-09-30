#pragma once
#include "CameraTopology.h"

namespace vosvideo
{
	namespace camera
	{
		class IpCameraTopology : public CameraTopology
		{
		public:
			IpCameraTopology();
			~IpCameraTopology();

			virtual HRESULT RenderUrlAsync(const vosvideo::data::CameraConfMsg& conf, boost::signal<void(HRESULT, std::shared_ptr<vosvideo::data::SendData>)>::slot_function_type subscriber);
			virtual HRESULT CreateMediaSource(std::wstring& sURL, std::wstring& username, std::wstring& pass);
			virtual HRESULT CreateTopology(vosvideo::data::CameraVideoRecording recordingType);

			// Main MF event handling function
			STDMETHODIMP Invoke(IMFAsyncResult* pAsyncResult);
		};
	}
}
