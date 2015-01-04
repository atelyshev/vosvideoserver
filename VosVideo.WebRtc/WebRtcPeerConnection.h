#pragma once
#define HAVE_WEBRTC_VIDEO
#include <talk/media/webrtc/webrtcvideocapturer.h>
#undef HAVE_WEBRTC_VIDEO

#include <webrtc/base/scoped_ref_ptr.h>
#include <talk/app/webrtc/peerconnectioninterface.h>
#include <talk/app/webrtc/mediastreaminterface.h>
#include <talk/app/webrtc/videosourceinterface.h>
#include <webrtc/base/win32socketinit.h>
#include <webrtc/base/win32socketserver.h>
#include <webrtc/base/json.h>
#include <vosvideocommon/SeverityLoggerMacros.h>
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
			~DummySetSessionDescriptionObserver() {}
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
								 rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface> peer_connection_factory,
								 std::shared_ptr<vosvideo::communication::InterprocessQueueEngine> queueEng);

			virtual ~WebRtcPeerConnection();

			bool IsPeerConnectionFinished();
			void InitSdp(const std::shared_ptr<vosvideo::data::SdpOffer> sdp);
			void InitIce(const std::shared_ptr<vosvideo::data::WebRtcIceCandidateMsg> ice);
			void SetCurrentThread(rtc::Thread* commandThr);
			void SetDeviceManager(std::shared_ptr<vosvideo::camera::CameraDeviceManager> deviceManager, int devId, bool isShutdownOnClose);
			void Close();

		protected:
			// PeerConnectionObserver implementation.
			virtual void OnError();
			virtual void OnAddStream(webrtc::MediaStreamInterface* stream);
			virtual void OnRemoveStream(webrtc::MediaStreamInterface* stream);
			virtual void OnIceCandidate(const webrtc::IceCandidateInterface* candidate);
			virtual void OnStateChange(webrtc::PeerConnectionObserver::StateType state_changed);
			virtual void OnSignalingChange(webrtc::PeerConnectionInterface::SignalingState new_state);
			virtual void OnRenegotiationNeeded() {}
			virtual void OnIceChange() {}
			// Triggered when a remote peer open a data channel.
			virtual void OnDataChannel(webrtc::DataChannelInterface* data_channel){}

			// PeerConnectionClientObserver implementation.
			virtual void OnSignedIn();
			virtual void OnDisconnected();
			virtual void OnPeerConnected(int id, const std::string& name);
			virtual void OnPeerDisconnected(int id);
			virtual void OnMessageFromPeer(int peer_id, const std::string& message);
			virtual void OnMessageSent(int err);
			virtual void OnServerConnectionFailure();

			// CreateSessionDescriptionObserver implementation.
			virtual void OnSuccess(webrtc::SessionDescriptionInterface* desc);
			virtual void OnFailure(const std::string& error);

			// Marshals SignalStateChange onto thread_.
			virtual void OnMessage(rtc::Message* message);

		private:
			template <class T>
			class TypedMessagePtr : public rtc::MessageData 
			{
			public:
				explicit TypedMessagePtr(T* data) : data_(data) { }
				const T* data() const { return data_; }
				T* data() { return data_; }
			private:
				T* data_;
			};

			typedef std::pair<std::string, rtc::scoped_refptr<webrtc::MediaStreamInterface> > MediaStreamPair;
			typedef std::map<std::string, rtc::scoped_refptr<webrtc::MediaStreamInterface> > MediaStreamMap;


			void InitSdp_r(const std::string& sdpPayload);
			void InitIce_r(const Json::Value& jmessage);
			void AddStreams_r();
			void Close_r();

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
			rtc::Thread* commandThr_;
			vosvideo::camera::CameraVideoCapturer* videoCapturer_;
			vosvideo::cameraplayer::CameraPlayerBase* player_;
			bool isPeerConnectionFinished_;
			bool isShutdownOnClose_;
		};
	}
}