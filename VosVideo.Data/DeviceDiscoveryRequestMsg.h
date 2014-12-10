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

			virtual void ToJsonValue(web::json::value& obj) const;
			virtual void FromJsonValue(web::json::value& obj);
			virtual std::wstring ToString() const;
		};
	}
}
