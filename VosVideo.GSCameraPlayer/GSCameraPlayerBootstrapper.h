#pragma once
#include "VosVideo.CameraPlayer/CameraPlayerBootstrapper.h"

namespace vosvideo
{
	namespace cameraplayer
	{
		class GSCameraPlayerBootstrapper : public CameraPlayerBootstrapper
		{
		public:
			void Init(int* argc, char **argv[]) override;
			void Shutdown() override;
		};
	}
}