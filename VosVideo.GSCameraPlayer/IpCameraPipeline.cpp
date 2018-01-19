#include "stdafx.h"
#include "IpCameraPipeline.h"

using namespace vosvideo::cameraplayer;
using namespace util;

IpCameraPipeline::IpCameraPipeline(
	const std::string& uri, 
	const std::wstring& username, 
	const std::wstring& password,
	bool isRecordingEnabled,
	vosvideo::data::CameraRecordingMode recordingMode,
	const std::wstring& recordingFolder,
	uint32_t recordingLength,
	uint32_t maxFilesNum,
	const std::wstring& camName) :
	GSPipelineBase(
		isRecordingEnabled, 
		recordingMode, 
		recordingFolder, 
		recordingLength,
		maxFilesNum,
		camName),
	_uri(uri), 
	_username(username), 
	_password(password)
{
}

GstElement* IpCameraPipeline::CreateSource()
{
	GstElement* source = gst_element_factory_make("uridecodebin", "source");
	g_object_set(G_OBJECT(source), "uri", _uri.c_str(), nullptr);

	//Connect to the pad-added signal of the uridecodebin element
	g_signal_connect(source, "pad-added", G_CALLBACK(CbPadAddedHandler), this);

	//Connect to the source-setup signal of the uridecodebin element
	g_signal_connect(source, "source-setup", G_CALLBACK(CbSourceSetupHandler), this);

	// Dunno what is this for
	g_signal_connect(source, "drained", G_CALLBACK(CbDrainedHandler), this);
	LOG_TRACE("Created video source uridecodebin");
	return source;
}

bool IpCameraPipeline::LinkElements()
{
	// Simple case real time pipeline
	if (_isRecordingEnabled)
	{   // Dual way, link up to tee element
		if (!gst_element_link_many(
			_videoConverter,
			_clockOverlay,
			_videoRate,
			_videoRateCapsFilter,
			_videoScale,
			_videoScaleCapsFilter,
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
			_videoConverter,
//This element is good to have but fo IP Camera makes problems related with timestamp
//			_videoRate,
//			_videoRateCapsFilter,
			_videoScale,
			_videoScaleCapsFilter,
			_tee,
			_appSinkQueue,
			_appSink,
			nullptr);
	}
	LOG_TRACE("Linked video pipeline elements");
} 

void IpCameraPipeline::ConfigureVideoBin()
{
	// Configure the appsink element
	GstCaps *appSinkCaps = gst_caps_from_string("video/x-raw");
	g_object_set(_appSink, "emit-signals", TRUE, "caps", appSinkCaps, nullptr);
	gst_caps_unref(appSinkCaps);

	gst_bin_add_many(GST_BIN(_pipeline),
		_sourceElement,
		_videoConverter,
//This element is good to have but fo IP Camera makes problems related with timestamp
//		_videoRate,
//		_videoRateCapsFilter,
		_videoScale,
		_videoScaleCapsFilter,
		_tee,
		_appSinkQueue,
		_appSink,
		nullptr);
	LOG_TRACE("Configured video bin");
}

GstPadProbeReturn IpCameraPipeline::CbHaveSample(GstPad* pad, GstPadProbeInfo* info, IpCameraPipeline* ipCameraPipeline)
{
#ifdef _DEBUG
	g_print("#");
#endif
	GstBuffer* buffer = GST_PAD_PROBE_INFO_BUFFER(info);
	buffer = gst_buffer_make_writable(buffer);
	if (buffer == NULL)
		return GST_PAD_PROBE_OK;

	if (ipCameraPipeline->_startTimestamp == 0)
	{
		ipCameraPipeline->_startTimestamp = gst_clock_get_time(ipCameraPipeline->_clock);
		ipCameraPipeline->_timestamp = ipCameraPipeline->_startTimestamp;
	}

	auto ts = gst_clock_get_time(ipCameraPipeline->_clock);
	GST_BUFFER_PTS(buffer) = ipCameraPipeline->_timestamp - ipCameraPipeline->_startTimestamp; // ipCameraPipeline->_timestamp;
	GST_BUFFER_DTS(buffer) = ipCameraPipeline->_timestamp - ipCameraPipeline->_startTimestamp;  //ipCameraPipeline->_timestamp;
	if (ipCameraPipeline->_timestamp == 0)
		GST_BUFFER_DURATION(buffer) = 0;//gst_util_uint64_scale_int(1, GST_SECOND, 2);
	else
		GST_BUFFER_DURATION(buffer) = ts - ipCameraPipeline->_timestamp;//gst_util_uint64_scale_int(1, GST_SECOND, 2);
	GST_BUFFER_OFFSET(buffer) = ipCameraPipeline->_timestamp - ipCameraPipeline->_startTimestamp;
	GST_BUFFER_OFFSET_END(buffer) = 0;
	ipCameraPipeline->_timestamp = ts;//+= GST_BUFFER_DURATION(buffer);
	return GST_PAD_PROBE_OK;
}

void IpCameraPipeline::CbPadAddedHandler(GstElement *src, GstPad *new_pad, IpCameraPipeline *ipCameraPipeline)
{
	//We will attempt to connect the source pad of the source element to the sink pad of the next element downstream		
	LOG_DEBUG("GSPipelineBase Received new pad " << GST_PAD_NAME(new_pad) << " from " << GST_ELEMENT_NAME(src));
	GstPad* downstreamSinkPad = gst_element_get_static_pad(ipCameraPipeline->_videoConverter, "sink");

	// Only works if contigious play
	if (ipCameraPipeline->_isRecordingEnabled)
	{
		GstPad* pad = gst_element_get_static_pad(ipCameraPipeline->_videoConverter, "src");
		gst_pad_add_probe(pad, GST_PAD_PROBE_TYPE_BUFFER, (GstPadProbeCallback)ipCameraPipeline->CbHaveSample, ipCameraPipeline, NULL);
	}

	//If the downstream sink pad is already linked, we have nothing to do here
	if (gst_pad_is_linked(downstreamSinkPad)) 
	{
		LOG_DEBUG("IpCameraPipeline We are already linked. Ignoring");
		gst_object_unref(downstreamSinkPad);
		return;
	}

	GstCaps* newPadCaps = gst_pad_query_caps(new_pad, nullptr);
	gst_caps_make_writable(newPadCaps);
	GstStructure* newPadStruct = gst_caps_get_structure(newPadCaps, 0);
	const gchar* newPadType = gst_structure_get_name(newPadStruct);

	//If the capabilities of the newly created source pad is anything other than raw video, we will ignore it
	if (!g_str_has_prefix(newPadType, "video/x-raw")) 
	{
		LOG_DEBUG("IpCameraPipeline It has type " << newPadType << " which is not raw audio or video. Ignoring");
		if (!newPadCaps)
			gst_caps_unref(newPadCaps);
		return;
	}

	PrintCaps(newPadCaps, " ");
	GstPadLinkReturn padLinkReturn = gst_pad_link(new_pad, downstreamSinkPad);
	gst_object_unref(downstreamSinkPad);

	if (GST_PAD_LINK_FAILED(padLinkReturn)) 
	{
		LOG_ERROR("GSCameraPlayer Type is " << newPadType << " but link failed");
	}
	GST_DEBUG_BIN_TO_DOT_FILE(GST_BIN(ipCameraPipeline->_pipeline), GST_DEBUG_GRAPH_SHOW_ALL, "uptotee");
	LOG_TRACE("Video pad added");
}

void IpCameraPipeline::CbSourceSetupHandler(GstElement *element, GstElement *source, IpCameraPipeline *ipCameraPipeline)
{
	g_object_set(source, "user-id", StringUtil::ToString(ipCameraPipeline->_username).c_str(), "user-pw", StringUtil::ToString(ipCameraPipeline->_password).c_str(), nullptr);
	LOG_TRACE("Configured source. Set username and password if provided");
}

void IpCameraPipeline::CbDrainedHandler(GstElement *element, IpCameraPipeline *ipCameraPipeline)
{
	LOG_TRACE("IpCameraPipeline drained");
}
