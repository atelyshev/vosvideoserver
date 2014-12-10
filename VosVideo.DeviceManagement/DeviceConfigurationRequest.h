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
			~DeviceConfigurationRequest();			
			virtual void ToJsonValue(web::json::value& obj) const;
			virtual void FromJsonValue(web::json::value& obj);
			virtual std::wstring ToString() const;

			const std::wstring& GetAccountId() const;
			const std::wstring& GetSiteId() const;
		private:
			std::wstring userAccountId_;
			std::wstring siteId_;
		};
	}
}


