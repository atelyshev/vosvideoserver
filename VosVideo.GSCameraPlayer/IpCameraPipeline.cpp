#include "stdafx.h"
#include "IpCameraPipeline.h"

using namespace vosvideo::cameraplayer;

IpCameraPipeline::IpCameraPipeline(const std::string& uri, const std::wstring& username, const std::wstring& password):
_uri(uri), _username(username), _password(password)
{
}

IpCameraPipeline::~IpCameraPipeline()
{
}

GstElement* IpCameraPipeline::CreateSource()
{
	GstElement* source = gst_element_factory_make("uridecodebin", "uridecodebin");
	//g_object_set (pipelineBase->_uriDecodeBin, "uri", "http://193.201.74.114:80/mjpg/video.mjpg", NULL);
	g_object_set(source, "uri", _uri.c_str(), NULL);

	//Connect to the pad-added signal of the uridecodebin element
	g_signal_connect(source, "pad-added", G_CALLBACK(PadAddedHandler), this);
	//Connect to the source-setup signal of the uridecodebin element
	g_signal_connect(source, "source-setup", G_CALLBACK(SourceSetupHandler), this);

	return source;
}


void IpCameraPipeline::PadAddedHandler(GstElement *src, GstPad *new_pad, IpCameraPipeline *ipCameraPipeline)
{
	//We will attempt to connect the source pad of the source element to the sink pad of the next element downstream	
	GstPadLinkReturn padLinkReturn;
	GstCaps *newPadCaps;
	GstStructure *newPadStruct;
	GstPad *downstreamSinkPad;
	const gchar *newPadType;

	LOG_TRACE("GSPipelineBase Received new pad " << GST_PAD_NAME(new_pad) << " from " << GST_ELEMENT_NAME(src));

	downstreamSinkPad = gst_element_get_static_pad(ipCameraPipeline->VideoRate, "sink");

	//If the downstream sink pad is already linked, we have nothing to do here
	if (gst_pad_is_linked(downstreamSinkPad)) 
	{
		LOG_TRACE("GSPipelineBase  We are already linked. Ignoring");
		gst_object_unref(downstreamSinkPad);
		return;
	}

	newPadCaps = gst_pad_get_caps(new_pad);
	newPadStruct = gst_caps_get_structure(newPadCaps, 0);
	newPadType = gst_structure_get_name(newPadStruct);

	//If the capabilities of the newly created source pad is anything other than raw video, we will ignore it
	if (!g_str_has_prefix(newPadType, "video/x-raw")) 
	{
		LOG_TRACE("GSPipelineBase It has type " << newPadType << " which is not raw audio or video. Ignoring");
		if (!newPadCaps)
			gst_caps_unref(newPadCaps);
		return;
	}

	padLinkReturn = gst_pad_link(new_pad, downstreamSinkPad);

	if (GST_PAD_LINK_FAILED(padLinkReturn)) 
	{
		LOG_ERROR("GSCameraPlayer Type is " << newPadType << " but link failed");
	}
	//GST_DEBUG_BIN_TO_DOT_FILE(GST_BIN(cameraPlayer->_pipeline), GST_DEBUG_GRAPH_SHOW_ALL, "debug.dot");
}

void IpCameraPipeline::SourceSetupHandler(GstElement *element, GstElement *source, IpCameraPipeline *ipCameraPipeline)
{
	using namespace util;
	g_object_set(source, "user-id", StringUtil::ToString(ipCameraPipeline->_username).c_str(), "user-pw", StringUtil::ToString(ipCameraPipeline->_password).c_str(), NULL);
}
