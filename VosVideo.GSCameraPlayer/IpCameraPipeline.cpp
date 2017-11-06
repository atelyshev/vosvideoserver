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
	GstElement* source = gst_element_factory_make("uridecodebin", "source");
	//g_object_set (pipelineBase->_uriDecodeBin, "uri", "http://193.201.74.114:80/mjpg/video.mjpg", NULL);
	g_object_set(G_OBJECT(source), "uri", _uri.c_str(), nullptr);

	//Connect to the pad-added signal of the uridecodebin element
	g_signal_connect(source, "pad-added", G_CALLBACK(PadAddedHandler), this);

	//Connect to the source-setup signal of the uridecodebin element
	g_signal_connect(source, "source-setup", G_CALLBACK(SourceSetupHandler), this);

	// Dunno what is this for
	g_signal_connect(source, "drained", G_CALLBACK(DrainedHandler), this);
	return source;
}

gboolean IpCameraPipeline::LinkElements()
{
	return gst_element_link_many(
//		_videoRate,
//		_videoRateCapsFilter,
		_videoScale,
		_videoScaleCapsFilter,
		_videoConverter,
		_appSinkQueue,
		_appSink,
		nullptr);
}

void IpCameraPipeline::PadAddedHandler(GstElement *src, GstPad *new_pad, IpCameraPipeline *ipCameraPipeline)
{
	//We will attempt to connect the source pad of the source element to the sink pad of the next element downstream		
	LOG_TRACE("GSPipelineBase Received new pad " << GST_PAD_NAME(new_pad) << " from " << GST_ELEMENT_NAME(src));

	GstPad* downstreamSinkPad = gst_element_get_static_pad(ipCameraPipeline->_videoScale, "sink");

	//If the downstream sink pad is already linked, we have nothing to do here
	if (gst_pad_is_linked(downstreamSinkPad)) 
	{
		LOG_TRACE("IpCameraPipeline We are already linked. Ignoring");
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
		LOG_TRACE("IpCameraPipeline It has type " << newPadType << " which is not raw audio or video. Ignoring");
		if (!newPadCaps)
			gst_caps_unref(newPadCaps);
		return;
	}

	PrintCaps(newPadCaps, " ");
	GstPadLinkReturn padLinkReturn = gst_pad_link(new_pad, downstreamSinkPad);

	if (GST_PAD_LINK_FAILED(padLinkReturn)) 
	{
		LOG_ERROR("GSCameraPlayer Type is " << newPadType << " but link failed");
	}
	GST_DEBUG_BIN_TO_DOT_FILE(GST_BIN(ipCameraPipeline->_pipeline), GST_DEBUG_GRAPH_SHOW_ALL, "ipcam.dot");
}

void IpCameraPipeline::SourceSetupHandler(GstElement *element, GstElement *source, IpCameraPipeline *ipCameraPipeline)
{
	using namespace util;
	g_object_set(source, "user-id", StringUtil::ToString(ipCameraPipeline->_username).c_str(), "user-pw", StringUtil::ToString(ipCameraPipeline->_password).c_str(), nullptr);
}

void IpCameraPipeline::DrainedHandler(GstElement *element, IpCameraPipeline *ipCameraPipeline)
{
	LOG_TRACE("IpCameraPipeline drained");
}
