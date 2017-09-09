#pragma once
#include <stdexcept>

namespace vosvideo
{
	namespace configuration
	{
		class CredentialsException : public std::invalid_argument
		{
		public:
			CredentialsException(std::string msg) : invalid_argument(msg)
			{
				LOG_ERROR(msg);
			}
			virtual ~CredentialsException(){}
		};
	}
}