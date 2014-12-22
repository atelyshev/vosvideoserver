#include "stdafx.h"
#include "GSCameraPlayer.h"


using namespace vosvideo::cameraplayer;

void GSCameraPlayer::NewBufferHandler(GstElement *sink, GSCameraPlayer *cameraPlayer){
	GstBuffer *buffer;
	guint8 *data;
	guint size;
	webrtc::VideoCaptureCapability webRtcCap;

	g_print("*");
	//Retrieve the buffer
	g_signal_emit_by_name (sink, "pull-buffer", &buffer);
	if (buffer) {
		data = GST_BUFFER_DATA(buffer);
		size = GST_BUFFER_SIZE(buffer);

		webRtcCap.width = GSCameraPlayer::FRAME_WIDTH;
		webRtcCap.height = GSCameraPlayer::FRAME_HEIGHT;
		webRtcCap.maxFPS = GSCameraPlayer::FRAMERATE_NUMERATOR;
		webRtcCap.expectedCaptureDelay = 0;

		webRtcCap.rawType = webrtc::RawVideoType::kVideoIYUV;
		webRtcCap.codecType = webrtc::VideoCodecType::kVideoCodecUnknown;
		webRtcCap.interlaced = true;

		{
			boost::shared_lock<boost::shared_mutex> lock(cameraPlayer->_mutex);
			for (auto iter = cameraPlayer->_webRtcVideoCapturers.begin(); iter != cameraPlayer->_webRtcVideoCapturers.end(); ++iter)
			{
				iter->second->IncomingFrame(data, size, webRtcCap);
			}
		}

		gst_buffer_unref (buffer);
	}
}

void GSCameraPlayer::SourceSetupHandler(GstElement *element, GstElement *source, GSCameraPlayer *cameraPlayer){
	using namespace util;
	g_object_set (source, "user-id", StringUtil::ToString(cameraPlayer->_userId).c_str(), "user-pw", StringUtil::ToString(cameraPlayer->_password).c_str(),  NULL);
}

void GSCameraPlayer::PadAddedHandler(GstElement *src, GstPad *new_pad, GSCameraPlayer *cameraPlayer){
	//We will attempt to connect the source pad of the source element to the sink pad of the next element downstream	
	
	GstPadLinkReturn padLinkReturn;
	GstCaps *newPadCaps;
	GstStructure *newPadStruct;
	GstPad *downstreamSinkPad;
	const gchar *newPadType;

	LOG_TRACE("GSCameraPlayer Received new pad " << GST_PAD_NAME (new_pad)  << " from " << GST_ELEMENT_NAME (src));

	downstreamSinkPad = gst_element_get_static_pad (cameraPlayer->_videoRate, "sink");
  
	//If the downstream sink pad is already linked, we have nothing to do here
	if (gst_pad_is_linked (downstreamSinkPad)) {
		LOG_TRACE("GSCameraPlayer  We are already linked. Ignoring");
		gst_object_unref (downstreamSinkPad);
		return;
	}

	newPadCaps = gst_pad_get_caps (new_pad);
	newPadStruct = gst_caps_get_structure (newPadCaps, 0);
	newPadType = gst_structure_get_name (newPadStruct);

	//If the capabilities of the newly created source pad is anything other than raw video, we will ignore it
	if (!g_str_has_prefix (newPadType, "video/x-raw")) {
		LOG_TRACE("GSCameraPlayer It has type " <<  newPadType << " which is not raw audio or video. Ignoring");
		if (!newPadCaps)
			gst_caps_unref (newPadCaps);
		return;
	}

	padLinkReturn = gst_pad_link (new_pad, downstreamSinkPad);

	if (GST_PAD_LINK_FAILED (padLinkReturn)) {
		LOG_ERROR("GSCameraPlayer Type is " << newPadType << " but link failed");
	} 

	GST_DEBUG_BIN_TO_DOT_FILE(GST_BIN(cameraPlayer->_pipeline), GST_DEBUG_GRAPH_SHOW_ALL, "debug.dot");

	cameraPlayer->_state = PlayerState::Ready;
}

gboolean GSCameraPlayer::BusWatchHandler(GstBus *bus, GstMessage *msg, gpointer data){
	GError *err;
	gchar *debug_info;

	switch (GST_MESSAGE_TYPE (msg)) {
		case GST_MESSAGE_ERROR:
			//_asm int 3;
			gst_message_parse_error (msg, &err, &debug_info);
			g_printerr ("GSCameraPlayer: Error received from element %s: %s\n", GST_OBJECT_NAME (msg->src), err->message);
			g_printerr ("GSCameraPlayer: Debugging information: %s\n", debug_info ? debug_info : "none");
			g_clear_error (&err);
			g_free (debug_info);
			break;
		case GST_MESSAGE_EOS:
			//_asm int 3;
			g_print ("End-Of-Stream reached.\n");
			break;
		case GST_MESSAGE_STATE_CHANGED:
			///* We are only interested in state-changed messages from the pipeline */
			//if (GST_MESSAGE_SRC (msg) == GST_OBJECT (cameraPlayer)) {
				gchar* messageSourceName = gst_element_get_name(GST_MESSAGE_SRC(msg));
				GstState old_state, new_state, pending_state;
				gst_message_parse_state_changed (msg, &old_state, &new_state, &pending_state);
				g_print ("GSCameraPlayer %s state changed from %s to %s:\n",
					messageSourceName, gst_element_state_get_name (old_state), gst_element_state_get_name (new_state));
				g_free(messageSourceName);
			//}
				
			break;
	}

	return TRUE;
}

bool GSCameraPlayer::ChangeElementState(GstElement *element, GstState state){
	GstStateChangeReturn stateChangeReturn;
	stateChangeReturn = gst_element_set_state (element, state);
	if (stateChangeReturn == GST_STATE_CHANGE_FAILURE) {
		LOG_ERROR("GSCameraPlayer: Unable to set the element state to " << state);
		return false;
	}
	return true;
}

GSCameraPlayer::GSCameraPlayer(){
	LOG_TRACE("GSCameraPlayer created");
}

GSCameraPlayer::~GSCameraPlayer(){
	LOG_TRACE("GSCameraPlayer destroying camera player");
	if(this->_appThread)
		delete this->_appThread;

	g_main_loop_unref (this->_mainLoop);

	if(this->_pipeline)
	{
		DestroyGSStreamerPipeline();
	}
}

void GSCameraPlayer::DestroyGSStreamerPipeline(){
	gst_element_set_state (this->_pipeline, GST_STATE_NULL);
	gst_object_unref (this->_pipeline);
	g_source_remove (this->_busWatchId);
	this->_pipeline = nullptr;
}

HRESULT GSCameraPlayer::OpenURL(vosvideo::data::CameraConfMsg& cameraConf){
	if(_state == PlayerState::OpenPending || _state == PlayerState::Started || _state == PlayerState::Paused || _state == PlayerState::Stopped || _state == PlayerState::Closing)
		return E_FAIL;

	if(this->_appThread)
		return E_FAIL;

	std::wstring waudioUri;
	std::wstring wvideoUri;
	cameraConf.GetUris(waudioUri, wvideoUri);
	cameraConf.GetCameraIds(this->_deviceId, this->_deviceName);
	cameraConf.GetCredentials(this->_userId, this->_password);

	
	//Need to convert to std::string due to LOG_TRACE not working with std::wstring
	this->_deviceVideoUri = std::string(wvideoUri.begin(), wvideoUri.end());
	std::string audioUri(waudioUri.begin(), waudioUri.end());

	LOG_TRACE("GSCameraPlayer Opening Video URI " << this->_deviceVideoUri << " Audio URI " << audioUri);

	this->_appThread = new boost::thread(boost::bind(&GSCameraPlayer::AppThreadStart, this));
	this->_appThread->detach();

	_state = PlayerState::OpenPending;
	return S_OK;
}

void GSCameraPlayer::AppThreadStart(){
	LOG_TRACE("GSCameraPlayer App thread started");

	this->_mainLoop = g_main_loop_new(nullptr, FALSE);
	if(!this->_mainLoop){
		LOG_ERROR("GSCameraPlayer error: Unable to create main loop");
		return;
	}

	//will not return until the main loop is quit
	g_main_loop_run(this->_mainLoop);
}

gboolean GSCameraPlayer::CreateGStreamerPipeline(gpointer data){
	GstBus *bus;
	GstCaps *rawVideoScaleCaps;
	GstCaps *rawVideoRateCaps;
	GstCaps *appSinkCaps;

	GSCameraPlayer *cameraPlayer = (GSCameraPlayer*)data;
	cameraPlayer->_uriDecodeBin = gst_element_factory_make ("uridecodebin", "uridecodebin");
	cameraPlayer->_videoScale = gst_element_factory_make("videoscale", "videoscale");
	cameraPlayer->_videoRate = gst_element_factory_make("videorate", "videorate");
	cameraPlayer->_videoScaleCapsFilter = gst_element_factory_make("capsfilter", "videoscalecapsfilter");
	cameraPlayer->_videoRateCapsFilter = gst_element_factory_make("capsfilter", "videoratecapsfilter");
	cameraPlayer->_videoConverter = gst_element_factory_make("ffmpegcolorspace", "videoconverter");
	cameraPlayer->_vp8Encoder = gst_element_factory_make("vp8enc", "vp8enc");
	cameraPlayer->_webmmuxer = gst_element_factory_make("webmmux", "webmmuxer");
	cameraPlayer->_appSinkQueue = gst_element_factory_make ("queue", "appsinkqueue");
	cameraPlayer->_appSink = gst_element_factory_make("appsink", "sink");

	//Testing	
	//cameraPlayer->_autoVideoSink = gst_element_factory_make("autovideosink", "testvideosink");

	cameraPlayer->_pipeline = gst_pipeline_new ("pipeline");

	if(!cameraPlayer->_uriDecodeBin){
		LOG_ERROR("GSCameraPlayer error: Unable to create uridecodebin source");
		return FALSE;
	}

	if(!cameraPlayer->_videoScale){
		LOG_ERROR("GSCameraPlayer error: Unable to create videoscale element");
		return FALSE;
	}

	if(!cameraPlayer->_videoRate){
		LOG_ERROR("GSCameraPlayer error: Unable to create videorate element");
		return FALSE;
	}

	if(!cameraPlayer->_videoScaleCapsFilter){
		LOG_ERROR("GSCameraPlayer error: Unable to create videoscalefilter element");
		return FALSE;
	}


	if(!cameraPlayer->_videoRateCapsFilter){
		LOG_ERROR("GSCameraPlayer error: Unable to create videoratesfilter element");
		return FALSE;
	}

	if(!cameraPlayer->_vp8Encoder){
		LOG_ERROR("GSCameraPlayer error: Unable to create vp8 decoder element");
		return FALSE;
	}

	if(!cameraPlayer->_webmmuxer)
	{
		LOG_ERROR("GSCameraPlayer error: Unable to create webmmux element");
		return FALSE;
	}

	if(!cameraPlayer->_videoConverter){
		LOG_ERROR("GSCameraPlayer error: Unable to create ffmpegcolorspace element");
		return FALSE;
	}

	if(!cameraPlayer->_appSinkQueue){
		LOG_ERROR("GSCameraPlayer error: Unable to create appsink queue element");
		return FALSE;
	}

	if(!cameraPlayer->_appSink){
		LOG_ERROR("GSCameraPlayer error: Unable to create appsink element");
		return FALSE;
	}

	if(!cameraPlayer->_pipeline){
		LOG_ERROR("GSCameraPlayer error: Unable to create the pipeline");
		return FALSE;
	}

	gst_bin_add_many (GST_BIN (cameraPlayer->_pipeline), 
		cameraPlayer->_uriDecodeBin, cameraPlayer->_videoRate, cameraPlayer->_videoRateCapsFilter, 
		cameraPlayer->_videoScale, cameraPlayer->_videoScaleCapsFilter, cameraPlayer->_videoConverter,
	    cameraPlayer->_appSinkQueue, cameraPlayer->_appSink, NULL);

	//gst_bin_add_many (GST_BIN (cameraPlayer->_pipeline), cameraPlayer->_uriDecodeBin, cameraPlayer->_videoRate, cameraPlayer->_videoRateCapsFilter, cameraPlayer->_videoScale, cameraPlayer->_videoScaleCapsFilter,  cameraPlayer->_videoConverter, cameraPlayer->_autoVideoSink, NULL);

	//if (!gst_element_link_many(cameraPlayer->_videoRate, cameraPlayer->_videoRateCapsFilter, cameraPlayer->_videoScale, cameraPlayer->_videoScaleCapsFilter, cameraPlayer->_vp8Encoder, cameraPlayer->_webmmuxer, cameraPlayer->_appSinkQueue, cameraPlayer->_appSink, NULL)) {
	//	LOG_ERROR("GSCameraPlayer error: Unable to link all elements");
	//	gst_object_unref (cameraPlayer->_pipeline);
	//	return;
	//}

	if (!gst_element_link_many(cameraPlayer->_videoRate, cameraPlayer->_videoRateCapsFilter, cameraPlayer->_videoScale, cameraPlayer->_videoScaleCapsFilter, cameraPlayer->_videoConverter, cameraPlayer->_appSinkQueue, cameraPlayer->_appSink, NULL)) {
		LOG_ERROR("GSCameraPlayer error: Unable to link all elements");
		gst_object_unref (cameraPlayer->_pipeline);
		return FALSE;
	}

	//if (!gst_element_link_many(cameraPlayer->_videoRate, cameraPlayer->_videoRateCapsFilter, cameraPlayer->_videoScale, cameraPlayer->_videoScaleCapsFilter, cameraPlayer->_autoVideoSink, NULL)) {
	//	LOG_ERROR("GSCameraPlayer error: Unable to link all elements");
	//	gst_object_unref (cameraPlayer->_pipeline);
	//	return;
	//}

	
	//Sets relevant properties for the elements in the pipeline

	//g_object_set (cameraPlayer->_uriDecodeBin, "uri", "http://193.201.74.114:80/mjpg/video.mjpg", NULL);
	g_object_set (cameraPlayer->_uriDecodeBin, "uri", cameraPlayer->_deviceVideoUri.c_str(), NULL);

	//Set the caps filter to DCIF size. This will force the videoscale element to scale the video to DCIF size.
	rawVideoScaleCaps = gst_caps_new_simple("video/x-raw-yuv", "width", G_TYPE_INT, GSCameraPlayer::FRAME_WIDTH, "height", G_TYPE_INT, GSCameraPlayer::FRAME_HEIGHT, NULL);
	g_object_set(cameraPlayer->_videoScaleCapsFilter, "caps", rawVideoScaleCaps, NULL); 
	gst_caps_unref(rawVideoScaleCaps);

	rawVideoRateCaps = gst_caps_new_simple("video/x-raw-yuv", "framerate", GST_TYPE_FRACTION, GSCameraPlayer::FRAMERATE_NUMERATOR, GSCameraPlayer::FRAMERATE_DENOMINATOR, NULL);
	g_object_set(cameraPlayer->_videoRateCapsFilter, "caps", rawVideoRateCaps, NULL);
	gst_caps_unref(rawVideoRateCaps);
	
	//Configure the appsink element
	appSinkCaps = gst_caps_from_string("video/x-raw-yuv");
	g_object_set(cameraPlayer->_appSink, "emit-signals", TRUE, "caps", appSinkCaps, NULL);
	//g_object_set(cameraPlayer->_appSink, "emit-signals", TRUE, NULL);	
	gst_caps_unref(appSinkCaps);


	//Connect to the pad-added signal of the uridecodebin element
	g_signal_connect (cameraPlayer->_uriDecodeBin, "pad-added", G_CALLBACK (cameraPlayer->PadAddedHandler), cameraPlayer);
	//Connect to the source-setup signal of the uridecodebin element
	g_signal_connect (cameraPlayer->_uriDecodeBin, "source-setup", G_CALLBACK (cameraPlayer->SourceSetupHandler), cameraPlayer);
	//Connect to the new-buffer signal so we can retrieve samples without blocking
	g_signal_connect (cameraPlayer->_appSink, "new-buffer", G_CALLBACK (cameraPlayer->NewBufferHandler), cameraPlayer);


	if (!GSCameraPlayer::ChangeElementState(cameraPlayer->_pipeline, GST_STATE_PLAYING)) {
		LOG_ERROR("GSCameraPlayer: Unable to set the pipeline to the playing state.");
		gst_object_unref (cameraPlayer->_pipeline);
		return FALSE;
	}

	bus = gst_pipeline_get_bus(GST_PIPELINE(cameraPlayer->_pipeline));
	cameraPlayer->_busWatchId = gst_bus_add_watch (bus, BusWatchHandler, cameraPlayer);
	gst_object_unref(bus);

	return FALSE;
}



void GSCameraPlayer::GetWebRtcCapability(webrtc::VideoCaptureCapability& webRtcCapability){
	LOG_TRACE("GSCameraPlayer GetWebRtcCapability called");
	webRtcCapability.width = GSCameraPlayer::FRAME_WIDTH;
	webRtcCapability.height = GSCameraPlayer::FRAME_HEIGHT;
	webRtcCapability.interlaced = true;
	webRtcCapability.codecType = webrtc::VideoCodecType::kVideoCodecUnknown;
	webRtcCapability.maxFPS = GSCameraPlayer::FRAMERATE_NUMERATOR;
}

HRESULT GSCameraPlayer::Play(){
	LOG_TRACE("GSCameraPlayer Play called");
	return E_FAIL;
}

HRESULT GSCameraPlayer::Pause(){
	LOG_TRACE("GSCameraPlayer Paused called");
	return E_FAIL;
}

HRESULT GSCameraPlayer::Stop(){
	LOG_TRACE("GSCameraPlayer Stop called");
	return E_FAIL;
}

HRESULT GSCameraPlayer::Shutdown(){
	LOG_TRACE("GSCameraPlayer Shutdown called");
	return E_FAIL;
}

PlayerState GSCameraPlayer::GetState(std::shared_ptr<vosvideo::data::SendData>& lastErrMsg) const{
	LOG_TRACE("GSCameraPlayer GetState(shared_ptr) called");
	return _state;
}

PlayerState GSCameraPlayer::GetState() const{
	LOG_TRACE("GSCameraPlayer GetState called");
	return _state;
}

// Probably most important method, through it camera communicates to WebRTC
void GSCameraPlayer::SetExternalCapturer(webrtc::VideoCaptureExternal* captureObserver){
	LOG_TRACE("GSCameraPlayer SetExternalCapturer called");

	if(!this->_mainLoop)
	{
		LOG_ERROR("GSCameraPlayer cannot call SetExternalCapturer when the main loop hasn't been initialized yet.");
		return;
	}

	{
		boost::unique_lock<boost::shared_mutex> lock(_mutex);
		this->_webRtcVideoCapturers.insert(std::make_pair(reinterpret_cast<uint32_t>(captureObserver), captureObserver));
	}

	//Only create a new pipeline if one isn't created yet
	if(!this->_pipeline)
		g_main_context_invoke(NULL, CreateGStreamerPipeline, this);

}

void GSCameraPlayer::RemoveExternalCapturers(){
	LOG_TRACE("GSCameraPlayer RemoveExternalCapturers called");	
	
	boost::unique_lock<boost::shared_mutex> lock(_mutex);
	this->_webRtcVideoCapturers.clear();
	DestroyGSStreamerPipeline();
}

void GSCameraPlayer::RemoveExternalCapturer(webrtc::VideoCaptureExternal* captureObserver){
	LOG_TRACE("GSCameraPlayer RemoveExternalCapturer called");
	
	boost::unique_lock<boost::shared_mutex> lock(_mutex);
	this->_webRtcVideoCapturers.erase(reinterpret_cast<uint32_t>(captureObserver));
	//If we don't have any more webrtc capturers we destroy the pipeline
	//TODO: don't destroy it if recording is enabled
	if(this->_webRtcVideoCapturers.size() == 0)
		DestroyGSStreamerPipeline();
}

uint32_t GSCameraPlayer::GetDeviceId() const{
	LOG_TRACE("GSCameraPlayer GetDeviceId called");
	return this->_deviceId;
}
