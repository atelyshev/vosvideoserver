#include "stdafx.h"
#include <exception>
#include <boost/algorithm/string/replace.hpp>
#include "WsClientHandler.h"

using vosvideo::vvwebrtc::WsClientHandler;

void WsClientHandler::on_fail(connection_ptr con) 
{
	std::cout << "Connection failed" << std::endl;
}

void WsClientHandler::on_open(connection_ptr con) 
{
	con_ = con;
	std::cout << "Successfully connected" << std::endl;
	SignalOpenEvent();
}

void WsClientHandler::on_close(connection_ptr con) 
{
	con_ = connection_ptr();

	std::cout << "client was disconnected" << std::endl;
	SignalCloseEvent();
}

void WsClientHandler::on_message(connection_ptr con, message_ptr msg) 
{
	SignalMessageEvent(msg->get_payload());
}

// CLIENT API
// client api methods will be called from outside the io_service.run thread
//  they need to be careful to not touch unsyncronized member variables.
void WsClientHandler::send(const std::string &msg) 
{
	if (!con_) 
	{
		std::cerr << "Error: no connected session" << std::endl;
		return;
	}

	if (msg == "/list") 
	{
		std::cout << "list all participants" << std::endl;
	}
	else if (msg == "/close") 
	{
		close();
	}
	else 
	{
		con_->send(msg);
	}
}

void WsClientHandler::close() 
{
	if (!con_) 
	{
		std::cerr << "Error: no connected session" << std::endl;
		return;
	}
	con_->close(websocketpp::close::status::GOING_AWAY,"");
}

