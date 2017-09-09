#pragma once

namespace vosvideo
{
	namespace cameraplayer
	{
		class CameraPlayerBootstrapper
		{
		public:
			CameraPlayerBootstrapper();
			virtual ~CameraPlayerBootstrapper();
			virtual void Init(int* argc, char **argv[]) = 0;
			virtual void Shutdown() = 0;
		};
	}
}