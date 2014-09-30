#pragma once
#include "ReceivedData.h"

namespace vosvideo
{
	namespace data
	{
		class DeviceStopTestMsg final : public ReceivedData
		{
		public:
			DeviceStopTestMsg();
			virtual ~DeviceStopTestMsg();

			virtual void ToJsonValue(web::json::value& obj) const;
			virtual void FromJsonValue(web::json::value& obj);
		};
	}
}