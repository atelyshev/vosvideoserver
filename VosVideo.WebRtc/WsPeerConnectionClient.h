#pragma once

#include "talk/base/sigslot.h"
#include "talk/base/signalthread.h"
#include "talk/base/json.h"
#include "peerconnectionclientbase.h"
#include "wsclienthandler.h"

namespace vosvideo{
	namespace vvwebrtc{
		//WebSocket based peer connection client
		class WsPeerConnectionClient : public PeerConnectionClientBase
		{
		public:

			WsPeerConnectionClient();
			virtual ~WsPeerConnectionClient();

			void RegisterObserver(PeerConnectionClientObserver* callback);

			void Connect(const std::string& server, int port,
				const std::string& client_name);

			bool SendToPeer(int peer_id, const std::string& message);
			bool SendHangUp(int peer_id);
			bool IsSendingMessage();

			bool SignOut();

			// implements the MessageHandler interface
			void OnMessage(talk_base::Message* msg);

		private:
			void DoConnect(const std::string& server, const int port, const std::string& client_name);
			void OnWebsocketOpen();
			void OnWebsocketFail();
			void OnWebsocketMessage(const std::string& message);
			void OnWebsocketClose();

			void Close();

			void ConnectWithWebsocketServer(const std::string& server, const std::string strPeerId);

			void SubscribeToWsClientEvents();

			void SendOfferRequest(const Json::Value& root, int peer_id);
			void SendIceCandidate(const Json::Value& root);


			boost::shared_ptr<WsClientHandler> ws_client_handler_;
			boost::shared_ptr<websocketpp::client> endpoint_;
			websocketpp::client::connection_ptr conn_;
			boost::shared_ptr<boost::thread> wsThread_;
		};
	}
}


