#pragma once
#include <cpprest/json.h>

namespace vosvideo
{
	namespace data
	{
		class IceCandidateMsg
		{
		public:
			IceCandidateMsg(){}
			virtual ~IceCandidateMsg(){}

			virtual void GetIceCandidate(std::wstring& iceCandidate) = 0;
		};
	}
}
