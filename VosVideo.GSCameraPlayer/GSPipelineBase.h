#pragma once
#include <boost/thread/thread.hpp>
#include <unordered_map>
#include <gst/gst.h>
#include <gst/video/video.h>
#include <modules/video_capture/include/video_capture_defines.h>

namespace vosvideo
{
	namespace cameraplayer
	{
		class GSPipelineBase
		{
		public:
			GSPipelineBase();
			~GSPipelineBase();

			void Create();
			void Start();
			void Stop();

			void GetWebRtcCapability(webrtc::VideoCaptureCapability& webRtcCapability);
			void AddExternalCapturer(webrtc::VideoCaptureExternal* externalCapturer);
			void RemoveExternalCapturer(webrtc::VideoCaptureExternal* externalCapturer);
			void RemoveAllExternalCapturers();

		protected:
			virtual GstElement* CreateSource() = 0;
			virtual gboolean LinkElements();
			GstElement *VideoRate;
			GstElement *SourceElement;
		private:
			void AppThreadStart();

			void DestroyGSStreamerPipeline();

			static gboolean CreateGStreamerPipeline(gpointer data);
			static bool ChangeElementState(GstElement *element, GstState state);
			static void NewBufferHandler(GstElement *sink, GSPipelineBase *cameraPlayer);
			static gboolean BusWatchHandler(GstBus *bus, GstMessage *msg, gpointer data);
			static GstVideoFormat GetGstVideoFormatFromCaps(GstCaps* caps);
			static webrtc::RawVideoType GetRawVideoTypeFromGsVideoFormat(const GstVideoFormat& videoFormat);
			static void PrintCaps(const GstCaps * caps, const gchar * pfx);
			static gboolean PrintField(GQuark field, const GValue * value, gpointer pfx);

			void SetWebRtcRawVideoType();

			static const int FRAME_WIDTH = 528;
			static const int FRAME_HEIGHT = 384;
			static const int FRAMERATE_NUMERATOR = 10;
			static const int FRAMERATE_DENOMINATOR = 1;

			boost::thread *_appThread;

			guint _busWatchId;
			GstElement *_videoConverter;
			GstElement *_videoScale;
			GstElement *_videoScaleCapsFilter;
			GstElement *_videoRateCapsFilter;
			GstElement *_autoVideoSink;
			GstElement *_appSinkQueue;
			GstElement *_appSink;
			GstElement *_pipeline;
			GMainLoop *_mainLoop;

			webrtc::RawVideoType _rawVideoType;

			boost::shared_mutex _mutex;
			std::unordered_map<uint32_t, webrtc::VideoCaptureExternal*> _webRtcVideoCapturers;
		};
	}
}