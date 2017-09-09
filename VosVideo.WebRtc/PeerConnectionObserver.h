#pragma once

#include <unordered_map>

namespace vosvideo
{
	namespace vvwebrtc
	{
		using PeersMap = std::unordered_map<int, std::string>;

		class PeerConnectionClientObserver 
		{
		public:
			virtual void OnSignedIn() = 0;  // Called when we're logged on.
			virtual void OnDisconnected() = 0;
			virtual void OnPeerConnected(int id, const std::string& name) = 0;
			virtual void OnPeerDisconnected(int peer_id) = 0;
			virtual void OnMessageFromPeer(int peer_id, const std::string& message) = 0;
			virtual void OnMessageSent(int err) = 0;
			virtual void OnServerConnectionFailure() = 0;

		protected:
			virtual ~PeerConnectionClientObserver() {}
		};
	}
}



