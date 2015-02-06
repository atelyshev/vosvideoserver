#pragma once
#include "VosVideo.CameraPlayer/CameraPlayerBootstrapper.h"

namespace vosvideo
{
	namespace cameraplayer
	{
		class GSCameraPlayerBootstrapper : public CameraPlayerBootstrapper{
		public:
			GSCameraPlayerBootstrapper();
			~GSCameraPlayerBootstrapper();
			void Init(int* argc, char **argv[]);
			void Shutdown();
		};
	}
}