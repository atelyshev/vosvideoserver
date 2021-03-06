#pragma once
#include <stdexcept>

namespace vosvideo
{
	namespace mediafile
	{
		class MediaFileException : public std::runtime_error
		{
		public:
			MediaFileException(const std::string& msg) : runtime_error(msg)
			{
				LOG_ERROR(msg);
			}

			virtual ~MediaFileException(){}
		};
	}
}