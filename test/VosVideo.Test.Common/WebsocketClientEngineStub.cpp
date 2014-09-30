#include "stdafx.h"
#include "WebsocketClientEngineStub.h"

using namespace vosvideo::communication;

WebsocketClientEngineStub::WebsocketClientEngineStub(std::shared_ptr<PubSubService> communicationMessageBroker) : WebsocketClientEngine(communicationMessageBroker)
{
}


WebsocketClientEngineStub::~WebsocketClientEngineStub(void)
{
}

void WebsocketClientEngineStub::Connect(std::wstring const& url)
{

}

void WebsocketClientEngineStub::Send( std::string const& msg )
{
	
}

void WebsocketClientEngineStub::Close()
{

}
