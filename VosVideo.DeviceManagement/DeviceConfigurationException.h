#pragma once
#include <stdexcept>

namespace vosvideo
{
	namespace devicemanagement
	{
		class DeviceConfigurationException : public std::runtime_error
		{
		public:
			DeviceConfigurationException(const std::string msg) : runtime_error(msg)
			{
				LOG_ERROR(msg);
			}

			virtual ~DeviceConfigurationException(){}
		};
	}
}