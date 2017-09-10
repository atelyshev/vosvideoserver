#pragma once
#include "ReceivedData.h"

namespace vosvideo
{
	namespace data
	{
		class WebsocketConnectionClosedMsg final : public ReceivedData
		{
		public:
			WebsocketConnectionClosedMsg();
			virtual ~WebsocketConnectionClosedMsg();

			virtual void FromJsonValue(const web::json::value& obj) override;
		};
	}
}

