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
			SdpAnswerMsg(const std::wstring& srvPeer, const std::wstring& clientPeer, const std::wstring& sdp, int devId);
			~SdpAnswerMsg();

			virtual void FromJsonValue(web::json::value& obj);
			virtual std::wstring ToString() const;
			web::json::value jObj_;
		};
	}
}
