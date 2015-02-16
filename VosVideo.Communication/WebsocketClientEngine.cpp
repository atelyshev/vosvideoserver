#include "stdafx.h"
#include "WebsocketClientEngine.h"

using namespace vosvideo::communication;

WebsocketClientEngine::WebsocketClientEngine(std::shared_ptr<PubSubService> pubsubService) : pubSubService_(pubsubService)
{
}


WebsocketClientEngine::~WebsocketClientEngine()
{
}

//boost::signals2::connection WebsocketClientEngine::ConnectToConnectionProblemSignal(boost::signals2::signal<void()>::slot_function_type subscriber)
//{
//	return connectionProblemSignal_.connect(subscriber);
//}
