#pragma once
#include "VosVideo.CameraPlayer\CameraPlayerBootstrapper.h"

namespace vosvideo
{
	namespace cameraplayer
	{
		class MFCameraPlayerBootstrapper : public CameraPlayerBootstrapper{
		public:
			MFCameraPlayerBootstrapper();
			~MFCameraPlayerBootstrapper();
			void Init(int* argc, char **argv[]);
			void Shutdown();
		};
	}
}