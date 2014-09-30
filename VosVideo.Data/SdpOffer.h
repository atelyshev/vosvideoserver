#pragma once
#include <cpprest/json.h>

namespace vosvideo
{
	namespace data
	{
		class SdpOffer
		{
		public:
			SdpOffer(){}
			virtual ~SdpOffer(){}

			virtual void GetSdpOffer(std::wstring& sdpOffer) = 0;
		};
	}
}
