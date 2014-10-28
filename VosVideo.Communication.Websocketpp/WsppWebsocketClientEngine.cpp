#include "stdafx.h"
#include <vosvideocommon/SeverityLoggerMacros.h>
#include <vosvideocommon/StringUtil.h>
#include "VosVideo.Communication/TypeInfoWrapper.h"
#include "VosVideo.Data/WebsocketConnectionOpenedMsg.h"
#include "VosVideo.Data/DtoParseException.h"
#include "VosVideo.Data/WebSocketMessageParser.h"
#include "WsppWebsocketClientEngine.h"


using namespace std;
using namespace util;
using namespace vosvideo::communication::wspp;
using namespace vosvideo::communication;
using namespace vosvideo::data;
using namespace websocketpp;


WsppWebsocketClientEngine::WsppWebsocketClientEngine(std::shared_ptr<PubSubService> pubsubService) : WebsocketClientEngine(pubsubService)
{
	endPointClient_.set_access_channels(websocketpp::log::alevel::all);
	endPointClient_.set_error_channels(websocketpp::log::elevel::all);
	// Initialize ASIO
	endPointClient_.init_asio();
	// Register our handlers
	endPointClient_.set_socket_init_handler(std::bind(&WsppWebsocketClientEngine::OnSocketInit, this, placeholders::_1));
	// We need it in case end point is TLS enabled
	//endPointClient_.set_tls_init_handler(std::bind(&WsppWebsocketClientEngine::OnTlsInit, this, placeholders::_1));
	endPointClient_.set_message_handler(std::bind(&WsppWebsocketClientEngine::OnMessage, this, placeholders::_1, placeholders::_2));
	endPointClient_.set_open_handler(std::bind(&WsppWebsocketClientEngine::OnOpened, this, placeholders::_1));
	endPointClient_.set_close_handler(std::bind(&WsppWebsocketClientEngine::OnClosed, this, placeholders::_1));
	endPointClient_.set_fail_handler(std::bind(&WsppWebsocketClientEngine::OnFailed, this, placeholders::_1));
}


WsppWebsocketClientEngine::~WsppWebsocketClientEngine(void)
{
	Close();
	wsThread_->join();
}

void WsppWebsocketClientEngine::Connect(std::wstring const& wUri)
{
	//Need to convert to std::string. Websocketpp doesn't like std::wstring
	string uri = StringUtil::ToString(wUri); 
	websocketpp::lib::error_code ec;
	connection_ = endPointClient_.get_connection(uri, ec);

	if (ec)
	{
		endPointClient_.get_alog().write(websocketpp::log::alevel::app, ec.message());
	}

	endPointClient_.connect(connection_);

	// Start the ASIO io_service run loop
	wsThread_.reset(new std::thread([this](){endPointClient_.run();}));
}

void WsppWebsocketClientEngine::Send(std::string const& msg)
{
	endPointClient_.send(connection_->get_handle(), msg, frame::opcode::text);
}

void WsppWebsocketClientEngine::Close()
{
	websocketpp::lib::error_code ec;
	endPointClient_.close(connection_->get_handle(), websocketpp::close::status::going_away, "", ec);

	if (ec) 
	{
		LOG_ERROR("Error closing websocket connection " << ec.message());
	}
}

void WsppWebsocketClientEngine::OnSocketInit(connection_hdl hdl)
{
	LOG_TRACE("Web Socket init is done.");
}

WsppWebsocketClientEngine::ContextPtr WsppWebsocketClientEngine::OnTlsInit(connection_hdl hdl)
{
	LOG_TRACE("Web Socket TLS init is done.");
	ContextPtr ctx(new boost::asio::ssl::context(boost::asio::ssl::context::tlsv1));

	try 
	{
		ctx->set_options(boost::asio::ssl::context::default_workarounds |
			boost::asio::ssl::context::no_sslv2 |
			boost::asio::ssl::context::single_dh_use);
	}
	catch (std::exception& e) 
	{
		LOG_ERROR(e.what());
	}
	return ctx;
}

void WsppWebsocketClientEngine::OnOpened(connection_hdl hdl)
{
	auto dto = dtoFactory_.Create(MsgType::ConnectionOpenedMsg);

	pubSubService_->Publish(dto);
}

void WsppWebsocketClientEngine::OnClosed(connection_hdl hdl)
{
	LOG_DEBUG("Hit OnClosed");
}

void WsppWebsocketClientEngine::OnFailed(connection_hdl hdl)
{
	websocketpp::lib::error_code ec = connection_->get_ec();
	LOG_DEBUG("Connection failed, Error code: " << ec.message());
}

void WsppWebsocketClientEngine::OnMessage(websocketpp::connection_hdl hdl, MessagePtr message)
{
	string payload = message->get_payload();
	shared_ptr<WebSocketMessageParser> msgParser(new WebSocketMessageParser(payload));
	LOG_TRACE("Received message with payload:" << payload);

	auto dto = dtoFactory_.Create(msgParser->GetMessageType());
	dto->Init(msgParser);
	pubSubService_->Publish(dto);
}
