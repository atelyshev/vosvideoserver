#pragma once
#include "ReceivedData.h"

namespace vosvideo
{
	namespace data
	{
		class DeletePeerConnectionRequestMsg final : public ReceivedData
		{
		public:
			DeletePeerConnectionRequestMsg();
			virtual ~DeletePeerConnectionRequestMsg();

			virtual void ToJsonValue(web::json::value& obj) const;
			virtual void FromJsonValue(web::json::value& obj);
		};
	}
}