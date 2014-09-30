#pragma once
#include <stdexcept>

namespace vosvideo
{
	namespace configuration
	{
		class ConfigurationParserException : public std::runtime_error
		{
		public:
			ConfigurationParserException(const std::string msg) : runtime_error(msg){}
			virtual ~ConfigurationParserException(){}
		};
	}
}