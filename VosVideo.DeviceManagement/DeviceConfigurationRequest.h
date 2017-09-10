#pragma once
#include "VosVideo.Data/JsonObjectBase.h"

namespace vosvideo
{
	namespace devicemanagement
	{
		class DeviceConfigurationRequest final : public vosvideo::data::JsonObjectBase
		{
		public:
			DeviceConfigurationRequest();
			DeviceConfigurationRequest(const std::wstring& accountid, const std::wstring& siteid);
			virtual ~DeviceConfigurationRequest();		

			virtual web::json::value ToJsonValue() const override;
			virtual void FromJsonValue(const web::json::value& obj) override;
			virtual std::wstring ToString() const override;

			const std::wstring& GetAccountId() const;
			const std::wstring& GetSiteId() const;
		private:
			std::wstring userAccountId_;
			std::wstring siteId_;
		};
	}
}


