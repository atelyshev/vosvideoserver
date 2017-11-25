#include "stdafx.h"
#include "WebCameraPipeline.h"

using namespace vosvideo::cameraplayer;

WebCameraPipeline::WebCameraPipeline(vosvideo::data::CameraRecordingMode recordingMode, 
	const std::wstring& recordingFolder, 
	const std::wstring& camName) :
	GSPipelineBase(recordingMode, recordingFolder, camName)
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
	GST_DEBUG_BIN_TO_DOT_FILE(GST_BIN(_pipeline), GST_DEBUG_GRAPH_SHOW_ALL, "webcam.dot");

	return true;
}
