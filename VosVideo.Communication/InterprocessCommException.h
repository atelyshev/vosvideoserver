#pragma once
#include <stdexcept>
#include <vosvideocommon/SeverityLoggerMacros.h>

namespace vosvideo
{
	namespace communication
	{
		class InterprocessCommException : public std::runtime_error
		{
		public:
			InterprocessCommException(const std::string msg) : runtime_error(msg)
			{
				LOG_ERROR(msg);
			}

			virtual ~InterprocessCommException(){}
		};
	}
}