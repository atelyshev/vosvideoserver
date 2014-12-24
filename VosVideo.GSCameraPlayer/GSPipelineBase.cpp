#include "stdafx.h"
#include "GSPipelineBase.h"


using vosvideo::cameraplayer::GSPipelineBase;

GSPipelineBase::GSPipelineBase()
{
	this->_appThread = new boost::thread(boost::bind(&GSPipelineBase::AppThreadStart, this));
	this->_appThread->detach();
}

GSPipelineBase::~GSPipelineBase(){
	LOG_TRACE("GSPipelineBase destroying camera player");
	if (this->_appThread)
		delete _appThread;

	g_main_loop_unref(_mainLoop);

	if (_pipeline)
	{
		DestroyGSStreamerPipeline();
	}
}

void GSPipelineBase::GetWebRtcCapability(webrtc::VideoCaptureCapability& webRtcCapability){
	webRtcCapability.width = GSPipelineBase::FRAME_WIDTH;
	webRtcCapability.height = GSPipelineBase::FRAME_HEIGHT;
	webRtcCapability.codecType = webrtc::VideoCodecType::kVideoCodecUnknown;
	webRtcCapability.maxFPS = GSPipelineBase::FRAMERATE_NUMERATOR;
}

void GSPipelineBase::Create(){
	g_main_context_invoke(NULL, CreateGStreamerPipeline, this);
}

void GSPipelineBase::Start(){
	if (!GSPipelineBase::ChangeElementState(_pipeline, GST_STATE_PLAYING)) {
		LOG_ERROR("GSPipeline: Unable to set the pipeline to the playing state.");
	}
}

void GSPipelineBase::Stop(){
	if (!GSPipelineBase::ChangeElementState(_pipeline, GST_STATE_NULL)) {
		LOG_ERROR("GSPipeline: Unable to set the pipeline to the null state.");
	}
}

void GSPipelineBase::AddExternalCapturer(webrtc::VideoCaptureExternal* externalCapturer){
	{
		boost::unique_lock<boost::shared_mutex> lock(_mutex);
		this->_webRtcVideoCapturers.insert(std::make_pair(reinterpret_cast<uint32_t>(externalCapturer), externalCapturer));
	}

	//Only create a new pipeline if one isn't created yet
	if (!this->_pipeline)
		g_main_context_invoke(NULL, CreateGStreamerPipeline, this);
}

void GSPipelineBase::RemoveExternalCapturer(webrtc::VideoCaptureExternal* externalCapturer){

	boost::unique_lock<boost::shared_mutex> lock(_mutex);
	this->_webRtcVideoCapturers.erase(reinterpret_cast<uint32_t>(externalCapturer));
	//If we don't have any more webrtc capturers we destroy the pipeline
	//TODO: don't destroy it if recording is enabled
	if (this->_webRtcVideoCapturers.size() == 0)
		DestroyGSStreamerPipeline();
}

void GSPipelineBase::RemoveAllExternalCapturers(){
	boost::unique_lock<boost::shared_mutex> lock(_mutex);
	this->_webRtcVideoCapturers.clear();
	DestroyGSStreamerPipeline();
}


void GSPipelineBase::AppThreadStart(){
	LOG_TRACE("GSPipeline App thread started");

	this->_mainLoop = g_main_loop_new(nullptr, FALSE);
	if (!this->_mainLoop){
		LOG_ERROR("GSPipeline error: Unable to create main loop");
		return;
	}

	//will not return until the main loop is quit
	g_main_loop_run(this->_mainLoop);
}

void GSPipelineBase::DestroyGSStreamerPipeline(){
	Stop();
	gst_object_unref(_pipeline);
	g_source_remove(_busWatchId);
	this->_pipeline = NULL;
}

gboolean GSPipelineBase::CreateGStreamerPipeline(gpointer data){
	GstBus *bus;
	GstCaps *rawVideoScaleCaps;
	GstCaps *rawVideoRateCaps;
	GstCaps *appSinkCaps;

	GSPipelineBase *pipelineBase = (GSPipelineBase*)data;
	pipelineBase->SourceElement = pipelineBase->CreateSource();
	pipelineBase->_videoScale = gst_element_factory_make("videoscale", "videoscale");
	pipelineBase->VideoRate = gst_element_factory_make("videorate", "videorate");
	pipelineBase->_videoScaleCapsFilter = gst_element_factory_make("capsfilter", "videoscalecapsfilter");
	pipelineBase->_videoRateCapsFilter = gst_element_factory_make("capsfilter", "videoratecapsfilter");
	pipelineBase->_videoConverter = gst_element_factory_make("ffmpegcolorspace", "videoconverter");
	pipelineBase->_appSinkQueue = gst_element_factory_make("queue", "appsinkqueue");
	pipelineBase->_appSink = gst_element_factory_make("appsink", "sink");

	//Testing	
	//pipelineBase->_autoVideoSink = gst_element_factory_make("autovideosink", "testvideosink");

	pipelineBase->_pipeline = gst_pipeline_new("pipeline");

	if (!pipelineBase->SourceElement){
		LOG_ERROR("GSPipelineBase error: Unable to create the source element");
		return FALSE;
	}

	if (!pipelineBase->_videoScale){
		LOG_ERROR("GSPipelineBase error: Unable to create videoscale element");
		return FALSE;
	}

	if (!pipelineBase->VideoRate){
		LOG_ERROR("GSPipelineBase error: Unable to create videorate element");
		return FALSE;
	}

	if (!pipelineBase->_videoScaleCapsFilter){
		LOG_ERROR("GSPipelineBase error: Unable to create videoscalefilter element");
		return FALSE;
	}


	if (!pipelineBase->_videoRateCapsFilter){
		LOG_ERROR("GSPipelineBase error: Unable to create videoratesfilter element");
		return FALSE;
	}

	if (!pipelineBase->_videoConverter){
		LOG_ERROR("GSPipelineBase error: Unable to create ffmpegcolorspace element");
		return FALSE;
	}

	if (!pipelineBase->_appSinkQueue){
		LOG_ERROR("GSPipelineBase error: Unable to create appsink queue element");
		return FALSE;
	}

	if (!pipelineBase->_appSink){
		LOG_ERROR("GSPipelineBase error: Unable to create appsink element");
		return FALSE;
	}

	if (!pipelineBase->_pipeline){
		LOG_ERROR("GSPipelineBase error: Unable to create the pipeline");
		return FALSE;
	}

	gst_bin_add_many(GST_BIN(pipelineBase->_pipeline),
		pipelineBase->SourceElement, pipelineBase->VideoRate, pipelineBase->_videoRateCapsFilter,
		pipelineBase->_videoScale, pipelineBase->_videoScaleCapsFilter, pipelineBase->_videoConverter,
		pipelineBase->_appSinkQueue, pipelineBase->_appSink, NULL);



	if (!pipelineBase->LinkElements()) {
		LOG_ERROR("GSPipelineBase error: Unable to link all elements");
		return FALSE;
	}

	//Sets relevant properties for the elements in the pipeline

	//Set the caps filter to DCIF size. This will force the videoscale element to scale the video to DCIF size.
	rawVideoScaleCaps = gst_caps_new_simple("video/x-raw-yuv", "width", G_TYPE_INT, GSPipelineBase::FRAME_WIDTH, "height", G_TYPE_INT, GSPipelineBase::FRAME_HEIGHT, NULL);
	g_object_set(pipelineBase->_videoScaleCapsFilter, "caps", rawVideoScaleCaps, NULL);
	gst_caps_unref(rawVideoScaleCaps);

	rawVideoRateCaps = gst_caps_new_simple("video/x-raw-yuv", "framerate", GST_TYPE_FRACTION, GSPipelineBase::FRAMERATE_NUMERATOR, GSPipelineBase::FRAMERATE_DENOMINATOR, NULL);
	g_object_set(pipelineBase->_videoRateCapsFilter, "caps", rawVideoRateCaps, NULL);
	gst_caps_unref(rawVideoRateCaps);

	//Configure the appsink element
	appSinkCaps = gst_caps_from_string("video/x-raw-yuv");
	g_object_set(pipelineBase->_appSink, "emit-signals", TRUE, "caps", appSinkCaps, NULL);
	//g_object_set(pipelineBase->_appSink, "emit-signals", TRUE, NULL);	
	gst_caps_unref(appSinkCaps);

	//Connect to the new-buffer signal so we can retrieve samples without blocking
	g_signal_connect(pipelineBase->_appSink, "new-buffer", G_CALLBACK(pipelineBase->NewBufferHandler), pipelineBase);

	if (!GSPipelineBase::ChangeElementState(pipelineBase->_pipeline, GST_STATE_PLAYING)) {
		LOG_ERROR("GSPipelineBase: Unable to set the pipeline to the playing state.");
		return FALSE;
	}

	bus = gst_pipeline_get_bus(GST_PIPELINE(pipelineBase->_pipeline));
	pipelineBase->_busWatchId = gst_bus_add_watch(bus, BusWatchHandler, pipelineBase);
	gst_object_unref(bus);
	

	return FALSE;
}

gboolean GSPipelineBase::LinkElements(){
	if (!gst_element_link_many(VideoRate, _videoRateCapsFilter, _videoScale, _videoScaleCapsFilter, _videoConverter, _appSinkQueue, _appSink, NULL)) {
		return FALSE;
	}
	return TRUE;
}

bool GSPipelineBase::ChangeElementState(GstElement *element, GstState state){
	GstStateChangeReturn stateChangeReturn;
	stateChangeReturn = gst_element_set_state(element, state);
	if (stateChangeReturn == GST_STATE_CHANGE_FAILURE) {
		LOG_ERROR("GSCameraPlayer: Unable to set the element state to " << state);
		return false;
	}
	return true;
}


void GSPipelineBase::NewBufferHandler(GstElement *sink, GSPipelineBase *pipelineBase){
	GstBuffer *buffer;
	guint8 *data;
	guint size;

	webrtc::VideoCaptureCapability webRtcCap;

	//g_print("*");
	//Retrieve the buffer
	g_signal_emit_by_name(sink, "pull-buffer", &buffer);
	if (buffer) {
		data = GST_BUFFER_DATA(buffer);
		size = GST_BUFFER_SIZE(buffer);

		//PrintCaps(caps, " ");

		webRtcCap.width = GSPipelineBase::FRAME_WIDTH;
		webRtcCap.height = GSPipelineBase::FRAME_HEIGHT;
		webRtcCap.maxFPS = GSPipelineBase::FRAMERATE_NUMERATOR;
		webRtcCap.expectedCaptureDelay = 0;

		webRtcCap.rawType = pipelineBase->_rawVideoType;
		webRtcCap.codecType = webrtc::VideoCodecType::kVideoCodecUnknown;
		webRtcCap.interlaced = true;

		{
			boost::shared_lock<boost::shared_mutex> lock(pipelineBase->_mutex);
			for (auto iter = pipelineBase->_webRtcVideoCapturers.begin(); iter != pipelineBase->_webRtcVideoCapturers.end(); ++iter)
			{
				iter->second->IncomingFrame(data, size, webRtcCap);
			}
		}

		gst_buffer_unref(buffer);
	}

}

GstVideoFormat GSPipelineBase::GetGstVideoFormatFromCaps(GstCaps* caps){
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

webrtc::RawVideoType GSPipelineBase::GetRawVideoTypeFromGsVideoFormat(const GstVideoFormat& videoFormat){
	//Try to map gstreamer video formats to webrtc video formats.
	switch (videoFormat){
		case GST_VIDEO_FORMAT_I420:
			return webrtc::kVideoI420;
		case GST_VIDEO_FORMAT_YV12:
			return webrtc::kVideoYV12;
		case GST_VIDEO_FORMAT_YUY2:
			return webrtc::kVideoYUY2;
		case GST_VIDEO_FORMAT_UYVY:
			return webrtc::kVideoUYVY;
		case GST_VIDEO_FORMAT_NV12:
			return webrtc::kVideoNV12;
		case GST_VIDEO_FORMAT_NV21:
			return webrtc::kVideoNV21;
		case GST_VIDEO_FORMAT_ARGB:
			return webrtc::kVideoARGB;
	}
	return webrtc::kVideoUnknown;
}

void GSPipelineBase::PrintCaps(const GstCaps * caps, const gchar * pfx) 
{
	guint i;

	g_return_if_fail(caps != NULL);

	if (gst_caps_is_any(caps)) {
		g_print("%sANY\n", pfx);
		return;
	}
	if (gst_caps_is_empty(caps)) {
		g_print("%sEMPTY\n", pfx);
		return;
	}

	for (i = 0; i < gst_caps_get_size(caps); i++) {
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
	return TRUE;
}


gboolean GSPipelineBase::BusWatchHandler(GstBus *bus, GstMessage *msg, gpointer data){
	GError *err;
	gchar *debug_info;

	switch (GST_MESSAGE_TYPE(msg)) {
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
				//	GST_DEBUG_BIN_TO_DOT_FILE(GST_BIN(pipelineBase->_pipeline), GST_DEBUG_GRAPH_SHOW_ALL, "debug.dot");
			}
		}

		g_free(messageSourceName);
		//}

		break;
	}

	return TRUE;
}

void GSPipelineBase::SetWebRtcRawVideoType(){
	GstPad *pad = NULL;
	GstCaps *caps = NULL;

	pad = gst_element_get_static_pad(_appSink, "sink");

	caps = gst_pad_get_negotiated_caps(pad);
	if (!caps)
		caps = gst_pad_get_caps_reffed(pad);

	_rawVideoType = GSPipelineBase::GetRawVideoTypeFromGsVideoFormat(GSPipelineBase::GetGstVideoFormatFromCaps(caps));

	gst_caps_unref(caps);
	gst_object_unref(pad);
}

