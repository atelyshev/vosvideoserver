#include "stdafx.h"
#include "WebsocketClient.h"

using namespace vosvideo::communication;

WebsocketClient::WebsocketClient(std::shared_ptr<WebsocketClientEngine> websocketClientEngine) : engine_(websocketClientEngine)
{
}


WebsocketClient::~WebsocketClient(void)
{
}

boost::signals2::connection WebsocketClient::ConnectToConnectionProblemSignal(boost::signals2::signal<void()>::slot_function_type subscriber)
{
	return engine_->ConnectToConnectionProblemSignal(subscriber);
}

void WebsocketClient::Connect(std::wstring const & url) const
{
	engine_->Connect(url);
}

void WebsocketClient::Send(const std::string &msg)
{
	engine_->Send(msg);
}

void WebsocketClient::Close()
{
	throw std::exception("not implemented yet");
}
