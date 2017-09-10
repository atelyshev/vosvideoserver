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

			virtual web::json::value ToJsonValue() const override;
			virtual void FromJsonValue(const web::json::value& obj) override;
		};
	}
}