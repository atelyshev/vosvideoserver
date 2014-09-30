#pragma once
#include <stdexcept>
#include <vosvideocommon/SeverityLoggerMacros.h>

namespace vosvideo
{
	namespace communication
	{
		class HttpClientException : public std::runtime_error
		{
		public:
			HttpClientException(const std::string msg) : runtime_error(msg)
			{
				LOG_ERROR(msg);
			}

			virtual ~HttpClientException(){}
		};
	}
}