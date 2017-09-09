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

			virtual void FromJsonValue(web::json::value& obj) override;
		};
	}
}

