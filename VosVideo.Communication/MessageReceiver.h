#pragma once
#include "VosVideo.Data/ReceivedData.h"

namespace vosvideo
{
	namespace communication
	{
		class MessageReceiver
		{
		public:
			MessageReceiver(void);
			virtual ~MessageReceiver(void);

			virtual void OnMessageReceived(const std::shared_ptr<vosvideo::data::ReceivedData> receivedMessage) = 0;
		};
	}
}
