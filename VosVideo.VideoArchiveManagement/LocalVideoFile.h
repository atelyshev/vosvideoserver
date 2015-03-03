#pragma once
#include "VideoFile.h"

namespace vosvideo
{
	namespace archive
	{
		class LocalVideoFile : public VideoFile
		{
		public:
			LocalVideoFile();
			LocalVideoFile(const std::wstring& id, const std::wstring& path, uint64_t duration, uint64_t startTime, bool isSeekable);
			virtual ~LocalVideoFile();
		};
	}
}
