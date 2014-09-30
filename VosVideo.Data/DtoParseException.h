#pragma once
#include <stdexcept>
#include <vosvideocommon/SeverityLoggerMacros.h>

namespace vosvideo
{
	namespace data
	{
		class DtoParseException : public std::runtime_error
		{
		public:
			DtoParseException(const std::string msg) : runtime_error(msg)
			{
				LOG_ERROR(msg);
			}

			virtual ~DtoParseException(){}
		};
	}
}