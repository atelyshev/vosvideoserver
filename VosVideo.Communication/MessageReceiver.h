#pragma once
#include "VosVideo.Data/ReceivedData.h"

namespace vosvideo
{
	namespace communication
	{
		class MessageReceiver
		{
		public:
			MessageReceiver();
			virtual ~MessageReceiver();

			virtual void OnMessageReceived(std::shared_ptr<vosvideo::data::ReceivedData> receivedMessage) = 0;
		};
	}
}
