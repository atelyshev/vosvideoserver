#pragma once
#include <webrtc/base/scoped_ref_ptr.h>
#include <webrtc/api/peerconnectioninterface.h>
#include <webrtc/base/physicalsocketserver.h>
#include "VosVideo.Communication/CommunicationManager.h"
#include "VosVideo.Communication/InterprocessComm.h"
#include "VosVideo.Communication.InterprocessQueue/InterprocessQueueEngine.h"
#include "VosVideo.Camera/CameraDeviceManager.h"
#include "VosVideo.CameraPlayer/CameraPlayerBase.h"
#include "VosVideo.Data/DtoFactory.h"
#include "WebRtcPeerConnection.h"


namespace vosvideo
{
	namespace vvwebrtc
	{
		class WebRtcManager : public vosvideo::communication::MessageReceiver
		{
		public:
			WebRtcManager(std::shared_ptr<vosvideo::communication::PubSubService> pubsubService, 
				std::shared_ptr<vosvideo::communication::InterprocessQueueEngine> queueEng);
			virtual ~WebRtcManager();

			virtual void OnMessageReceived(std::shared_ptr<vosvideo::data::ReceivedData> receivedMessage);

		private:
			using WebRtcPeerConnectionMap = std::map<std::wstring, rtc::scoped_refptr<WebRtcPeerConnection> >;
			using WebRtcPeerConnectionVector = std::vector<rtc::scoped_refptr<WebRtcPeerConnection>>;
			using  WebRtcDeferredIceMap = std::unordered_map<std::wstring, std::vector<std::shared_ptr<vosvideo::data::ReceivedData> >>;

			void CreatePeerConnectionFactory();
			void DeleteAllPeerConnections();
			void DeletePeerConnection(const std::wstring& fromPeer);
			int RemoveFinishedPeerConnections();
			void Shutdown();

			std::shared_ptr<vosvideo::communication::PubSubService> pubSubService_;
			int activeDeviseId_;
			std::wstring deviceName_;
//			rtc::AutoThread* mainThread_;
            rtc::Thread* mainThread_ = nullptr;
			rtc::PhysicalSocketServer* physicalSocketServer_ = nullptr;
			WebRtcPeerConnectionMap peer_connections_;
			WebRtcPeerConnectionVector finishing_peer_connections_;
			WebRtcDeferredIceMap deferredIce_;
			rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface> peer_connection_factory_;
			std::shared_ptr<vosvideo::communication::InterprocessQueueEngine> queueEng_;
			vosvideo::cameraplayer::CameraPlayerBase* player_ = nullptr;
			std::mutex mutex_;
			bool inShutdown_ = false;
			Concurrency::timer<WebRtcManager*>* isaliveTimer_ = nullptr; 
			const static int isaliveTimeout_ = 60000; // 1 min
		};
	}
}
