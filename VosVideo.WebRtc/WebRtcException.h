#pragma once
#include <stdexcept>

namespace vosvideo
{
	namespace vvwebrtc
	{
		class WebRtcException : public std::runtime_error
		{
		public:
			WebRtcException(const std::string& msg) : runtime_error(msg)
			{
				LOG_ERROR(msg);
			}

			virtual ~WebRtcException(){}
		};
	}
}