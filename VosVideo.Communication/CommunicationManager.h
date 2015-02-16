#pragma once
#include "VosVideo.Data/SendData.h"
#include "HttpClient.h"
#include "WebsocketClient.h"
#include "PubSubService.h"

namespace vosvideo
{
	namespace communication
	{
		class CommunicationManager final
		{
		public:
			CommunicationManager(std::shared_ptr<HttpClient> httpClient, std::shared_ptr<WebsocketClient> websocketClient);	
			~CommunicationManager(void);

			concurrency::task<web::json::value> HttpGet(std::wstring const& path);
			concurrency::task<web::json::value> HttpPost(std::wstring const& path, web::json::value const& payload);
			void WebsocketSend(std::string const& message);
			void WebsocketClose();
			void WebsocketConnect(std::wstring const& path);
			// Add symbol \" to message string to make Json compatible
			static std::string StringToJson(std::string str);
			// WebSocket message formatter 
			static void CreateWebsocketMessageString(const std::wstring& fromPeer, 
												const std::wstring& toPeer, 
												std::shared_ptr<vosvideo::data::SendData> outMsg, 
												std::string& returnedMessage);
			static void CreateWebsocketMessageString(const std::wstring& fromPeer, 
												  const std::wstring& toPeer, 
												  vosvideo::data::MsgType msgType, 
												  const std::string& body, 
												  std::string& returnedMessage);

			boost::signals2::connection ConnectToWsConnectionProblemSignal(boost::signals2::signal<void()>::slot_function_type subscriber);
			boost::signals2::connection ConnectToRestConnectionProblemSignal(boost::signals2::signal<void()>::slot_function_type subscriber);

		private:
			std::shared_ptr<WebsocketClient> websocketClient_;
			std::shared_ptr<HttpClient> httpClient_;
			static const std::string msgFormat_;
		};
	}
}

