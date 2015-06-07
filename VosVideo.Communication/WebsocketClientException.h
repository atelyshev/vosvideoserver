#pragma once
#include <stdexcept>
#include <vosvideocommon/SeverityLoggerMacros.h>

namespace vosvideo
{
	namespace communication
	{
		class WebsocketClientException : public std::runtime_error
		{
		public:
			WebsocketClientException(const std::string& url, const std::string& msg) : runtime_error(msg)
			{
				LOG_ERROR("Constructed HttpClientException. URL: " + url + " MSG: " + msg);
			}

			virtual ~WebsocketClientException(){}
		};
	}
}