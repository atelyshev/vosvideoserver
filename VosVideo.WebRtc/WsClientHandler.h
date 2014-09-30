#pragma once
#include <queue>
#include <talk/base/sigslot.h>
#include "websocketpp/include/roles/client.hpp"
#include "websocketpp/include/websocketpp.hpp"

namespace vosvideo
{
	namespace vvwebrtc
	{
		class WsClientHandler : public websocketpp::client::handler 
		{
		public:
			WsClientHandler() {}
			virtual ~WsClientHandler() {}

			void on_fail(connection_ptr con);

			// connection to chat room complete
			void on_open(connection_ptr con);

			// connection to chat room closed
			void on_close(connection_ptr con);

			// got a new message from server
			void on_message(connection_ptr con, message_ptr msg);

			// CLIENT API
			void send(const std::string &msg);
			void close();

			//Signals
			sigslot::signal0<> SignalOpenEvent;        
			sigslot::signal0<> SignalCloseEvent;       
			sigslot::signal0<> SignalFailEvent;    
			sigslot::signal1<const std::string&> SignalMessageEvent;

		private:
			std::queue<std::string> msg_queue_;
			connection_ptr con_;
		};
	}
}

