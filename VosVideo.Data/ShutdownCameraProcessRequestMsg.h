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
			~ShutdownCameraProcessRequestMsg();

			virtual void FromJsonValue(web::json::value& obj);
			virtual std::wstring ToString() const;

		private:
			web::json::value jObj_;
		};
	}
}
