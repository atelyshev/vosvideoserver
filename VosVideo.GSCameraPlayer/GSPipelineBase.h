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
			GSPipelineBase(bool isRecordingEnabled, 
				vosvideo::data::CameraRecordingMode recordingMode,
				const std::wstring& recordingFolder, 
				uint32_t recordingLength,
				uint32_t maxFilesNum,
				const std::wstring& camName);
			~GSPipelineBase();

			virtual void Create();
			virtual void StartVideo();
			virtual void StopVideo();

			void StopPipeline();
			void GetWebRtcCapability(webrtc::VideoCaptureCapability& webRtcCapability);
			void AddExternalCapturer(webrtc::VideoCaptureExternal* externalCapturer);
			void RemoveExternalCapturer(webrtc::VideoCaptureExternal* externalCapturer);
			void RemoveAllExternalCapturers();

		protected:
			virtual GstElement* CreateSource() = 0;
			virtual bool LinkElements();
			virtual void ConfigureVideoBin();
			bool ChangeElementState(GstElement *element, GstState state);
			void DestroyPipeline();

			static void PrintCaps(const GstCaps * caps, const gchar * pfx);
			static gboolean PrintField(GQuark field, const GValue * value, gpointer pfx);
			static GstFlowReturn CbNewSampleHandler(GstElement *sink, GSPipelineBase *pipeline);

			GstElement *_pipeline = nullptr;
			GstElement *_sourceElement = nullptr;
			GstElement *_videoRate = nullptr;
			GstElement *_videoRateCapsFilter = nullptr;
			GstElement *_videoScale = nullptr;
			GstElement *_videoScaleCapsFilter = nullptr;
			GstElement *_videoConverter = nullptr;
			GstElement* _tee = nullptr; // tee
			GstElement* _queueRecord = nullptr; // queue
			GstElement* _x264encoder = nullptr; // x264enc
			GstElement* _h264parser = nullptr; // h264parser
			GstElement *_autoVideoSink = nullptr;
			GstElement* _clockOverlay = nullptr;
			GstElement *_appSinkQueue = nullptr;
			GstElement *_appSink = nullptr;
			GstElement *_fileSink = nullptr;

			GstPad* _teeFilePad = nullptr;
			GstPad* _teeVideoPad = nullptr;

			GstClock *_clock;

			bool _isRecordingEnabled = false;;
			vosvideo::data::CameraRecordingMode _recordingMode = vosvideo::data::CameraRecordingMode::PERMANENT;
			std::wstring _recordingFolder;
			std::wstring _camName;
			uint32_t _recordingLength;
			uint32_t _maxFilesNum;

			static const int FRAME_WIDTH = 528;
			static const int FRAME_HEIGHT = 384;
			static const int FRAMERATE_NUMERATOR = 10;
			static const int FRAMERATE_DENOMINATOR = 1;

		private:
			void AppThreadStart();
			void ConfigureCaps();

			// Static callbacks
			static GstPadProbeReturn CbUnlinkPad(GstPad *pad, GstPadProbeInfo *info, gpointer data);
			static void CbNewTeePadAdded(GstElement *element, GstPad* pad, gpointer data);
			static gboolean CreatePipeline(gpointer data);
			static gboolean CbBusWatchHandler(GstBus *bus, GstMessage *msg, gpointer data);
			static GstVideoFormat GetGstVideoFormatFromCaps(GstCaps* caps);
			static webrtc::VideoType GetRawVideoTypeFromGsVideoFormat(const GstVideoFormat& videoFormat);
			static bool CheckFileWriterElements(GSPipelineBase* pipelineBase);
			// But this one called from object scope
			bool CheckRealTimeElements();
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