#pragma once
#include <talk/base/scoped_ref_ptr.h>
#include <talk/app/webrtc/peerconnectioninterface.h>
#include <talk/base/physicalsocketserver.h>
#include "VosVideo.Communication/CommunicationManager.h"
#include "VosVideo.Communication/InterprocessComm.h"
#include "VosVideo.Communication.InterprocessQueue/InterprocessQueueEngine.h"
#include "VosVideo.Camera/CameraDeviceManager.h"
#include "VosVideo.Data/DtoFactory.h"
#include "WebRtcPeerConnection.h"


namespace vosvideo
{
	namespace vvwebrtc
	{
		class WebRtcDeviceBroker : public vosvideo::communication::MessageReceiver
		{
		public:
			WebRtcDeviceBroker(std::shared_ptr<vosvideo::communication::PubSubService> pubsubService, 
				std::shared_ptr<vosvideo::communication::InterprocessQueueEngine> queueEng);
			virtual ~WebRtcDeviceBroker();

			virtual void OnMessageReceived(const std::shared_ptr<vosvideo::data::ReceivedData> receivedMessage);

		private:
			typedef std::map<std::wstring, talk_base::scoped_refptr<WebRtcPeerConnection> > WebRtcPeerConnectionMap;
			typedef std::vector<talk_base::scoped_refptr<WebRtcPeerConnection>> WebRtcPeerConnectionVector;
			typedef std::unordered_map<std::wstring, std::vector<std::shared_ptr<vosvideo::data::ReceivedData> >> WebRtcDeferredIceMap;
			void CreatePeerConnectionFactory();
			void DeleteAllPeerConnections();
			void DeletePeerConnection(const std::wstring& fromPeer);
			int CheckFinishingPeerConnections();

			std::shared_ptr<vosvideo::communication::PubSubService> pubSubService_;
			int activeDeviseId_;
			std::wstring deviceName_;
			talk_base::AutoThread* mainThread_;
			talk_base::PhysicalSocketServer* physicalSocketServer_;
			WebRtcPeerConnectionMap peer_connections_;
			WebRtcPeerConnectionVector finishing_peer_connections_;
			WebRtcDeferredIceMap deferredIce_;
			talk_base::scoped_refptr<webrtc::PeerConnectionFactoryInterface> peer_connection_factory_;
			std::shared_ptr<vosvideo::communication::InterprocessQueueEngine> queueEng_;
			vosvideo::camera::CameraPlayer* player_;
			std::mutex mutex_;
			bool inShutdown_;
		};
	}
}
