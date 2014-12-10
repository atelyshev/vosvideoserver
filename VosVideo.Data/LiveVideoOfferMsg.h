#pragma once
#include "SdpOffer.h"
#include "MediaInfo.h"
#include "ReceivedData.h"

namespace vosvideo
{
	namespace data
	{
		class LiveVideoOfferMsg final : public ReceivedData, public SdpOffer, public MediaInfo
		{
		public:
			LiveVideoOfferMsg();
			virtual ~LiveVideoOfferMsg();

			virtual void FromJsonValue(web::json::value& obj);

			// from SdpOfferMsg interface
			virtual void GetSdpOffer(std::wstring& sdpOffer);
			// from MediaInfoMsg interface
			virtual void GetMediaInfo(web::json::value& mi);
		};
	}
}