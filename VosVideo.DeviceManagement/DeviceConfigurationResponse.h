#pragma once
#include "VosVideo.Data/JsonObjectBase.h"

namespace vosvideo
{
	namespace devicemanagement
	{
		class DeviceConfigurationResponse final : public vosvideo::data::JsonObjectBase
		{
		public:
			DeviceConfigurationResponse();
			~DeviceConfigurationResponse();
			virtual void ToJsonValue(web::json::value& obj) const;
			virtual void FromJsonValue(web::json::value& obj);
			virtual std::wstring ToString() const;

			friend std::wstring operator+(std::wstring const& leftStr, DeviceConfigurationResponse const& rightResp);
			std::wstring operator+(std::wstring const& str) const;		
		};
	}
}
