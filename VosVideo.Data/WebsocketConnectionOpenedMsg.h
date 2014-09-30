#pragma once
#include "ReceivedData.h"

namespace vosvideo
{
	namespace data
	{
		class WebsocketConnectionOpenedMsg final : public ReceivedData
		{
		public:
			WebsocketConnectionOpenedMsg();
			~WebsocketConnectionOpenedMsg();

			virtual void FromJsonValue(web::json::value& obj);
		};
	}
}

