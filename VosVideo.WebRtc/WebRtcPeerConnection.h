#pragma once
#include <webrtc/media/engine/webrtcvideocapturer.h>
#include <webrtc/base/scoped_ref_ptr.h>
#include <webrtc/api/peerconnectioninterface.h>
#include <webrtc/api/mediastreaminterface.h>
#include <webrtc/media/base/videosourceinterface.h>
#include <webrtc/base/win32socketinit.h>
#include <webrtc/base/win32socketserver.h>
#include <webrtc/base/json.h>

#include "VosVideo.Data/LiveVideoOfferMsg.h"
#include "VosVideo.Communication/CommunicationManager.h"
#include "VosVideo.Communication.InterprocessQueue/InterprocessQueueEngine.h"
#include "VosVideo.Data/SdpOffer.h"
#include "VosVideo.Data/WebRtcIceCandidateMsg.h"
#include "VosVideo.Camera/CameraDeviceManager.h"
#include "VosVideo.Camera/CameraVideoCapturer.h"
#include "VosVideo.CameraPlayer/CameraPlayerBase.h"

#include "PeerConnectionObserver.h"
#include "WebRtcMessageWrapper.h"

namespace vosvideo
{
	namespace vvwebrtc
	{
		enum class PeerConnectionMessages
		{
			DoInitSdp,
			DoInitIce,
			DoOnSuccess,
			DoOnIceCandidate,
			DoAddStreams,
			DoOnSignalChange,
			DoCloseCapturer
		};

		class DummySetSessionDescriptionObserver
			: public webrtc::SetSessionDescriptionObserver
		{
		public:
			static DummySetSessionDescriptionObserver* Create() 
			{
				return new rtc::RefCountedObject<DummySetSessionDescriptionObserver>();
			}

			virtual void OnSuccess() 
			{
				LOG_TRACE("");
			}

			virtual void OnFailure(const std::string& error) 
			{
				LOG_TRACE(error);
			}

		protected:
			DummySetSessionDescriptionObserver() {}
			virtual ~DummySetSessionDescriptionObserver() {}
		};

		class WebRtcPeerConnection : 
			public rtc::MessageHandler,
			public webrtc::PeerConnectionObserver,
			public webrtc::CreateSessionDescriptionObserver,
			public PeerConnectionClientObserver
		{
		public:
			WebRtcPeerConnection(std::wstring clientPeer, 
								 std::wstring srvPeer, 
								 vosvideo::cameraplayer::CameraPlayerBase* player,
//								 rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface> peer_connection_factory,
								 std::shared_ptr<vosvideo::communication::InterprocessQueueEngine> queueEng);

			virtual ~WebRtcPeerConnection();

			bool IsPeerConnectionFinished();
			void InitSdp(std::shared_ptr<vosvideo::data::SdpOffer> sdp);
			void InitIce(std::shared_ptr<vosvideo::data::WebRtcIceCandidateMsg> ice);
			void SetCurrentThread(rtc::Thread* commandThr);
			void SetDeviceManager(std::shared_ptr<vosvideo::camera::CameraDeviceManager> deviceManager, int devId, bool isShutdownOnClose);
			void Close();

		protected:
			// MessageHandler implementation. Marshals SignalStateChange onto thread_.
			virtual void OnMessage(rtc::Message* message);

			// PeerConnectionObserver implementation.
			virtual void OnSignalingChange(webrtc::PeerConnectionInterface::SignalingState new_state);
			virtual void OnAddStream(rtc::scoped_refptr<webrtc::MediaStreamInterface> stream);
			virtual void OnRemoveStream(rtc::scoped_refptr<webrtc::MediaStreamInterface> stream);
			// Triggered when a remote peer open a data channel.
			virtual void OnDataChannel(rtc::scoped_refptr<webrtc::DataChannelInterface> data_channel) {}
			virtual void OnRenegotiationNeeded() {}
			virtual void OnIceConnectionChange(webrtc::PeerConnectionInterface::IceConnectionState new_state) {}
			virtual void OnIceGatheringChange(webrtc::PeerConnectionInterface::IceGatheringState new_state) {}
			virtual void OnIceCandidate(const webrtc::IceCandidateInterface* candidate);
			virtual void OnIceChange() {}

			// CreateSessionDescriptionObserver implementation.
			virtual void OnSuccess(webrtc::SessionDescriptionInterface* desc);
			virtual void OnFailure(const std::string& error);

			// PeerConnectionClientObserver implementation.
			virtual void OnSignedIn();
			virtual void OnDisconnected();
			virtual void OnPeerConnected(int id, const std::string& name);
			virtual void OnPeerDisconnected(int id);
			virtual void OnMessageFromPeer(int peer_id, const std::string& message);
			virtual void OnMessageSent(int err);
			virtual void OnServerConnectionFailure();

		private:
			template <class T>
			class TypedMessagePtr : public rtc::MessageData 
			{
			public:
				explicit TypedMessagePtr(T* data) : data_(data) { }
				const T* data() const { return data_; }
				T* data() { return data_; }
			private:
				T* data_ = nullptr;
			};

			using MediaStreamPair = std::pair<std::string, rtc::scoped_refptr<webrtc::MediaStreamInterface> >;
			using MediaStreamMap = std::map<std::string, rtc::scoped_refptr<webrtc::MediaStreamInterface> >;

			void InitSdp_r(const std::string& sdpPayload);
			void InitIce_r(const Json::Value& jmessage);
			void AddStreams_r();
			void Close_r();
            void CreatePeerConnectionFactory();

			vosvideo::camera::CameraVideoCapturer* OpenVideoCaptureDevice();
			void ProcessSdpMessage(const std::string& message);
			void ProcessIceMessage(const std::string& message);
			rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface> peer_connection_factory_;
			rtc::scoped_refptr<webrtc::PeerConnectionInterface> peer_connection_;

			MediaStreamMap active_streams_;
			std::shared_ptr<vosvideo::camera::CameraDeviceManager> deviceManager_;
			std::shared_ptr<vosvideo::communication::InterprocessQueueEngine> queueEng_;
			std::string server_;
			std::wstring clientPeer_;
			std::wstring srvPeer_;
			rtc::Thread* commandThr_ = nullptr;
			vosvideo::camera::CameraVideoCapturer* videoCapturer_ = nullptr;
			vosvideo::cameraplayer::CameraPlayerBase* player_ = nullptr;
			bool isPeerConnectionFinished_ = false;
			bool isShutdownOnClose_ = false;
		};
	}
}