#include "stdafx.h"
#include <ppltasks.h>
#include "PubSubService.h"

using namespace std;
using namespace vosvideo::communication;
using namespace concurrency;

void PubSubService::Publish(shared_ptr<vosvideo::data::ReceivedData> receivedData)
{
	LOG_TRACE("Publishing data: " << receivedData->ToString());

	for(auto s : subscriptions_)
	{
		concurrency::task<void> publishTask([receivedData, s]()
		{
			auto types = s->GetTypes();
			for (const auto& type : types)
			{
				if (type.Get() == typeid(*receivedData))
				{
					MessageReceiver& receiver = s->GetMessageReceiver();
					try
					{
						receiver.OnMessageReceived(receivedData);
					}
					catch (...)
					{
#ifdef _DEBUG
						LOG_DEBUG("Calling windows debugger.");
						__asm int 3;
#endif
					}
				}
			}
		});
	}
}


std::shared_ptr<PubSubSubscription> PubSubService::Subscribe(std::vector<TypeInfoWrapper> types, MessageReceiver& messageReceiver)
{
	std::shared_ptr<PubSubSubscription> subscription(new PubSubSubscription(types, messageReceiver));
	subscriptions_.push_back(subscription);
	subscribercount_++;
	return subscription;
}

void PubSubService::UnSubscribe(std::shared_ptr<PubSubSubscription> subscription)
{
	if(std::count(subscriptions_.begin(), subscriptions_.end(), subscription) > 0)
	{
		subscriptions_.erase(std::remove(subscriptions_.begin(), subscriptions_.end(), subscription), subscriptions_.end());
		subscribercount_--;
		return;
	}
	throw std::runtime_error("Provided subscription is not recognized");
}
