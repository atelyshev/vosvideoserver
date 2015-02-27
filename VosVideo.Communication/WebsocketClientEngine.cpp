#include "stdafx.h"
#include "WebsocketClientEngine.h"

using namespace vosvideo::communication;

WebsocketClientEngine::WebsocketClientEngine(std::shared_ptr<PubSubService> pubsubService) : pubSubService_(pubsubService)
{
}


WebsocketClientEngine::~WebsocketClientEngine()
{
}
