#pragma once
#include "ReceivedData.h"

namespace vosvideo
{
	namespace data
	{
		class IceCandidateResponseMsg final : public ReceivedData
		{
		public:
			IceCandidateResponseMsg();
			~IceCandidateResponseMsg();

			virtual void FromJsonValue(web::json::value& obj);
			virtual std::wstring ToString() const;
		};
	}
}
