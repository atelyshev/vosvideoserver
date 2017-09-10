#pragma once
#include "ReceivedData.h"

namespace vosvideo
{
	namespace data
	{
		class DeviceDiscoveryRequestMsg final : public ReceivedData
		{
		public:
			DeviceDiscoveryRequestMsg();
			virtual ~DeviceDiscoveryRequestMsg();

			virtual web::json::value ToJsonValue() const override;
			virtual void FromJsonValue(const web::json::value& obj) override;
			virtual std::wstring ToString() const override;
		};
	}
}
