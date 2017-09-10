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

			virtual void FromJsonValue(const web::json::value& obj) override;

			// from SdpOfferMsg interface
			virtual std::wstring GetSdpOffer() override;
			// from MediaInfoMsg interface
			virtual web::json::value GetMediaInfo() override;
		};
	}
}