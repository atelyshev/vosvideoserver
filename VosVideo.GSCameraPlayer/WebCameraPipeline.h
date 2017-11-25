#pragma once
#include "GSPipelineBase.h"

namespace vosvideo
{
	namespace cameraplayer
	{
		class WebCameraPipeline : public GSPipelineBase
		{
		public:
			WebCameraPipeline(vosvideo::data::CameraRecordingMode recordingMode, 
				const std::wstring& recordingFolder,
				const std::wstring& camName);
		protected:
			GstElement* CreateSource();
			gboolean LinkElements();
		};
	}
}