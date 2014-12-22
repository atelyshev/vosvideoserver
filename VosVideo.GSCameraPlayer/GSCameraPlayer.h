#pragma once
#include <boost\thread\thread.hpp>
#include <gst\gst.h>
#include "VosVideo.CameraPlayer\CameraPlayerBase.h"

namespace vosvideo
{
	namespace cameraplayer
	{
		class GSCameraPlayer : public vosvideo::cameraplayer::CameraPlayerBase{
		public:
			GSCameraPlayer();
			~GSCameraPlayer();

			HRESULT OpenURL(vosvideo::data::CameraConfMsg&);

			void GetWebRtcCapability(webrtc::VideoCaptureCapability& webRtcCapability);
			HRESULT Play();
			HRESULT Pause();
			HRESULT Stop();
			HRESULT Shutdown();

			PlayerState GetState(std::shared_ptr<vosvideo::data::SendData>& lastErrMsg) const;
			PlayerState GetState() const;

			// Probably most important method, through it camera communicates to WebRTC
			void SetExternalCapturer(webrtc::VideoCaptureExternal* captureObserver);
			void RemoveExternalCapturers();
			void RemoveExternalCapturer(webrtc::VideoCaptureExternal* captureObserver);

			uint32_t GetDeviceId() const;			
		private:			
			void AppThreadStart();

			void DestroyGSStreamerPipeline();

			static gboolean CreateGStreamerPipeline(gpointer data);
			static gboolean RemoveWebRtcCapturer(gpointer data);
			static bool ChangeElementState(GstElement *element, GstState state);
			static void PadAddedHandler(GstElement *src, GstPad *new_pad, GSCameraPlayer *cameraPlayer);
			static void SourceSetupHandler(GstElement *element, GstElement *source, GSCameraPlayer *cameraPlayer);
			static void NewBufferHandler(GstElement *sink, GSCameraPlayer *cameraPlayer);
			static gboolean BusWatchHandler(GstBus *bus, GstMessage *msg, gpointer data);

			static const int FRAME_WIDTH = 528;
			static const int FRAME_HEIGHT = 384;
			static const int FRAMERATE_NUMERATOR = 10;
			static const int FRAMERATE_DENOMINATOR = 1;

			boost::thread *_appThread;

			PlayerState  _state;
			int _deviceId;
			std::wstring _deviceName;
			std::string _deviceVideoUri;
			std::wstring _userId;
			std::wstring _password;


			guint _busWatchId;
			GstElement *_uriDecodeBin;
			GstElement *_vp8Encoder;
			GstElement *_webmmuxer;
			GstElement *_videoConverter;
			GstElement *_videoScale;
			GstElement *_videoRate;
			GstElement *_videoScaleCapsFilter;
			GstElement *_videoRateCapsFilter;
			GstElement *_autoVideoSink;
			GstElement *_appSinkQueue;
			GstElement *_appSink;
			GstElement *_pipeline;
			GMainLoop *_mainLoop;

			boost::shared_mutex _mutex;
			std::unordered_map<uint32_t, webrtc::VideoCaptureExternal*> _webRtcVideoCapturers;
		};
	}
}