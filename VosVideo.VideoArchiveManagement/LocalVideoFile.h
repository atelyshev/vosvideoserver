#pragma once
#include "videofile.h"

namespace vosvideo
{
	namespace archive
	{
		class LocalVideoFile :	public VideoFile
		{
		public:
			LocalVideoFile();
			virtual ~LocalVideoFile();
		};
	}
}
