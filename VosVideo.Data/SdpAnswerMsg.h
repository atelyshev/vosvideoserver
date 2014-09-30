#pragma once
#include "ReceivedData.h"

namespace vosvideo
{
	namespace data
	{
		class SdpAnswerMsg final : public ReceivedData
		{
		public:
			SdpAnswerMsg();
			~SdpAnswerMsg();

			virtual void FromJsonValue(web::json::value& obj);
			virtual std::wstring ToString() const;
		};
	}
}
