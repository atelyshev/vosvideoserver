#pragma once
#include <cpprest/containerstream.h>
#include <cpprest/streams.h>
#include <cpprest/ws_client.h>
#include "VosVideo.Communication/WebsocketClientEngine.h"
#include "VosVideo.Communication/MessageReceiver.h"
#include "VosVideo.Data/DtoFactory.h"

namespace vosvideo
{
	namespace communication
	{
		namespace casablanca
		{
			class CbWebsocketClientEngine final : public WebsocketClientEngine
			{
			public:

				CbWebsocketClientEngine(std::shared_ptr<PubSubService> communicationMessageBroker);
				~CbWebsocketClientEngine(void);

				virtual void Connect(std::wstring const& url);
				virtual void Send(std::string const& msg);
				virtual void Close();

			private:

				void StartListeningForMessages();
				pplx::task<void> AsyncDoWhile(std::function<pplx::task<bool>(void)> func);
				pplx::task<bool> DoWhileIteration(std::function<pplx::task<bool>(void)> func);
				pplx::task<bool> DoWhileImpl(std::function<pplx::task<bool>(void)> func);

				web::web_sockets::client::websocket_client _client;
				vosvideo::data::DtoFactory _dtoFactory;
			};
		}
	}
}