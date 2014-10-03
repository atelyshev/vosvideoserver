#include "stdafx.h"

#include <vosvideocommon/SeverityLoggerMacros.h>
#include "VosVideo.Communication/TypeInfoWrapper.h"
#include "VosVideo.Data/WebsocketConnectionOpenedMsg.h"
#include "VosVideo.Data/DtoParseException.h"
#include "VosVideo.Data/WebSocketMessageParser.h"
#include "WsppWebsocketClientEngine.h"
#include "WsppClientHandler.h"

using namespace std;
using namespace vosvideo::communication::wspp;
using namespace vosvideo::communication;
using namespace vosvideo::data;
using namespace websocketpp;

WsppWebsocketClientEngine::WsppWebsocketClientEngine(std::shared_ptr<PubSubService> pubsubService) : WebsocketClientEngine(pubsubService)
{
	clientHandler_ = boost::shared_ptr<WsppClientHandler>(new WsppClientHandler());
	client_ = std::shared_ptr<websocketpp::client>(new client(clientHandler_));
	client_->alog().unset_level(websocketpp::log::alevel::ALL);
	client_->elog().unset_level(websocketpp::log::elevel::ALL);
	client_->elog().set_level(websocketpp::log::elevel::RERROR);
	client_->elog().set_level(websocketpp::log::elevel::FATAL);

	openSignalConn_ = clientHandler_->ConnectToOpenSignal(boost::bind(&WsppWebsocketClientEngine::OnWebsocketOpened, this));
	closeSignalConn_ = clientHandler_->ConnectToCloseSignal(boost::bind(&WsppWebsocketClientEngine::OnWebsocketClosed, this));
	failedSignalConn_ = clientHandler_->ConnectToFailSignal(boost::bind(&WsppWebsocketClientEngine::OnWebsocketFailed, this));
	messageSignalConn_ = clientHandler_->ConnectToMessageSignal(boost::bind(&WsppWebsocketClientEngine::OnWebsocketMessage, this, _1));
}


WsppWebsocketClientEngine::~WsppWebsocketClientEngine(void)
{
}

void WsppWebsocketClientEngine::Connect(std::wstring const& url)
{
	//Need to convert to std::string. Websocketpp doesn't like std::wstring
	std::string convUrl(url.begin(), url.end());
	conn_ = client_->get_connection(convUrl);
	client_->connect(conn_);
	wsThread_ = boost::shared_ptr<boost::thread>(new boost::thread(boost::bind(&client::run, client_, false)));
}

void WsppWebsocketClientEngine::OnWebsocketOpened()
{
	auto dto = dtoFactory_.Create(MsgType::ConnectionOpenedMsg);

	pubSubService_->Publish(dto);
}

void WsppWebsocketClientEngine::OnWebsocketClosed()
{
	LOG_DEBUG("Hit OnWebsocketClosed");
}

void WsppWebsocketClientEngine::OnWebsocketFailed()
{
	boost::system::error_code err = conn_->get_system_fail_code();
	LOG_DEBUG("Connection failed, Error code: " << err.value());
}

void WsppWebsocketClientEngine::OnWebsocketMessage(boost::intrusive_ptr<websocketpp::message::data> message)
{
	string payload = message->get_payload();
	shared_ptr<WebSocketMessageParser> msgParser(new WebSocketMessageParser(payload));
	LOG_TRACE("Received message with payload:" << payload);

	auto dto = dtoFactory_.Create(msgParser->GetMessageType());
	dto->Init(msgParser);
	pubSubService_->Publish(dto);
}

void WsppWebsocketClientEngine::Send(std::string const& msg)
{
	clientHandler_->send(msg);
}

void WsppWebsocketClientEngine::Close()
{
	clientHandler_->close();
}
