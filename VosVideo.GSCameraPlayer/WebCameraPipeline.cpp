#include "stdafx.h"
#include "WebCameraPipeline.h"

using namespace vosvideo::cameraplayer;

WebCameraPipeline::WebCameraPipeline(
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
		camName)
{
}

GstElement* WebCameraPipeline::CreateSource()
{
	GstElement* source = gst_element_factory_make("ksvideosrc", "webcamera source");
	return source;
}

