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

			virtual void FromJsonValue(web::json::value& obj) override;
			// from IceCandidateMsg interface
			virtual void GetIceCandidate(std::wstring& iceCandidate) override;
			// from MediaInfoMsg interface
			virtual void GetMediaInfo(web::json::value& mi) override;
		};
	}
}