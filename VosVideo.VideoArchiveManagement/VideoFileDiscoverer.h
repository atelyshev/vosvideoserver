#pragma once
#include "LocalVideoFileDiscoverer.h"

namespace vosvideo
{
	namespace archive
	{
		class VideoFileDiscoverer
		{
		public:
			VideoFileDiscoverer();
			~VideoFileDiscoverer();

			std::shared_ptr<VideoFile> Discover(const std::wstring& path);
			std::vector<std::shared_ptr<VideoFile>> Discover(std::vector<std::wstring>& pathVect);

		private:
			LocalVideoFileDiscoverer localDiscoverer_;
		};
	}
}
