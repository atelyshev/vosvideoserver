#include "stdafx.h"
#include <ppltasks.h>
#include <vosvideocommon/SeverityLoggerMacros.h>
#include "PubSubService.h"

using namespace std;
using namespace vosvideo::communication;
using namespace concurrency;


void PubSubService::Publish(shared_ptr<vosvideo::data::ReceivedData> receivedData)
{
	for_each
		(
			subscriptions_.begin(), subscriptions_.end(), 
			[receivedData](std::shared_ptr<PubSubSubscription> subscription)
			{

				concurrency::task<void> publishTask
					(
						[receivedData, subscription]() 
						{ 
							std::vector<TypeInfoWrapper> types = subscription->GetTypes();
							for(auto it = types.begin(); it != types.end(); ++it)
							{
								if(it->Get() == typeid(*receivedData))
								{
									MessageReceiver& receiver = subscription->GetMessageReceiver();
									receiver.OnMessageReceived(receivedData);
								}
							}
						}
				   );
			}
		);
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
