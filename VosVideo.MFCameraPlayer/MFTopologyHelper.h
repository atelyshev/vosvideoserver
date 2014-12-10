#pragma once

#include <mfapi.h>
#include <mfidl.h>
#include <mferror.h>
#include <modules/video_capture/include/video_capture_defines.h>

namespace vosvideo
{
	namespace cameraplayer
	{
		class MfTopologyHelper
		{
		public:
			MfTopologyHelper();
			~MfTopologyHelper();

			static HRESULT CreateMpeg4Decoder(IMFTransform** ppTransf);
			static HRESULT CreateMjpegDecoder(IMFTransform** ppTransf);
			static HRESULT CreateH264Decoder(IMFTransform** ppTransf);

			// Negotiators
			// Negotiate media types between source (already created member variable) and the specified MFT.
			static HRESULT NegotiateSourceToMft(CComQIPtr<IMFMediaSource> pSource, CComPtr<IMFTransform> pMft);
			//// Negotiate connection from given MFT to VP8 MFT
			static HRESULT NegotiateMftToVP8(CComPtr<IMFTransform> pMft1, CComPtr<IMFTransform> pMft2, webrtc::VideoCaptureCapability& webRtcCapability);
			//// Negotiate connection from MFT with YUY2 output to MFT with I420 input
			static HRESULT NegotiateYUY2ToI420(CComPtr<IMFTransform> pMft1, CComPtr<IMFTransform> pMft2);

			static HRESULT CreateYuYtoI420ColorTransf(IMFTransform** ppTransf);
			static HRESULT CreateVP8Encoder(IMFTransform** ppTransf);
			static HRESULT CreateVP8CallbackSink(IMFMediaSink** ppSink);
			static HRESULT CreateVP8FileSink(IMFMediaSink** ppSink);
			static HRESULT CreateSourceStreamNode(CComQIPtr<IMFMediaSource> pSource, 
				CComPtr<IMFPresentationDescriptor> pPresDescriptor,
				CComPtr<IMFStreamDescriptor> pStreamDescriptor,
				CComPtr<IMFTopologyNode> &pNode);

		private:
			static HRESULT CreateDecoder(IMFTransform** ppTransf, MFT_REGISTER_TYPE_INFO& inputFilter);
		};

	}
}