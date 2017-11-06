#include "stdafx.h"
#include "WebCameraPipeline.h"

using namespace vosvideo::cameraplayer;

WebCameraPipeline::WebCameraPipeline()
{
}

WebCameraPipeline::~WebCameraPipeline()
{
}

GstElement* WebCameraPipeline::CreateSource()
{
	GstElement* source = gst_element_factory_make("ksvideosrc", "webcamera source");
	return source;
}

gboolean WebCameraPipeline::LinkElements()
{
	if (!GSPipelineBase::LinkElements())
	{
		return false;
	}
	if (!gst_element_link_many(_sourceElement, _videoRate, nullptr))
	{
		return false;
	}
	GST_DEBUG_BIN_TO_DOT_FILE(GST_BIN(_pipeline), GST_DEBUG_GRAPH_SHOW_ALL, "webcam.dot");

	return true;
}