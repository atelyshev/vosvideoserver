#pragma once
#include <stdexcept>

namespace vosvideo
{
	namespace archive
	{
		class VideoFileDiscovererException : public std::runtime_error
		{
		public:
			VideoFileDiscovererException(const std::string& msg) : runtime_error(msg)
			{
				LOG_ERROR(msg);
			}

			virtual ~VideoFileDiscovererException(){}
		};
	}
}