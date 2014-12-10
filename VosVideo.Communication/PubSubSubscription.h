#pragma once
#include <algorithm>
#include "TypeInfoWrapper.h"
#include "MessageReceiver.h"

namespace vosvideo
{
	namespace communication
	{
		class PubSubSubscription final
		{
		public:
			PubSubSubscription(std::vector<TypeInfoWrapper> types, MessageReceiver& messageReceiver);
			~PubSubSubscription(void);

			std::vector<TypeInfoWrapper>const & GetTypes() const;
			MessageReceiver& GetMessageReceiver() const;

		private:
			 std::vector<TypeInfoWrapper> types_;
			 MessageReceiver& messageReceiver_;
		};
	}
}


