#pragma once
#include "GSPipelineBase.h"

namespace vosvideo
{
	namespace cameraplayer
	{
		class WebCameraPipeline : public GSPipelineBase
		{
		public:
			WebCameraPipeline(bool isRecordingEnabled,
				vosvideo::data::CameraRecordingMode recordingMode,
				const std::wstring& recordingFolder,
				uint32_t recordingLength, 
				uint32_t maxFilesNum,
				const std::wstring& camName);
		protected:
			GstElement* CreateSource();
		};
	}
}