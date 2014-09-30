#pragma once
#include <talk/base/scoped_ref_ptr.h>
#include <talk/app/webrtc/peerconnectioninterface.h>
#include <talk/base/physicalsocketserver.h>
#include "VosVideo.Communication/CommunicationManager.h"
#include "VosVideo.Data/DtoFactory.h"
#include "VosVideo.Camera/CameraDeviceManager.h"
#include "WebRtcPeerConnection.h"


namespace vosvideo
{
	namespace vvwebrtc
	{
		class WebRtcManager : public vosvideo::communication::MessageReceiver
		{
		public:
			WebRtcManager(
				std::shared_ptr<vosvideo::communication::CommunicationManager> communicationManager,
				std::shared_ptr<vosvideo::camera::CameraDeviceManager> deviceManager,
				std::shared_ptr<vosvideo::communication::PubSubService> pubsubService);
			~WebRtcManager();

			virtual void OnMessageReceived(const std::shared_ptr<vosvideo::data::ReceivedData> receivedMessage);
			void Shutdown();

		private:
			void AddStreams();
			void CreatePeerConnectionFactory();
			cricket::VideoCapturer* OpenVideoCaptureDevice();
			talk_base::scoped_refptr<webrtc::PeerConnectionInterface> CreatePeerConnection();
			void DeletePeerConnection(const std::wstring& fromPeer);
			int  CheckFinishingPeerConnections();
			void CreateClientPeerKey(const std::shared_ptr<vosvideo::data::ReceivedData> receivedMessage, std::wstring& clientPeerKey, int& devId);

			typedef std::map<std::wstring, talk_base::scoped_refptr<WebRtcPeerConnection> > WebRtcPeerConnectionMap;
			typedef std::map<std::wstring, std::shared_ptr<vosvideo::camera::CameraDeviceManager> > TestDevManagersMap;
			typedef std::vector<talk_base::scoped_refptr<WebRtcPeerConnection>> WebRtcPeerConnectionVector;
			talk_base::scoped_refptr<webrtc::PeerConnectionFactoryInterface> peer_connection_factory_;
			std::shared_ptr<vosvideo::communication::CommunicationManager> communicationManager_;
			std::shared_ptr<vosvideo::camera::CameraDeviceManager> deviceManager_;
			std::shared_ptr<vosvideo::communication::PubSubService> pubSubService_;
			WebRtcPeerConnectionMap peer_connections_;
			WebRtcPeerConnectionVector finishing_peer_connections_;
			TestDevManagersMap test_devmanagers_;
			talk_base::PhysicalSocketServer* physicalSocketServer_;
			talk_base::AutoThread* mainThread_;
			std::mutex mutex_;
		};
	}
}