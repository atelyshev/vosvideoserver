#pragma once
#include "ReceivedData.h"

namespace vosvideo
{
	namespace data
	{
		class DeviceConfigurationMsg final : public ReceivedData
		{
		public:
			DeviceConfigurationMsg();
			virtual ~DeviceConfigurationMsg();

			virtual void ToJsonValue(web::json::value& obj) const override;
			virtual void FromJsonValue(web::json::value& obj) override;
			virtual std::wstring ToString() const override;
		};
	}
}