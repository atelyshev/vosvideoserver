#include "stdafx.h"
#include "TestUtils.h"

using namespace vosvideo::communication;

std::shared_ptr<CommunicationManager> CreateCommManager()
{
	std::shared_ptr<HttpClientEngineStub> httpClientEngine(new HttpClientEngineStub());
	std::shared_ptr<HttpClient> httpClient(new HttpClient(httpClientEngine));
	std::shared_ptr<PubSubService> messageBroker;
	std::shared_ptr<WebsocketClientEngineStub> websocketClientEngine(new WebsocketClientEngineStub(messageBroker));
	std::shared_ptr<WebsocketClient> websocketClient(new WebsocketClient(websocketClientEngine));
	std::shared_ptr<CommunicationManager> commManager = std::shared_ptr<CommunicationManager>(new CommunicationManager(httpClient, websocketClient));
	return commManager;
}