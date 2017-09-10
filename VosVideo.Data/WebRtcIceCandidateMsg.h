#pragma once
#include "IceCandidateMsg.h"
#include "MediaInfo.h"
#include "ReceivedData.h"

namespace vosvideo
{
	namespace data
	{
		class WebRtcIceCandidateMsg : public ReceivedData, public IceCandidateMsg, public MediaInfo
		{
		public:
			WebRtcIceCandidateMsg();
			virtual ~WebRtcIceCandidateMsg();

			virtual void FromJsonValue(const web::json::value& obj) override;
			// from IceCandidateMsg interface
			virtual std::wstring GetIceCandidate() override;
			// from MediaInfoMsg interface
			virtual web::json::value GetMediaInfo() override;
		};
	}
}