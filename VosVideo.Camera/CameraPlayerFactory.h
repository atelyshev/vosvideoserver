#pragma once
#include "VosVideo.CameraPlayer/CameraPlayerBase.h"

namespace vosvideo
{
	namespace camera
	{
		class CameraPlayerFactory
		{
		private:
			CameraPlayerFactory();
			virtual ~CameraPlayerFactory();
		public:
			static void Init(int* argc, char **argv[]);
			static void Shutdown();
			static vosvideo::cameraplayer::CameraPlayerBase* CreateCameraPlayer();
		};
	}
}