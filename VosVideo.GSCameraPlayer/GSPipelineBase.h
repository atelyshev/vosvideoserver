#pragma once
#include <boost/thread/thread.hpp>
#include <unordered_map>
#include <gst/gst.h>
#include <gst/video/video.h>
#include <webrtc/modules/video_capture/video_capture_defines.h>
#include "VosVideo.Data/CameraConfMsg.h"

namespace vosvideo
{
	namespace cameraplayer
	{
		class GSPipelineBase
		{
		public:
			GSPipelineBase(vosvideo::data::CameraRecordingMode recordingMode, 
				const std::wstring& recordingFolder, const std::wstring& camName);
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
			static void PrintCaps(const GstCaps * caps, const gchar * pfx);
			static gboolean PrintField(GQuark field, const GValue * value, gpointer pfx);

			GstElement *_pipeline = nullptr;
			GstElement *_sourceElement = nullptr;
			GstElement *_videoRate = nullptr;
			GstElement *_videoRateCapsFilter = nullptr;
			GstElement *_videoScale = nullptr;
			GstElement *_videoScaleCapsFilter = nullptr;
			GstElement *_videoConverter = nullptr;
			GstElement* _tee = nullptr; // tee
			GstElement* _queueRecord = nullptr; // x264enc
			GstElement* _x264encoder = nullptr; // x264enc
			GstElement* _h264parser = nullptr; // h264parser
			GstElement *_autoVideoSink = nullptr;
			GstElement* _clockOverlay = nullptr;
			GstElement *_appSinkQueue = nullptr;
			GstElement *_appSink = nullptr;
			GstElement *_fileSink = nullptr;

			vosvideo::data::CameraRecordingMode _recordingMode;
			std::wstring _recordingFolder;
			std::wstring _camName;

			static const int FRAME_WIDTH = 528;
			static const int FRAME_HEIGHT = 384;
			static const int FRAMERATE_NUMERATOR = 10;
			static const int FRAMERATE_DENOMINATOR = 1;

		private:
			void AppThreadStart();

			void DestroyPipeline();

			static gboolean CreatePipeline(gpointer data);
			static bool ChangeElementState(GstElement *element, GstState state);
			static GstFlowReturn NewSampleHandler(GstElement *sink, GSPipelineBase *cameraPlayer);
			static gboolean BusWatchHandler(GstBus *bus, GstMessage *msg, gpointer data);
			static GstVideoFormat GetGstVideoFormatFromCaps(GstCaps* caps);
			static webrtc::VideoType GetRawVideoTypeFromGsVideoFormat(const GstVideoFormat& videoFormat);
			static bool CheckElements(GSPipelineBase* pipelineBase);
			void SetWebRtcRawVideoType();

			std::unique_ptr<std::thread> _appThread;

			guint _busWatchId = 0;
			GMainLoop *_mainLoop = nullptr;

			webrtc::VideoType _rawVideoType = webrtc::VideoType::kUnknown;

			boost::shared_mutex _mutex;
			std::unordered_map<uint32_t, webrtc::VideoCaptureExternal*> _webRtcVideoCapturers;
		};
	}
}