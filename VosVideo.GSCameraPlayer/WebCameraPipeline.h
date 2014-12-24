#pragma once
#include "GSPipelineBase.h"

namespace vosvideo{
	namespace cameraplayer{
		class WebCameraPipeline : public GSPipelineBase{
		public:
			WebCameraPipeline();
			~WebCameraPipeline();
		protected:
			GstElement* CreateSource();
			gboolean LinkElements();
		};
	}
}