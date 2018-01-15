#include "stdafx.h"
#include <thread>
#include <chrono> 
#include <gst/base/gstbaseparse.h>
#include <boost/format.hpp>
#include "GSPipelineBase.h"

using namespace util;
using vosvideo::cameraplayer::GSPipelineBase;
using boost::wformat;

GSPipelineBase* _this;

BOOL WINAPI EndProcessHandler(int32_t type)
{
	_this->StopPipeline();
	return true;
}

// Need it only to finalize file recording. Called on app close only
void GSPipelineBase::StopPipeline()
{
	GstPad *filesink = gst_element_get_static_pad(_queueRecord, "sink");
	gst_pad_unlink(_teeFilePad, filesink);
	gst_object_unref(filesink);
	gst_element_send_event(_x264encoder, gst_event_new_eos());
	std::this_thread::sleep_for(std::chrono::seconds(10));
}

GSPipelineBase::GSPipelineBase(
	bool isRecordingEnabled,
	vosvideo::data::CameraRecordingMode recordingMode, 
	const std::wstring& recordingFolder, 
	uint32_t recordingLength,
	uint32_t maxFilesNum,
	const std::wstring& camName):
	_isRecordingEnabled(isRecordingEnabled),
	_recordingMode(recordingMode), 
	_recordingFolder(recordingFolder),
	_recordingLength(recordingLength),
	_maxFilesNum(maxFilesNum),
	_camName(camName)
{
//	__asm int 3;
	_appThread = std::make_unique<std::thread>(std::thread([this]() { AppThreadStart(); }));
	_appThread->detach();
	_this = this;
	SetConsoleCtrlHandler((PHANDLER_ROUTINE)EndProcessHandler, TRUE);
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

GstPadProbeReturn GSPipelineBase::CbUnlinkPad(GstPad *pad, GstPadProbeInfo *info, gpointer data)
{
	LOG_TRACE("Unlink app sink pad");
	g_print("Unlinking app sink pad");
	GSPipelineBase *pipelineBase = (GSPipelineBase*)data;
	GstPad *sinkpad = gst_element_get_static_pad(pipelineBase->_appSinkQueue, "sink");
	gst_pad_unlink(pipelineBase->_teeVideoPad, sinkpad);
	gst_object_unref(sinkpad);
	gst_element_send_event(pipelineBase->_appSink, gst_event_new_eos());
	// Finalize real-time branch
	gst_element_set_state(pipelineBase->_appSinkQueue, GST_STATE_NULL);
	gst_element_set_state(pipelineBase->_appSink, GST_STATE_NULL);

	gst_bin_remove(GST_BIN(pipelineBase->_pipeline), pipelineBase->_appSinkQueue);
	gst_bin_remove(GST_BIN(pipelineBase->_pipeline), pipelineBase->_appSink);

	gst_object_unref(pipelineBase->_appSinkQueue);
	gst_object_unref(pipelineBase->_appSink);

	gst_element_release_request_pad(pipelineBase->_tee, pipelineBase->_teeVideoPad);
	gst_object_unref(pipelineBase->_teeVideoPad);

	LOG_TRACE("Sink pad was unlinked and will be removed");
	return GST_PAD_PROBE_REMOVE;
}

void GSPipelineBase::StartVideo()
{
	LOG_TRACE("Start real-time video was requested");
	// Create new pad
	GstPadTemplate* templ = gst_element_class_get_pad_template(GST_ELEMENT_GET_CLASS(_tee), "src_%u");
	_teeVideoPad = gst_element_request_pad(_tee, templ, nullptr, nullptr);
	// Create second branch for real-time video
	_appSinkQueue = gst_element_factory_make("queue", "appsinkqueue");
	_appSink = gst_element_factory_make("appsink", "sink");

	if (!CheckRealTimeElements())
	{
		return;
	}
	//Connect to the new-buffer signal so we can retrieve samples without blocking
	if (!g_signal_connect(_appSink, "new-sample", G_CALLBACK(CbNewSampleHandler), this))
	{
		LOG_ERROR("Failed to add sample handler");
	}
	// Configure the appsink element
	GstCaps *appSinkCaps = gst_caps_from_string("video/x-raw");
	g_object_set(_appSink, "emit-signals", TRUE, "caps", appSinkCaps, nullptr);
	gst_caps_unref(appSinkCaps);

	gst_bin_add_many(GST_BIN(_pipeline), _appSinkQueue, _appSink, nullptr);

	if (!gst_element_link_many(_appSinkQueue, _appSink, nullptr))
	{
		LOG_ERROR("Failed to link real-time video elements");
		return;
	}
	gst_element_sync_state_with_parent(_appSinkQueue);
	gst_element_sync_state_with_parent(_appSink);

	GstPad *sinkpad = gst_element_get_static_pad(_appSinkQueue, "sink");
	gst_pad_link(_teeVideoPad, sinkpad);
	gst_object_unref(sinkpad);

	if (!_isRecordingEnabled) // In other mode pipeline is always playing
	{
		if (!GSPipelineBase::ChangeElementState(_pipeline, GST_STATE_PLAYING))
		{
			LOG_ERROR("Unable to set the pipeline to the playing state.");
		}
	}
	GST_DEBUG_BIN_TO_DOT_FILE(GST_BIN(_pipeline), GST_DEBUG_GRAPH_SHOW_ALL, "full_pipeline");
	LOG_TRACE("Started video pipeline");
}

void GSPipelineBase::StopVideo()
{
	LOG_TRACE("Stop real-time video was requested");
	if (!_isRecordingEnabled)
	{
		if (!GSPipelineBase::ChangeElementState(_pipeline, GST_STATE_NULL))
		{
			LOG_ERROR("Unable to STOP pipeline and set it to the NULL state.");
		}
	}
	gst_pad_add_probe(_teeVideoPad, GST_PAD_PROBE_TYPE_IDLE, CbUnlinkPad, this, nullptr);
	GST_DEBUG_BIN_TO_DOT_FILE(GST_BIN(_pipeline), GST_DEBUG_GRAPH_SHOW_ALL, "tee_unlinked");
	LOG_TRACE("Stopped video pipeline");
}

void GSPipelineBase::AddExternalCapturer(webrtc::VideoCaptureExternal* externalCapturer)
{
	{
		boost::unique_lock<boost::shared_mutex> lock(_mutex);
		_webRtcVideoCapturers.insert(std::make_pair(reinterpret_cast<uint32_t>(externalCapturer), externalCapturer));

		//Start only for first request, second request will get same frames
		if (_webRtcVideoCapturers.size() == 1)
		{
			StartVideo();
		}
		LOG_TRACE("Added new capturer. Number of active video capturers: " << _webRtcVideoCapturers.size());
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
		StopVideo();
	}
	LOG_TRACE("Removed capturer. Number of active video capturers: " << _webRtcVideoCapturers.size());
}

void GSPipelineBase::RemoveAllExternalCapturers()
{
	{
		boost::unique_lock<boost::shared_mutex> lock(_mutex);
		_webRtcVideoCapturers.clear();
	}
	StopVideo();
}


void GSPipelineBase::AppThreadStart()
{
	LOG_TRACE("GSPipeline App thread started");

	_mainLoop = g_main_loop_new(nullptr, FALSE);
	if (!_mainLoop)
	{
		LOG_ERROR("Unable to create main loop");
		return;
	}

	//will not return until the main loop is quit
	g_main_loop_run(_mainLoop);
}

void GSPipelineBase::DestroyPipeline()
{
	StopVideo();
	g_source_remove(_busWatchId);
	gst_object_unref(_pipeline);
	gst_object_unref(_sourceElement);
	gst_object_unref(_videoRate);
	gst_object_unref(_videoRateCapsFilter);
	gst_object_unref(_videoScale);
	gst_object_unref(_videoScaleCapsFilter);
	gst_object_unref(_videoConverter);
	gst_object_unref(_tee); 
	gst_object_unref(_queueRecord);
	gst_object_unref(_x264encoder);
	gst_object_unref(_h264parser);
	gst_object_unref(_autoVideoSink);
	gst_object_unref(_clockOverlay);
	gst_object_unref(_appSinkQueue);
	gst_object_unref(_appSink);
	gst_object_unref(_fileSink);
	_pipeline = nullptr;
	LOG_TRACE("Pipeline destroyed");
}

void GSPipelineBase::CbNewTeePadAdded(GstElement *element, GstPad* pad, gpointer data)
{
	GSPipelineBase *pipelineBase = (GSPipelineBase*)data;
	pipelineBase->_teeFilePad = pad;
}

gboolean GSPipelineBase::CreatePipeline(gpointer data)
{			
	GSPipelineBase *pipelineBase = (GSPipelineBase*)data;
	// Set pipeline
	pipelineBase->_pipeline = gst_pipeline_new("pipeline");
	pipelineBase->_clock = gst_pipeline_get_clock(GST_PIPELINE(pipelineBase->_pipeline));

	// Create components
	// Common part
	pipelineBase->_sourceElement = pipelineBase->CreateSource();
	pipelineBase->_videoConverter = gst_element_factory_make("videoconvert", "videoconverter");

	pipelineBase->_clockOverlay = gst_element_factory_make("clockoverlay", "clockoverlay");
	g_object_set(pipelineBase->_clockOverlay, "time-format", "%Y-%m-%d %H:%M:%S", nullptr);

	pipelineBase->_videoScale = gst_element_factory_make("videoscale", "videoscale");
	pipelineBase->_videoScaleCapsFilter = gst_element_factory_make("capsfilter", "videoscalecapsfilter");

	pipelineBase->_videoRate = gst_element_factory_make("videorate", "videorate");
	pipelineBase->_videoRateCapsFilter = gst_element_factory_make("capsfilter", "videoratecapsfilter");

	pipelineBase->_tee = gst_element_factory_make("tee", "tee");
	g_signal_connect(pipelineBase->_tee, "pad-added", G_CALLBACK(pipelineBase->CbNewTeePadAdded), pipelineBase);
	// First branch: file writer
	pipelineBase->_queueRecord = gst_element_factory_make("queue", "queue_record");
	pipelineBase->_x264encoder = gst_element_factory_make("x264enc", "x264enc");
	g_object_set(pipelineBase->_x264encoder, "key-int-max", 10, nullptr);
	g_object_set(pipelineBase->_x264encoder, "tune", 0x00000004, nullptr);

	pipelineBase->_h264parser = gst_element_factory_make("h264parse", "h264parse");
	gst_base_parse_set_pts_interpolation((GstBaseParse*)pipelineBase->_h264parser, true);
	gst_base_parse_set_infer_ts((GstBaseParse*)pipelineBase->_h264parser, true);

	// Configure file sink
	pipelineBase->_fileSink = gst_element_factory_make("splitmuxsink", "filesink");
	auto filePattern = boost::str(wformat(L"%1%\\%2%%3%.mp4") % pipelineBase->_recordingFolder % pipelineBase->_camName % L"%04d");
	LOG_TRACE("Video file writer name pattern: " << filePattern);
	g_object_set(pipelineBase->_fileSink, 
		"location", StringUtil::ToString(filePattern).c_str(), 
		"max-size-time", pipelineBase->_recordingLength * 60000000000, 
		"max-files", pipelineBase->_maxFilesNum, nullptr);

	// Check if components properly built
	if (!CheckFileWriterElements(pipelineBase))
		return false;

	pipelineBase->ConfigureCaps();

	// Make simple streamming pipeline
	if (pipelineBase->_isRecordingEnabled)
	{
		// Make complex dual pipeline with filewriter and real time stream
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
	else
	{  
		pipelineBase->ConfigureVideoBin();
	}

	if (!pipelineBase->LinkElements()) 
	{
		LOG_ERROR("Unable to link all elements");
		return false;
	}

	if (pipelineBase->_isRecordingEnabled)
	{
		if (!pipelineBase->ChangeElementState(pipelineBase->_pipeline, GST_STATE_PLAYING))
		{
			LOG_ERROR("Unable to set the pipeline to the PLAYING state.");
			return false;
		}
	}
	else
	{
		if (!pipelineBase->ChangeElementState(pipelineBase->_pipeline, GST_STATE_NULL))
		{
			LOG_ERROR("Unable to set the pipeline to the PAUSED state.");
			return false;
		}
	}

	GstBus *bus = gst_pipeline_get_bus(GST_PIPELINE(pipelineBase->_pipeline));
	pipelineBase->_busWatchId = gst_bus_add_watch(bus, CbBusWatchHandler, pipelineBase);
	gst_object_unref(bus);

	return false;
}

void GSPipelineBase::ConfigureVideoBin()
{
	_appSinkQueue = gst_element_factory_make("queue", "appsinkqueue");
	_appSink = gst_element_factory_make("appsink", "sink");
	//Connect to the new-buffer signal so we can retrieve samples without blocking
	g_signal_connect(_appSink, "new-sample", G_CALLBACK(CbNewSampleHandler), this);

	gst_bin_add_many(GST_BIN(_pipeline),
		_sourceElement,
		_videoConverter,
		_videoRate,
		_videoRateCapsFilter,
		_videoScale,
		_videoScaleCapsFilter,
		_tee,
		nullptr);
}

gboolean GSPipelineBase::CbBusWatchHandler(GstBus *bus, GstMessage *msg, gpointer data)
{
	GSPipelineBase* pipelineBase = (GSPipelineBase*)data;

	switch (GST_MESSAGE_TYPE(msg))
	{
	case GST_MESSAGE_ERROR:
	{
		//_asm int 3;
		GError *err;
		gchar *debug_info;
		gst_message_parse_error(msg, &err, &debug_info);
		g_printerr("Error received from element %s: %s\n", GST_OBJECT_NAME(msg->src), err->message);
		g_printerr("Additional debugging information: %s\n", debug_info ? debug_info : "none");
		g_clear_error(&err);
		g_free(debug_info);
		break;
	}
	case GST_MESSAGE_WARNING:
	{
		GError *err = nullptr;
		gchar *debug = nullptr;

		gst_message_parse_warning(msg, &err, &debug);

		g_printerr("ERROR: from element %s: %s\n", GST_OBJECT_NAME(msg->src), err->message);
		if (debug != nullptr)
			g_printerr("Additional debug info:\n%s\n", debug);

		g_error_free(err);
		g_free(debug);
		break;
	}
	case GST_MESSAGE_EOS:
	{
		//_asm int 3;
		g_print("End-Of-Stream reached.");
		break;
	}
	case GST_MESSAGE_STATE_CHANGED:
	{
		gchar* messageSourceName = gst_element_get_name(GST_MESSAGE_SRC(msg));
		GstState old_state, new_state, pending_state;
		gst_message_parse_state_changed(msg, &old_state, &new_state, &pending_state);
		if (GST_MESSAGE_SRC(msg) == GST_OBJECT(pipelineBase->_pipeline))
		{
			g_print("GSPipelineBase %s state changed from %s to %s:\n",
				messageSourceName, gst_element_state_get_name(old_state), gst_element_state_get_name(new_state));

			if (new_state == GstState::GST_STATE_PLAYING)
			{
				LOG_TRACE("Camera is in PLAYING state, you should see the video");
				//We need to set the webrtc raw video type that appsink will receive. This will be used later as frames are coming in to the appsink.
				pipelineBase->SetWebRtcRawVideoType();
			}
		}

		g_free(messageSourceName);
		break;
	}
	default:
		break;
	}
	return true;
}

void GSPipelineBase::ConfigureCaps()
{
	//Sets relevant properties for the elements in the pipeline
	//Set the caps filter to DCIF size. This will force the videoscale element to scale the video to DCIF size.
	GstCaps *videoScaleCaps = gst_caps_new_simple(
		"video/x-raw",
		"format", G_TYPE_STRING, "I420",
		"width", G_TYPE_INT, GSPipelineBase::FRAME_WIDTH,
		"height", G_TYPE_INT, GSPipelineBase::FRAME_HEIGHT,
		nullptr);
	g_object_set(_videoScaleCapsFilter, "caps", videoScaleCaps, nullptr);
	gst_caps_unref(videoScaleCaps);

	GstCaps *videoRateCaps = gst_caps_new_simple(
		"video/x-raw",
		"framerate", GST_TYPE_FRACTION,
		GSPipelineBase::FRAMERATE_NUMERATOR,
		GSPipelineBase::FRAMERATE_DENOMINATOR,
		nullptr);
	g_object_set(_videoRateCapsFilter, "caps", videoRateCaps, nullptr);
	gst_caps_unref(videoRateCaps);
}

bool GSPipelineBase::LinkElements()
{
	// Simple case real time pipeline
	if (_isRecordingEnabled)
	{   // Dual way, link up to tee element
		if(!gst_element_link_many(
			_sourceElement,
			_videoConverter,
			_clockOverlay,
			_videoScale,
			_videoScaleCapsFilter,
			_videoRate,
			_videoRateCapsFilter,
			_tee,
			nullptr))
		{
			LOG_ERROR("Failed to link elements before tee");
			return false;
		}

		if (!gst_element_link_many(
			_tee,
			_queueRecord,
			_x264encoder,
			_h264parser,
			_fileSink,
			nullptr))
		{
			LOG_ERROR("Failed to link elements after tee");
			return false;
		}
		g_object_set(GST_BIN(_pipeline), "message-forward", TRUE, nullptr);
		return true;
	}
	else
	{
		return gst_element_link_many(
			_sourceElement,
			_videoConverter,
			_videoRate,
			_videoRateCapsFilter,
			_videoScale,
			_videoScaleCapsFilter,
			_tee,
			nullptr);
	}
}

bool GSPipelineBase::ChangeElementState(GstElement *element, GstState state)
{
	GstStateChangeReturn stateChangeReturn = gst_element_set_state(element, state);
	if (stateChangeReturn == GST_STATE_CHANGE_FAILURE)
	{
		LOG_ERROR("Unable to set the element state to " << state);
		return false;
	}
	return true;
}

GstFlowReturn GSPipelineBase::CbNewSampleHandler(GstElement *sink, GSPipelineBase *pipelineBase)
{
#ifdef _DEBUG
	g_print("*");
#endif
	//Retrieve the buffer
	if (pipelineBase->_rawVideoType == webrtc::VideoType::kUnknown)
	{
		pipelineBase->SetWebRtcRawVideoType();
	}
	if (pipelineBase->_rawVideoType == webrtc::VideoType::kUnknown)
	{
		return GST_FLOW_OK;
	}

	GstSample *sample;
	g_signal_emit_by_name(sink, "pull-sample", &sample);
	if (sample)
	{
		GstMapInfo info;
		GstBuffer* buffer = gst_sample_get_buffer(sample);
		if (!gst_buffer_map(buffer, &info, GST_MAP_READ)) 
		{
			return GST_FLOW_ERROR;
		}

		guint8* data = info.data;
		guint size = info.size;
		gst_buffer_unmap(buffer, &info);
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
	GstStructure* capsStructure = gst_caps_get_structure(caps, 0);
	const GValue* formatTypeGValue = gst_structure_get_value(capsStructure, "format");
	const gchar *videoFormatStr = gst_value_serialize(formatTypeGValue);
	if (videoFormatStr != nullptr)
	{
		guint32 fourCCFormat = GST_STR_FOURCC(videoFormatStr);
		GstVideoFormat gstVideoFormat = gst_video_format_from_fourcc(fourCCFormat);
		return gstVideoFormat;
	}
	else
	{
		return GST_VIDEO_FORMAT_UNKNOWN;
	}
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

bool GSPipelineBase::CheckFileWriterElements(GSPipelineBase* pipelineBase)
{
	if (!pipelineBase->_pipeline)
	{
		LOG_ERROR("Unable to create the pipeline");
		return false;
	}

	if (!pipelineBase->_sourceElement)
	{
		LOG_ERROR("Unable to create the source element");
		return false;
	}

	if (!pipelineBase->_videoScale)
	{
		LOG_ERROR("Unable to create videoscale element");
		return false;
	}

	if (!pipelineBase->_videoScaleCapsFilter)
	{
		LOG_ERROR("Unable to create videoscalefilter element");
		return false;
	}

	if (!pipelineBase->_videoRate)
	{
		LOG_ERROR("Unable to create videorate element");
		return false;
	}

	if (!pipelineBase->_videoRateCapsFilter)
	{
		LOG_ERROR("Unable to create videoratesfilter element");
		return false;
	}

	if (!pipelineBase->_tee)
	{
		LOG_ERROR("Unable to create tee element");
		return false;
	}

	if (!pipelineBase->_videoConverter)
	{
		LOG_ERROR("Unable to create videoconverter element");
		return false;
	}

	if (!pipelineBase->_clockOverlay)
	{
		LOG_ERROR("Unable to create clockoverlay element");
		return false;
	}

	if (!pipelineBase->_x264encoder)
	{
		LOG_ERROR("Unable to create x264encoder element");
		return false;
	}

	if (!pipelineBase->_h264parser)
	{
		LOG_ERROR("Unable to create h264parser element");
		return false;
	}

	if (!pipelineBase->_fileSink)
	{
		LOG_ERROR("Unable to create filesink element");
		return false;
	}

	return true;
}

bool GSPipelineBase::CheckRealTimeElements()
{
	if (!_appSinkQueue)
	{
		LOG_ERROR("Unable to create appsink queue element");
		return false;
	}

	if (!_appSink)
	{
		LOG_ERROR("Unable to create appsink element");
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

void GSPipelineBase::SetWebRtcRawVideoType()
{
	if (_appSink)
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
}

