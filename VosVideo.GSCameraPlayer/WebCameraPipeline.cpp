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
		return FALSE;
	}
	if (!gst_element_link_many(SourceElement, VideoRate, NULL))
	{
		return FALSE;
	}
	return TRUE;
}