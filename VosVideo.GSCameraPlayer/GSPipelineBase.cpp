#include "stdafx.h"
#include <boost/format.hpp>
#include "GSPipelineBase.h"

using namespace util;
using vosvideo::cameraplayer::GSPipelineBase;
using boost::wformat;

GSPipelineBase::GSPipelineBase(vosvideo::data::CameraRecordingMode recordingMode, 
	const std::wstring& recordingFolder, 
	const std::wstring& camName):
	_recordingMode(recordingMode), _recordingFolder(recordingFolder), _camName(camName)
{
//	__asm int 3;
	_appThread = std::make_unique<std::thread>(std::thread([this]() { AppThreadStart(); }));
	_appThread->detach();
}

GSPipelineBase::~GSPipelineBase()
{
	LOG_TRACE("GSPipelineBase destroying camera player");

	g_main_loop_unref(_mainLoop);

	if (_pipeline)
	{
		DestroyPipeline();
	}
}

void GSPipelineBase::GetWebRtcCapability(webrtc::VideoCaptureCapability& webRtcCapability)
{
	webRtcCapability.width = GSPipelineBase::FRAME_WIDTH;
	webRtcCapability.height = GSPipelineBase::FRAME_HEIGHT;
	webRtcCapability.videoType = webrtc::VideoType::kUnknown;
	webRtcCapability.maxFPS = GSPipelineBase::FRAMERATE_NUMERATOR;
}

void GSPipelineBase::Create()
{
	//Only create a new pipeline if one isn't created yet
	if (!_pipeline)
	{
		g_main_context_invoke(nullptr, CreatePipeline, this);
	}
}

void GSPipelineBase::Start()
{
	if (!GSPipelineBase::ChangeElementState(_pipeline, GST_STATE_PLAYING)) 
	{
		LOG_ERROR("GSPipeline: Unable to set the pipeline to the playing state.");
	}
	LOG_ERROR("GSPipeline: started pipeline");
}

void GSPipelineBase::Stop()
{
	if (!GSPipelineBase::ChangeElementState(_pipeline, GST_STATE_NULL)) 
	{
		LOG_ERROR("GSPipeline: Unable to STOP pipeline and set it to the NULL state.");
	}
	LOG_ERROR("GSPipeline: stopped pipeline");
}

void GSPipelineBase::AddExternalCapturer(webrtc::VideoCaptureExternal* externalCapturer)
{
	{
		boost::unique_lock<boost::shared_mutex> lock(_mutex);
		_webRtcVideoCapturers.insert(std::make_pair(reinterpret_cast<uint32_t>(externalCapturer), externalCapturer));
		//Start only once
		if (_webRtcVideoCapturers.size() == 1)
		{
			Start();
		}
		LOG_TRACE("Added new capturer. Number of active video capturers: ", _webRtcVideoCapturers.size());
	}
}

void GSPipelineBase::RemoveExternalCapturer(webrtc::VideoCaptureExternal* externalCapturer)
{
	boost::unique_lock<boost::shared_mutex> lock(_mutex);
	{
		_webRtcVideoCapturers.erase(reinterpret_cast<uint32_t>(externalCapturer));
	}
	//If we don't have any more webrtc capturers we destroy the pipeline
	if (_webRtcVideoCapturers.size() == 0)
	{
		Stop();
	}
	LOG_TRACE("Removed capturer. Number of active video capturers: ", _webRtcVideoCapturers.size());
}

void GSPipelineBase::RemoveAllExternalCapturers()
{
	{
		boost::unique_lock<boost::shared_mutex> lock(_mutex);
		_webRtcVideoCapturers.clear();
	}
	DestroyPipeline();
}


void GSPipelineBase::AppThreadStart()
{
	LOG_TRACE("GSPipeline App thread started");

	_mainLoop = g_main_loop_new(nullptr, FALSE);
	if (!_mainLoop)
	{
		LOG_ERROR("GSPipeline error: Unable to create main loop");
		return;
	}

	//will not return until the main loop is quit
	g_main_loop_run(_mainLoop);
}

void GSPipelineBase::DestroyPipeline()
{
	Stop();
	gst_object_unref(_pipeline);
	g_source_remove(_busWatchId);
	_pipeline = nullptr;
	LOG_TRACE("Pipeline destroyed");
}

gboolean GSPipelineBase::CreatePipeline(gpointer data)
{			
	GSPipelineBase *pipelineBase = (GSPipelineBase*)data;
	//Testing	
	//pipelineBase->_autoVideoSink = gst_element_factory_make("autovideosink", "testvideosink");
	// Set pipeline
	pipelineBase->_pipeline = gst_pipeline_new("pipeline");
	// Create components
	// generic part
	pipelineBase->_sourceElement = pipelineBase->CreateSource();

	pipelineBase->_videoConverter = gst_element_factory_make("videoconvert", "videoconverter");
	pipelineBase->_appSinkQueue = gst_element_factory_make("queue", "appsinkqueue");

	pipelineBase->_clockOverlay = gst_element_factory_make("clockoverlay", "clockoverlay");
	g_object_set(pipelineBase->_clockOverlay, "time-format", "%Y-%m-%d %H:%M:%S", nullptr);

	pipelineBase->_videoScale = gst_element_factory_make("videoscale", "videoscale");
	pipelineBase->_videoScaleCapsFilter = gst_element_factory_make("capsfilter", "videoscalecapsfilter");

	pipelineBase->_videoRate = gst_element_factory_make("videorate", "videorate");
	pipelineBase->_videoRateCapsFilter = gst_element_factory_make("capsfilter", "videoratecapsfilter");

	pipelineBase->_tee = gst_element_factory_make("tee", "tee");
	// First branch: file writer
	pipelineBase->_queueRecord = gst_element_factory_make("queue", "queue_record");
	pipelineBase->_x264encoder = gst_element_factory_make("x264enc", "x264enc");
	g_object_set(pipelineBase->_x264encoder, "key-int-max", 10, nullptr);

	pipelineBase->_h264parser = gst_element_factory_make("h264parse", "h264parse");

	pipelineBase->_fileSink = gst_element_factory_make("splitmuxsink", "filesink");
	auto filePattern = boost::str(wformat(L"%1%\\%2%%3%.mp4") % pipelineBase->_recordingFolder % pipelineBase->_camName % L"%04d");
	LOG_TRACE("Video file writer name pattern: ", filePattern);
	g_object_set(pipelineBase->_fileSink, "location", StringUtil::ToString(filePattern), "max-size-time", 60000000000, "max-files", 10, nullptr);

	pipelineBase->_appSink = gst_element_factory_make("appsink", "sink");

	// Check if components properly built
	if (!CheckElements(pipelineBase))
		return false;

	// Fill first branch, filewriter
	if (pipelineBase->_recordingMode == vosvideo::data::CameraRecordingMode::DISABLED)
	{
		gst_bin_add_many(GST_BIN(pipelineBase->_pipeline),
			pipelineBase->_sourceElement,
			pipelineBase->_videoRate,
			pipelineBase->_videoRateCapsFilter,
			pipelineBase->_videoScale,
			pipelineBase->_videoScaleCapsFilter,
			pipelineBase->_videoConverter,
			pipelineBase->_appSinkQueue,
			pipelineBase->_appSink,
			nullptr);
	}
	else
	{
		gst_bin_add_many(
			GST_BIN(pipelineBase->_pipeline),
			pipelineBase->_sourceElement,
			pipelineBase->_videoConverter,
			pipelineBase->_clockOverlay,
			pipelineBase->_videoScale,
			pipelineBase->_videoScaleCapsFilter,
			pipelineBase->_videoRate,
			pipelineBase->_videoRateCapsFilter,
			pipelineBase->_tee,
			pipelineBase->_queueRecord,
			pipelineBase->_x264encoder,
			pipelineBase->_h264parser,
			pipelineBase->_fileSink,
			nullptr);
	}

	if (!pipelineBase->LinkElements()) 
	{
		LOG_ERROR("GSPipelineBase error: Unable to link all elements");
		return false;
	}

	//Sets relevant properties for the elements in the pipeline
	//Set the caps filter to DCIF size. This will force the videoscale element to scale the video to DCIF size.
	GstCaps *rawVideoScaleCaps = gst_caps_new_simple(
		"video/x-raw",
		"width", G_TYPE_INT, GSPipelineBase::FRAME_WIDTH, 
		"height", G_TYPE_INT, GSPipelineBase::FRAME_HEIGHT, 
		nullptr);
	g_object_set(pipelineBase->_videoScaleCapsFilter, "caps", rawVideoScaleCaps, nullptr);
	gst_caps_unref(rawVideoScaleCaps);

	GstCaps *rawVideoRateCaps = gst_caps_new_simple(
		"video/x-raw",
		"framerate", GST_TYPE_FRACTION, 
		GSPipelineBase::FRAMERATE_NUMERATOR, 
		GSPipelineBase::FRAMERATE_DENOMINATOR, 
		nullptr);
	g_object_set(pipelineBase->_videoRateCapsFilter, "caps", rawVideoRateCaps, nullptr);
	gst_caps_unref(rawVideoRateCaps);

	//Configure the appsink element
	GstCaps *appSinkCaps = gst_caps_from_string("video/x-raw");
	g_object_set(pipelineBase->_appSink, "emit-signals", TRUE, "caps", appSinkCaps, nullptr);
	gst_caps_unref(appSinkCaps);

	//Connect to the new-buffer signal so we can retrieve samples without blocking
	g_signal_connect(pipelineBase->_appSink, "new-sample", G_CALLBACK(pipelineBase->NewSampleHandler), pipelineBase);

	if (!GSPipelineBase::ChangeElementState(pipelineBase->_pipeline, GST_STATE_PAUSED)) 
	{
		LOG_ERROR("GSPipelineBase: Unable to set the pipeline to the playing state.");
		return false;
	}

	GstBus *bus = gst_pipeline_get_bus(GST_PIPELINE(pipelineBase->_pipeline));
	pipelineBase->_busWatchId = gst_bus_add_watch(bus, BusWatchHandler, pipelineBase);
	gst_object_unref(bus);	

	return false;
}

gboolean GSPipelineBase::LinkElements()
{
	if (_recordingMode == vosvideo::data::CameraRecordingMode::DISABLED)
	{
		return gst_element_link_many(
			_sourceElement,
			_videoRate,
			_videoRateCapsFilter, 
			_videoScale, 
			_videoScaleCapsFilter, 
			_videoConverter, 
			_appSinkQueue, 
			_appSink, 
			nullptr);
	}
	else
	{
		return gst_element_link_many(
			_sourceElement,
			_videoConverter,
			_clockOverlay,
			_videoScale,
			_videoScaleCapsFilter,
			_videoRate,
			_videoRateCapsFilter,
			_tee,
			nullptr);
	}
}

bool GSPipelineBase::ChangeElementState(GstElement *element, GstState state)
{
	GstStateChangeReturn stateChangeReturn;
	stateChangeReturn = gst_element_set_state(element, state);
	if (stateChangeReturn == GST_STATE_CHANGE_FAILURE)
	{
		LOG_ERROR("GSCameraPlayer: Unable to set the element state to " << state);
		return false;
	}
	return true;
}

GstFlowReturn GSPipelineBase::NewSampleHandler(GstElement *sink, GSPipelineBase *pipelineBase)
{
	GstSample *sample = nullptr;
	guint8 *data = nullptr;
	guint size = 0;

	//g_print("*");
	//Retrieve the buffer
	if (pipelineBase->_rawVideoType == webrtc::VideoType::kUnknown)
	{
		pipelineBase->SetWebRtcRawVideoType();
	}

	g_signal_emit_by_name(sink, "pull-sample", &sample);
	if (sample)
	{
		GstMapInfo info;
		GstBuffer* buffer = gst_sample_get_buffer(sample);
		if (gst_buffer_map(buffer, &info, GST_MAP_READ)) 
		{
			data = info.data;
			size = info.size;
			gst_buffer_unmap(buffer, &info);
		}

		//PrintCaps(caps, " ");
		webrtc::VideoCaptureCapability webRtcCap;
		webRtcCap.width = GSPipelineBase::FRAME_WIDTH;
		webRtcCap.height = GSPipelineBase::FRAME_HEIGHT;
		webRtcCap.maxFPS = GSPipelineBase::FRAMERATE_NUMERATOR;
		webRtcCap.videoType = pipelineBase->_rawVideoType;

		{
			boost::shared_lock<boost::shared_mutex> lock(pipelineBase->_mutex);
			for (const auto& cap : pipelineBase->_webRtcVideoCapturers)
			{
				cap.second->IncomingFrame(data, size, webRtcCap);
			}
		}
		gst_sample_unref(sample);
		return GST_FLOW_OK;
	}
	return GST_FLOW_ERROR;
}

GstVideoFormat GSPipelineBase::GetGstVideoFormatFromCaps(GstCaps* caps)
{
	GstStructure *capsStructure;
	const GValue *formatTypeGValue;
	const gchar *videoFormatStr;
	guint32 fourCCFormat;
	GstVideoFormat gstVideoFormat;

	capsStructure = gst_caps_get_structure(caps, 0);
	formatTypeGValue = gst_structure_get_value(capsStructure, "format");
	videoFormatStr = gst_value_serialize(formatTypeGValue);
	fourCCFormat = GST_STR_FOURCC(videoFormatStr);
	gstVideoFormat = gst_video_format_from_fourcc(fourCCFormat);

	return gstVideoFormat;
}

webrtc::VideoType GSPipelineBase::GetRawVideoTypeFromGsVideoFormat(const GstVideoFormat& videoFormat)
{
	//Try to map gstreamer video formats to webrtc video formats.
	switch (videoFormat){
		case GST_VIDEO_FORMAT_I420:
			return webrtc::VideoType::kI420;
		case GST_VIDEO_FORMAT_YV12:
			return webrtc::VideoType::kYV12;
		case GST_VIDEO_FORMAT_YUY2:
			return webrtc::VideoType::kYUY2;
		case GST_VIDEO_FORMAT_UYVY:
			return webrtc::VideoType::kUYVY;
		case GST_VIDEO_FORMAT_NV12:
			return webrtc::VideoType::kNV12;
		case GST_VIDEO_FORMAT_NV21:
			return webrtc::VideoType::kNV21;
		case GST_VIDEO_FORMAT_ARGB:
			return webrtc::VideoType::kARGB;
	}
	return webrtc::VideoType::kUnknown;
}

bool GSPipelineBase::CheckElements(GSPipelineBase* pipelineBase)
{
	if (!pipelineBase->_pipeline)
	{
		LOG_ERROR("GSPipelineBase error: Unable to create the pipeline");
		return false;
	}

	if (!pipelineBase->_sourceElement)
	{
		LOG_ERROR("GSPipelineBase error: Unable to create the source element");
		return false;
	}

	if (!pipelineBase->_videoScale)
	{
		LOG_ERROR("GSPipelineBase error: Unable to create videoscale element");
		return false;
	}

	if (!pipelineBase->_videoScaleCapsFilter)
	{
		LOG_ERROR("GSPipelineBase error: Unable to create videoscalefilter element");
		return false;
	}

	if (!pipelineBase->_videoRate)
	{
		LOG_ERROR("GSPipelineBase error: Unable to create videorate element");
		return false;
	}

	if (!pipelineBase->_videoRateCapsFilter)
	{
		LOG_ERROR("GSPipelineBase error: Unable to create videoratesfilter element");
		return false;
	}

	if (!pipelineBase->_tee)
	{
		LOG_ERROR("GSPipelineBase error: Unable to create tee element");
		return false;
	}

	if (!pipelineBase->_videoConverter)
	{
		LOG_ERROR("GSPipelineBase error: Unable to create videoconverter element");
		return false;
	}

	if (!pipelineBase->_clockOverlay)
	{
		LOG_ERROR("GSPipelineBase error: Unable to create clockoverlay element");
		return false;
	}

	if (!pipelineBase->_x264encoder)
	{
		LOG_ERROR("GSPipelineBase error: Unable to create x264encoder element");
		return false;
	}

	if (!pipelineBase->_h264parser)
	{
		LOG_ERROR("GSPipelineBase error: Unable to create h264parser element");
		return false;
	}

	if (!pipelineBase->_appSinkQueue)
	{
		LOG_ERROR("GSPipelineBase error: Unable to create appsink queue element");
		return false;
	}

	if (!pipelineBase->_appSink)
	{
		LOG_ERROR("GSPipelineBase error: Unable to create appsink element");
		return false;
	}

	if (!pipelineBase->_fileSink)
	{
		LOG_ERROR("GSPipelineBase error: Unable to create filesink element");
		return false;
	}

	return true;
}

void GSPipelineBase::PrintCaps(const GstCaps * caps, const gchar * pfx) 
{
	g_return_if_fail(caps != nullptr);

	if (gst_caps_is_any(caps)) 
	{
		g_print("%sANY\n", pfx);
		return;
	}
	if (gst_caps_is_empty(caps)) 
	{
		g_print("%sEMPTY\n", pfx);
		return;
	}

	for (guint i = 0; i < gst_caps_get_size(caps); i++)
	{
		GstStructure *structure = gst_caps_get_structure(caps, i);

		g_print("%s%s\n", pfx, gst_structure_get_name(structure));
		gst_structure_foreach(structure, PrintField, (gpointer)pfx);
	}
}

gboolean GSPipelineBase::PrintField(GQuark field, const GValue * value, gpointer pfx) 
{
	gchar *str = gst_value_serialize(value);

	g_print("%s  %15s: %s\n", (gchar *)pfx, g_quark_to_string(field), str);
	g_free(str);
	return true;
}


gboolean GSPipelineBase::BusWatchHandler(GstBus *bus, GstMessage *msg, gpointer data)
{
	GError *err;
	gchar *debug_info;

	switch (GST_MESSAGE_TYPE(msg)) 
	{
	case GST_MESSAGE_ERROR:
		//_asm int 3;
		gst_message_parse_error(msg, &err, &debug_info);
		g_printerr("GSPipelineBase: Error received from element %s: %s\n", GST_OBJECT_NAME(msg->src), err->message);
		g_printerr("GSPipelineBase: Debugging information: %s\n", debug_info ? debug_info : "none");
		g_clear_error(&err);
		g_free(debug_info);
		break;
	case GST_MESSAGE_EOS:
		//_asm int 3;
		g_print("End-Of-Stream reached.\n");
		break;
	case GST_MESSAGE_STATE_CHANGED:
		///* We are only interested in state-changed messages from the pipeline */
		//if (GST_MESSAGE_SRC (msg) == GST_OBJECT (cameraPlayer)) {
		gchar* messageSourceName = gst_element_get_name(GST_MESSAGE_SRC(msg));
		GstState old_state, new_state, pending_state;
		gst_message_parse_state_changed(msg, &old_state, &new_state, &pending_state);
		GSPipelineBase* pipelineBase = (GSPipelineBase*)data;
		if (GST_MESSAGE_SRC(msg) == GST_OBJECT(pipelineBase->_pipeline))
		{
			g_print("GSPipelineBase %s state changed from %s to %s:\n",
				messageSourceName, gst_element_state_get_name(old_state), gst_element_state_get_name(new_state));

			if (new_state == GstState::GST_STATE_PLAYING)
			{
				//We need to set the webrtc raw video type that appsink will receive. This will be used later as frames are coming in to the appsink.
				pipelineBase->SetWebRtcRawVideoType();
			}
		}

		g_free(messageSourceName);
		break;
	}

	return true;
}

void GSPipelineBase::SetWebRtcRawVideoType()
{
	GstPad* pad = gst_element_get_static_pad(_appSink, "sink");
	GstCaps* caps = gst_pad_get_current_caps(pad);
	if (!caps)
	{
		caps = gst_pad_query_caps(pad, nullptr);
		gst_caps_make_writable(caps);
	}

	_rawVideoType = GSPipelineBase::GetRawVideoTypeFromGsVideoFormat(GSPipelineBase::GetGstVideoFormatFromCaps(caps));

	gst_caps_unref(caps);
	gst_object_unref(pad);
}

