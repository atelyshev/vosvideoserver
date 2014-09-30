#pragma once
#include "VosVideo.Communication/WebsocketClientEngine.h"
#include "VosVideo.Communication/MessageReceiver.h"
#include "VosVideo.Data/DtoFactory.h"
#include "WsppClientHandler.h"

namespace vosvideo
{
	namespace communication
	{
		namespace wspp
		{
			class WsppWebsocketClientEngine final : public WebsocketClientEngine
			{
			public:
				WsppWebsocketClientEngine(std::shared_ptr<PubSubService> communicationMessageBroker);
				~WsppWebsocketClientEngine(void);

				virtual void Connect(std::wstring const& url);
				virtual void Send(std::string const& msg);
				virtual void Close();

			private:
				void OnWebsocketOpened();
				void OnWebsocketClosed();
				void OnWebsocketFailed();
				void OnWebsocketMessage(boost::intrusive_ptr<websocketpp::message::data> message);

				boost::shared_ptr<WsppClientHandler> clientHandler_;
				std::shared_ptr<::websocketpp::client> client_;
				boost::shared_ptr<boost::thread> wsThread_;
				websocketpp::client::connection_ptr conn_;
				data::DtoFactory dtoFactory_;

				boost::signals::connection openSignalConn_;		
				boost::signals::connection closeSignalConn_;
				boost::signals::connection failedSignalConn_; 
				boost::signals::connection messageSignalConn_;
			};
		}
	}
}

