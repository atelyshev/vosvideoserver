#include "stdafx.h"
#include "PubSubSubscription.h"

using namespace vosvideo::communication;

PubSubSubscription::PubSubSubscription(std::vector<TypeInfoWrapper> types, MessageReceiver& messageReceiver) : 
	types_(types), messageReceiver_(messageReceiver)
{
}


PubSubSubscription::~PubSubSubscription()
{
}

std::vector<TypeInfoWrapper> const & PubSubSubscription::GetTypes() const
{
	return types_;
}

MessageReceiver& PubSubSubscription::GetMessageReceiver() const 
{
	return messageReceiver_;
}
