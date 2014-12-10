#pragma once
#include <boost/shared_ptr.hpp>
#include "PeerConnectionClientBase.h"

namespace vosvideo
{
	namespace vvwebrtc
	{
		class PeerConnectionClientManager final
		{
		public:
			PeerConnectionClientManager();
			~PeerConnectionClientManager();

		protected:
			std::shared_ptr<PeerConnectionClientBase> client_;
		};
	}
}

