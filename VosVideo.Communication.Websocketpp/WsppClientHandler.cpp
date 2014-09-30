#include "stdafx.h"
#include <exception>
#include "WsppClientHandler.h"

using vosvideo::communication::wspp::WsppClientHandler;

WsppClientHandler::WsppClientHandler(void)
{
}


WsppClientHandler::~WsppClientHandler(void)
{
}

void WsppClientHandler::on_message( connection_ptr con, message_ptr msg )
{
	messageSignal_(msg);
}

void WsppClientHandler::on_close( connection_ptr con )
{
	con_ = connection_ptr();
	closeSignal_();
}

void WsppClientHandler::on_open( connection_ptr con )
{
	con_ = con;
	openSignal_();
}

void WsppClientHandler::on_fail( connection_ptr con )
{
	failSignal_();
}

boost::signals::connection WsppClientHandler::ConnectToFailSignal( boost::signal<void()>::slot_function_type subscriber )
{
	return failSignal_.connect(subscriber);
}

void WsppClientHandler::DisconnectFromFailSignal( boost::signals::connection connection )
{
	failSignal_.disconnect(connection);
}

boost::signals::connection WsppClientHandler::ConnectToOpenSignal( boost::signal<void()>::slot_function_type subscriber )
{
	return openSignal_.connect(subscriber);
}

void WsppClientHandler::DisconnectFromOpenSignal( boost::signals::connection connection )
{
	openSignal_.disconnect(connection);
}

boost::signals::connection WsppClientHandler::ConnectToCloseSignal( boost::signal<void()>::slot_function_type subscriber )
{
	return closeSignal_.connect(subscriber);
}

void WsppClientHandler::DisconnectFromCloseSignal( boost::signals::connection connection )
{
	closeSignal_.disconnect(connection);
}

boost::signals::connection WsppClientHandler::ConnectToMessageSignal( boost::signal<void(message_ptr)>::slot_function_type subscriber )
{
	return messageSignal_.connect(subscriber);
}

void WsppClientHandler::DisconnectFromMessageSignal( boost::signals::connection connection )
{
	messageSignal_.disconnect(connection);
}

void WsppClientHandler::send(std::string const& msg )
{
	con_->send(msg);
}

void WsppClientHandler::close()
{
	throw std::exception("not implemented");
}
