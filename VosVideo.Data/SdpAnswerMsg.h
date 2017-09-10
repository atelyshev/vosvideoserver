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
			virtual ~SdpAnswerMsg();

			virtual void FromJsonValue(const web::json::value& obj) override;
			virtual std::wstring ToString() const override;
			web::json::value jObj_;
		};
	}
}
