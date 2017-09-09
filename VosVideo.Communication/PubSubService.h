#pragma once
#include "VosVideo.Data/ReceivedData.h"
#include "PubSubSubscription.h"
#include "MessageReceiver.h"
#include "TypeInfoWrapper.h"


namespace vosvideo
{
	namespace communication
	{
		class PubSubService final
		{
		public:
			PubSubService() : subscribercount_(0){};
			virtual ~PubSubService(){};

			int GetSubscriberCount(){ return subscribercount_;}

			void Publish(std::shared_ptr<vosvideo::data::ReceivedData> receivedData);	
			std::shared_ptr<PubSubSubscription> Subscribe(std::vector<TypeInfoWrapper> types, MessageReceiver& messageReceiver);
			void UnSubscribe(std::shared_ptr<PubSubSubscription> subscription);
		private:
			int subscribercount_;
			std::vector<std::shared_ptr<PubSubSubscription>> subscriptions_;
		};
	}
}

