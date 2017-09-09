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
			virtual ~DeviceConfigurationResponse();
			virtual void ToJsonValue(web::json::value& obj) const override;
			virtual void FromJsonValue(web::json::value& obj) override;
			virtual std::wstring ToString() const override;

			friend std::wstring operator+(std::wstring const& leftStr, DeviceConfigurationResponse const& rightResp);
			std::wstring operator+(std::wstring const& str) const;		
		};
	}
}
