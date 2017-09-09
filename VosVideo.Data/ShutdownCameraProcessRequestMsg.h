#pragma once
#include "ReceivedData.h"

namespace vosvideo
{
	namespace data
	{
		class ShutdownCameraProcessRequestMsg final : public ReceivedData
		{
		public:
			ShutdownCameraProcessRequestMsg();
			virtual ~ShutdownCameraProcessRequestMsg();

			virtual void FromJsonValue(web::json::value& obj) override;
			virtual std::wstring ToString() const override;

		private:
			web::json::value jObj_;
		};
	}
}
