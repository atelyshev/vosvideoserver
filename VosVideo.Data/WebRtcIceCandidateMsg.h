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
			~WebRtcIceCandidateMsg();

			virtual void FromJsonValue(web::json::value& obj);
			// from IceCandidateMsg interface
			virtual void GetIceCandidate(std::wstring& iceCandidate);
			// from MediaInfoMsg interface
			virtual void GetMediaInfo(web::json::value& mi);
		};
	}
}