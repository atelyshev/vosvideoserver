#pragma once
#include <thread>
#include <websocketpp/config/asio_client.hpp>
#include <websocketpp/client.hpp>

#include "VosVideo.Communication/WebsocketClientEngine.h"
#include "VosVideo.Communication/MessageReceiver.h"
#include "VosVideo.Data/DtoFactory.h"

namespace vosvideo
{
	namespace communication
	{
		namespace wspp
		{
			class WsppWebsocketClientEngine final : public WebsocketClientEngine
			{
			public:
//				typedef websocketpp::client<websocketpp::config::asio_tls_client> WsClient;
				typedef websocketpp::client<websocketpp::config::asio_client> WsClient;
				// pull out the type of messages sent by our config
//				typedef websocketpp::config::asio_tls_client::message_type::ptr MessagePtr;
				typedef websocketpp::config::asio_client::message_type::ptr MessagePtr;
				typedef websocketpp::lib::shared_ptr<boost::asio::ssl::context> ContextPtr;
				typedef WsClient::connection_ptr ConnectionPtr;

				WsppWebsocketClientEngine(std::shared_ptr<PubSubService> communicationMessageBroker);
				~WsppWebsocketClientEngine(void);

				virtual void Connect(std::wstring const& url);
				// There are no Receive() because messages coming as event OnWebsocketMessage
				virtual void Send(std::string const& msg);
				virtual void Close();

			private:
				void OnSocketInit(websocketpp::connection_hdl hdl);
				ContextPtr OnTlsInit(websocketpp::connection_hdl hdl);
				void OnOpened(websocketpp::connection_hdl hdl);
				void OnClosed(websocketpp::connection_hdl hdl);
				void OnFailed(websocketpp::connection_hdl hdl);
				void OnMessage(websocketpp::connection_hdl hdl, MessagePtr message);

				std::shared_ptr<std::thread> wsThread_;
				WsClient endPointClient_;
				ConnectionPtr connection_;
				data::DtoFactory dtoFactory_;
			};
		}
	}
}

