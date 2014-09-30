#pragma once
#include <websocketpp/include/roles/client.hpp>
#include <websocketpp/include/messages/data.hpp>
#include <websocketpp/include/websocketpp.hpp>

namespace vosvideo{
	namespace communication{
		namespace wspp{
			class WsppClientHandler final : public websocketpp::client::handler
			{
			public:
				WsppClientHandler(void);
				~WsppClientHandler(void);

				virtual void on_fail(connection_ptr con);
				virtual void on_open(connection_ptr con);
				virtual void on_close(connection_ptr con);
				virtual void on_message(connection_ptr con, message_ptr msg);

				// CLIENT API
				virtual void send(std::string const& msg);
				virtual void close();


				boost::signals::connection ConnectToFailSignal(boost::signal<void()>::slot_function_type subscriber);
				void DisconnectFromFailSignal(boost::signals::connection connection);
				boost::signals::connection ConnectToOpenSignal(boost::signal<void()>::slot_function_type subscriber);
				void DisconnectFromOpenSignal(boost::signals::connection connection);
				boost::signals::connection ConnectToCloseSignal(boost::signal<void()>::slot_function_type subscriber);
				void DisconnectFromCloseSignal(boost::signals::connection connection);
				boost::signals::connection ConnectToMessageSignal(boost::signal<void(boost::intrusive_ptr<websocketpp::message::data>)>::slot_function_type subscriber);
				void DisconnectFromMessageSignal(boost::signals::connection connection);


			private:
				connection_ptr con_;
				boost::signal<void()> failSignal_;
				boost::signal<void()> openSignal_;
				boost::signal<void()> closeSignal_;
				boost::signal<void(message_ptr)> messageSignal_;
			};
		}
	}
}

