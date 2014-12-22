#include "stdafx.h"
#include <ppltasks.h>
#include "PubSubTest.h"

using namespace testing::internal;
using namespace vosvideo::data;

int Subscribe();
int UnSubscribe();
void UnSubscribeWithNoSubscription();
bool Publish();

TEST(VosVideoCommunicationPubSub, Subscribe)
{
	EXPECT_NO_THROW(Subscribe());
	EXPECT_EQ(1, Subscribe());
}

TEST(VosVideoCommunicationPubSub, UnSubscribe)
{
	EXPECT_NO_THROW(UnSubscribe());
	EXPECT_EQ(0, UnSubscribe());
}

TEST(VosVideoCommunicationPubSub, UnSubscribeWithNoSubscription)
{
	EXPECT_THROW(UnSubscribeWithNoSubscription(), std::runtime_error);
}

TEST(VosVideoCommunicationPubSub, Publish)
{
	EXPECT_TRUE(Publish());
}

int Subscribe()
{
	PubSubService pubsubService;
	MessageReceiverStub messageReceiverStub;
	TypeInfoWrapper subcriptionTypeStub(typeid(SubscriptionTypeStub));
	std::vector<TypeInfoWrapper> types;
	types.push_back(subcriptionTypeStub);
	pubsubService.Subscribe(types, messageReceiverStub);
	return pubsubService.GetSubscriberCount();
}

int UnSubscribe()
{
	PubSubService pubsubService;
	MessageReceiverStub messageReceiverStub;
	TypeInfoWrapper subcriptionTypeStub(typeid(SubscriptionTypeStub));
	std::vector<TypeInfoWrapper> types;
	types.push_back(subcriptionTypeStub);
	std::shared_ptr<PubSubSubscription> subscription = pubsubService.Subscribe(types, messageReceiverStub);
	pubsubService.UnSubscribe(subscription);
	return pubsubService.GetSubscriberCount();
}

void UnSubscribeWithNoSubscription()
{
	PubSubService pubsubService;
	MessageReceiverStub messageReceiverStub;
	TypeInfoWrapper subcriptionTypeStub(typeid(SubscriptionTypeStub));
	std::vector<TypeInfoWrapper> types;
	types.push_back(subcriptionTypeStub);
	std::shared_ptr<PubSubSubscription> subscription;
	pubsubService.UnSubscribe(subscription);
}

bool Publish()
{
	PubSubService pubsubService;
	std::shared_ptr<ReceivedDataStub> receivedData(new ReceivedDataStub());

	MessageReceiverStub messageReceiverStub;
	HANDLE hWaitHandle = CreateEvent(NULL, true, false, L"PublishEvent");
	messageReceiverStub.SetWaitHandle(hWaitHandle);
	TypeInfoWrapper subcriptionTypeStub(typeid(ReceivedDataStub));
	std::vector<TypeInfoWrapper> types;
	types.push_back(subcriptionTypeStub);

	concurrency::task<void> tSubscribe
		(
			[&]()
				{
					std::shared_ptr<PubSubSubscription> subscription = pubsubService.Subscribe(types, messageReceiverStub);
				}
		);

	  tSubscribe.then
		(
			[&]()
				{
					pubsubService.Publish(receivedData);	
				}
		);
	

	WaitForSingleObject(hWaitHandle, 2000);

	bool messageReceived = messageReceiverStub.GetMessageReceived();
	return messageReceived;
}