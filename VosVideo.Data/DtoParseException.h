#pragma once
#include <stdexcept>

namespace vosvideo
{
	namespace data
	{
		class DtoParseException : public std::runtime_error
		{
		public:
			DtoParseException(const std::string& msg) : runtime_error(msg)
			{
				LOG_ERROR(msg);
			}

			virtual ~DtoParseException(){}
		};
	}
}