#pragma once

#include <boost/asio/detail/config.hpp>
#include "webrtc/base/nethelpers.h"
#include "webrtc/base/signalthread.h"
#include "webrtc/base/sigslot.h"
#include "webrtc/base/physicalsocketserver.h"
#include "webrtc/base/scoped_ptr.h"
#include "PeerConnectionObserver.h"


namespace vosvideo
{
	namespace vvwebrtc
	{
		class PeerConnectionClientBase : public sigslot::has_slots<>,
			public rtc::MessageHandler 
		{
		public:
			enum class State 
			{
				NOT_CONNECTED,
				RESOLVING,
				SIGNING_IN,
				CONNECTED,
				SIGNING_OUT_WAITING,
				SIGNING_OUT,
			};

			PeerConnectionClientBase();
			virtual ~PeerConnectionClientBase();

			int id() const;
			bool is_connected() const ;
			const PeersMap& peers() const;

			void RegisterObserver(PeerConnectionClientObserver* callback);

			virtual void Connect(const std::string& server, int port,
				const std::string& client_name) = 0;

			virtual bool SendToPeer(int peer_id, const std::string& message) = 0;
			virtual bool SendHangUp(int peer_id) = 0;
			virtual bool IsSendingMessage() = 0;

			virtual bool SignOut() = 0;

			// implements the MessageHandler interface
			virtual void OnMessage(rtc::Message* msg) = 0;

		protected:
			PeerConnectionClientObserver* callback_;
			PeersMap peers_;
			State state_;
			std::string client_name_;
			int my_id_;
		};
	}
}

