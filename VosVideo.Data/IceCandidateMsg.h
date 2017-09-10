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

			virtual std::wstring GetIceCandidate() = 0;
		};
	}
}
