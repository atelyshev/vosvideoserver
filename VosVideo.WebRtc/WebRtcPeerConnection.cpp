#include "stdafx.h"
#include <webrtc/api/mediaconstraintsinterface.h>

#include "VosVideo.Camera/CameraVideoCapturer.h"
#include "VosVideo.Data/CameraConfMsg.h"
#include "VosVideo.Data/SdpAnswerMsg.h"
#include "VosVideo.Data/IceCandidateResponseMsg.h"
#include "VosVideo.Data/RtbcDeviceErrorOutMsg.h"
#include "VosVideo.Communication/CommunicationManager.h"
#include "VosVideo.CameraPlayer/CameraPlayerBase.h"

#include "WebRtcException.h"
#include "WebRtcPeerConnection.h"
#include "defaults.h"
#include "MediaConstraints.h"

using namespace std;
using namespace util;
using vosvideo::communication::CommunicationManager;
using namespace vosvideo::vvwebrtc;
using namespace vosvideo::data;
using namespace vosvideo::camera;
using namespace vosvideo::cameraplayer;

// Names used for a IceCandidate JSON object.
const char kCandidateSdpMidName[] = "sdpMid";
const char kCandidateSdpMlineIndexName[] = "sdpMLineIndex";
const char kCandidateSdpName[] = "candidate";

// Names used for a SessionDescription JSON object.
const char kSessionDescriptionTypeName[] = "type";
const char kSessionDescriptionSdpName[] = "sdp";


WebRtcPeerConnection::WebRtcPeerConnection(wstring clientPeer,
										   wstring srvPeer,
										   CameraPlayerBase* player,
										   std::shared_ptr<vosvideo::communication::InterprocessQueueEngine> queueEng): 
	clientPeer_(clientPeer),
	srvPeer_(srvPeer),
	commandThr_(nullptr),
	videoCapturer_(nullptr),
	player_(player),
	queueEng_(queueEng),
	isPeerConnectionFinished_(false),
	isShutdownOnClose_(false)
{
}

WebRtcPeerConnection::~WebRtcPeerConnection()
{
	peer_connection_ = nullptr;
}

void WebRtcPeerConnection::SetCurrentThread(rtc::Thread* commandThr)
{
	commandThr_ = commandThr;
}

void WebRtcPeerConnection::SetDeviceManager(std::shared_ptr<vosvideo::camera::CameraDeviceManager> deviceManager, int devId, bool isShutdownOnClose)
{
	deviceManager_ = deviceManager;
	isShutdownOnClose_ = isShutdownOnClose;
}

void WebRtcPeerConnection::OnMessage(rtc::Message* message) 
{
	switch (message->message_id) 
	{
	case (uint32_t)PeerConnectionMessages::DoInitSdp:
		{
			shared_ptr<rtc::TypedMessageData<string>> data(static_cast<rtc::TypedMessageData<string>*>(message->pdata));
			string wrappedMessage = data->data();
			InitSdp_r(wrappedMessage);
			break;
		}
	case (uint32_t)PeerConnectionMessages::DoInitIce:
		{
			shared_ptr<rtc::TypedMessageData<Json::Value>> 
				data(static_cast<rtc::TypedMessageData<Json::Value>*>(message->pdata));
			Json::Value wrappedMessage = data->data();
			InitIce_r(wrappedMessage);		
			break;
		}
	case (uint32_t)PeerConnectionMessages::DoAddStreams:
		{
			AddStreams_r();
			break;
		}
	case (uint32_t)PeerConnectionMessages::DoCloseCapturer:
		{
			Close_r();
			break;
		}
	}
}

void WebRtcPeerConnection::InitSdp(std::shared_ptr<SdpOffer> sdp)
{
	wstring wpayload;
	sdp->GetSdpOffer(wpayload);
	string sdpPayload = StringUtil::ToString(wpayload);
	commandThr_->Send(RTC_FROM_HERE, this, static_cast<uint32_t>(PeerConnectionMessages::DoInitSdp),
		new rtc::TypedMessageData<string>(sdpPayload));
}

void WebRtcPeerConnection::InitSdp_r(const std::string& sdpPayload) 
{
	webrtc::PeerConnectionInterface::RTCConfiguration config;
	webrtc::PeerConnectionInterface::IceServer server;

	server.uri = GetPeerConnectionString();
	config.servers.push_back(server);
	MediaConstraints pcmc;
	pcmc.SetAllowDtlsSctpDataChannels();
	//pcmc.AddOptional(webrtc::MediaConstraintsInterface::kLeakyBucket, "true");
    CreatePeerConnectionFactory();
	peer_connection_ = peer_connection_factory_->CreatePeerConnection(config, &pcmc, NULL, NULL, this);

	if (!peer_connection_.get()) 
	{
		peer_connection_ = nullptr;
		throw WebRtcException("CreatePeerConnection failed");
	}

	AddStreams_r();
	ProcessSdpMessage(sdpPayload);
}

void WebRtcPeerConnection::InitIce(std::shared_ptr<vosvideo::data::WebRtcIceCandidateMsg> iceMsg)
{
	wstring wpayload;
	iceMsg->GetIceCandidate(wpayload);
	string payload = StringUtil::ToString(wpayload);

	Json::Reader reader;
	Json::Value jmessage;
	if (!reader.parse(payload, jmessage)) 
	{
		throw WebRtcException("Received unknown message: " + payload);
	}

	commandThr_->Send(RTC_FROM_HERE, this, static_cast<uint32_t>(PeerConnectionMessages::DoInitIce),
		new rtc::TypedMessageData<Json::Value>(jmessage));
}

void WebRtcPeerConnection::InitIce_r(const Json::Value& jmessage)
{
	// Check if reached end of Ice Candidates list
	if (jmessage.type() == Json::nullValue)
	{
		LOG_TRACE("Reached end of Ice Candidates list.");
		return;
	}

	std::string type;
	std::string json_object;
	rtc::GetStringFromJsonObject(jmessage, kSessionDescriptionTypeName, &type);

	if (type.empty())
	{
		int sdp_mlineindex = 0;
		string sdp_mid;
		string ice;

		if (!rtc::GetStringFromJsonObject(jmessage, kCandidateSdpMidName, &sdp_mid) ||
			!rtc::GetIntFromJsonObject(jmessage, kCandidateSdpMlineIndexName, &sdp_mlineindex) ||
			!rtc::GetStringFromJsonObject(jmessage, kCandidateSdpName, &ice))
		{
			LOG_ERROR("Can't parse received ICE candidate message.");
			return;
		}

		webrtc::SdpParseError error;
		std::unique_ptr<webrtc::IceCandidateInterface> candidate(webrtc::CreateIceCandidate(sdp_mid, sdp_mlineindex, ice, &error));
		if (!candidate.get())
		{
			LOG_ERROR("Can't parse received candidate message.");
			return;
		}

		if (!peer_connection_->AddIceCandidate(candidate.get()))
		{
			LOG_ERROR("Failed to apply the received candidate");
		}
	}
}

void WebRtcPeerConnection::OnSuccess(webrtc::SessionDescriptionInterface* desc) 
{
	peer_connection_->SetLocalDescription(DummySetSessionDescriptionObserver::Create(), desc);
	Json::StyledWriter writer;
	Json::Value jmessage;
	jmessage[kSessionDescriptionTypeName] = desc->type();
	std::string sdp;
	desc->ToString(&sdp);
	jmessage[kSessionDescriptionSdpName] = sdp;
	sdp = writer.write(jmessage);

	wstring wsdp = StringUtil::ToWstring(sdp);
	SdpAnswerMsg sdpAnswer(srvPeer_, clientPeer_, wsdp, player_->GetDeviceId());
	wstring wrespSdp = sdpAnswer.ToString();
	string respSdp = StringUtil::ToString(wrespSdp);

	queueEng_->Send(respSdp);
}

void WebRtcPeerConnection::OnIceCandidate(const webrtc::IceCandidateInterface* icecandidate) 
{
	Json::StyledWriter writer;
	Json::Value jmessage;

	jmessage[kCandidateSdpMidName] = icecandidate->sdp_mid();
	jmessage[kCandidateSdpMlineIndexName] = icecandidate->sdp_mline_index();
	std::string candidateStr;
	if (!icecandidate->ToString(&candidateStr))
	{
		LOG_ERROR("Failed to serialize ICE candidate");
		return;
	}

	jmessage[kCandidateSdpName] = candidateStr;
	candidateStr = writer.write(jmessage);

	wstring wice = StringUtil::ToWstring(candidateStr);
	IceCandidateResponseMsg iceAnswer(srvPeer_, clientPeer_, wice, player_->GetDeviceId());
	wstring wrespIce = iceAnswer.ToString();
	string respIce = StringUtil::ToString(wrespIce);

	queueEng_->Send(respIce);
}


void WebRtcPeerConnection::OnSignalingChange(webrtc::PeerConnectionInterface::SignalingState new_state)
{
	if (peer_connection_->signaling_state() == webrtc::PeerConnectionInterface::SignalingState::kClosed)
	{
		for (MediaStreamMap::iterator iter = active_streams_.begin(); iter != active_streams_.end(); ++iter)
		{
			peer_connection_->RemoveStream(iter->second);
		}

		active_streams_.clear();
		// So now it can be safely removed from list of finishing peer_connections
		isPeerConnectionFinished_ = true;
	}
}

void WebRtcPeerConnection::OnFailure(const std::string& error) 
{
	LOG(LERROR) << __FUNCTION__ << " " << error;
}

void WebRtcPeerConnection::ProcessSdpMessage(const string& message)
{
	Json::Reader reader;
	Json::Value jmessage;

	if (!reader.parse(message, jmessage)) 
	{
		throw WebRtcException("Received unknown message: " + message);
	}

	std::string type;
	std::string json_object;
	rtc::GetStringFromJsonObject(jmessage, kSessionDescriptionTypeName, &type);

	if (!type.empty()) 
	{
		std::string sdp;
		if (!rtc::GetStringFromJsonObject(jmessage, kSessionDescriptionSdpName, &sdp))
		{
			throw WebRtcException("Can't parse received session description message.");
		}

		webrtc::SdpParseError error;
		webrtc::SessionDescriptionInterface* session_description(webrtc::CreateSessionDescription(type, sdp, &error));
		if (!session_description)
		{
			throw WebRtcException("Can't parse received session description message.");
		}
		LOG_DEBUG("Received session description :" << message);
		peer_connection_->SetRemoteDescription(DummySetSessionDescriptionObserver::Create(), session_description);

		if (session_description->type() == webrtc::SessionDescriptionInterface::kOffer) 
		{
			peer_connection_->CreateAnswer(this, NULL);
		}
	}
}

void WebRtcPeerConnection::ProcessIceMessage(const string& message)
{
	throw WebRtcException("Received unknown message: " + message);
}

void WebRtcPeerConnection::AddStreams_r() 
{
	if (active_streams_.find(kStreamLabel) != active_streams_.end())
	{
		return;  // Already added.
	}

	rtc::scoped_refptr<webrtc::AudioTrackInterface> audio_track;

	// This moment we support sound only for WebCamera
	if (player_->GetCameraType() == CameraType::WEBCAM)
	{
		audio_track = peer_connection_factory_->CreateAudioTrack(kAudioLabel, peer_connection_factory_->CreateAudioSource(NULL));
	}

	videoCapturer_ = OpenVideoCaptureDevice();
	if (videoCapturer_ == nullptr)
	{
		wstring errMsg = L"Failed to create Capture Device";
		int devId = player_->GetDeviceId();
		shared_ptr<RtbcDeviceErrorOutMsg> capturerErr(new RtbcDeviceErrorOutMsg(devId, errMsg));
		string respRtbc;
		CommunicationManager::CreateWebsocketMessageString(srvPeer_, clientPeer_, capturerErr, respRtbc);
		queueEng_->Send(respRtbc);
		return;
	}

	rtc::scoped_refptr<webrtc::VideoTrackInterface> 
		video_track(peer_connection_factory_->CreateVideoTrack(kVideoLabel,peer_connection_factory_->CreateVideoSource(videoCapturer_, NULL)));

	rtc::scoped_refptr<webrtc::MediaStreamInterface> stream = peer_connection_factory_->CreateLocalMediaStream(kStreamLabel);

	// This moment we support sound only for WebCamera
	if (player_->GetCameraType() == CameraType::WEBCAM)
	{
		stream->AddTrack(audio_track);
	}

	stream->AddTrack(video_track);

	if (!peer_connection_->AddStream(stream)) 
	{
		LOG_ERROR("Adding stream to PeerConnection failed");
	}	
	active_streams_.insert(MediaStreamPair(stream->label(), stream));
}

vosvideo::camera::CameraVideoCapturer* WebRtcPeerConnection::OpenVideoCaptureDevice()
{
    int devId = player_->GetDeviceId();

	CameraVideoCapturer* capturer = new CameraVideoCapturer();
    if (!capturer->Init(devId, player_))
	{
		delete capturer;
		return nullptr;
	}

    return capturer;
}

// Called when a remote stream is added
void WebRtcPeerConnection::OnAddStream(rtc::scoped_refptr<webrtc::MediaStreamInterface> stream)
{
	LOG_TRACE(stream->label());
	stream->AddRef();
}

void WebRtcPeerConnection::OnRemoveStream(rtc::scoped_refptr<webrtc::MediaStreamInterface> stream)
{
	LOG_TRACE(stream->label());
	stream->AddRef();
}

void WebRtcPeerConnection::OnSignedIn() 
{
	LOG_TRACE(__FUNCTION__);
}

void WebRtcPeerConnection::OnDisconnected() 
{
	LOG_TRACE(__FUNCTION__);
}

void WebRtcPeerConnection::OnPeerConnected(int id, const std::string& name) 
{
	LOG_TRACE(__FUNCTION__);
}

bool WebRtcPeerConnection::IsPeerConnectionFinished()
{
    LOG_TRACE(__FUNCTION__);
    return isPeerConnectionFinished_;
}

void WebRtcPeerConnection::OnPeerDisconnected(int id) 
{
    LOG_TRACE(__FUNCTION__);
}

void WebRtcPeerConnection::OnMessageFromPeer(int peer_id, const std::string& message) 
{
    LOG_TRACE(__FUNCTION__);
}

void WebRtcPeerConnection::OnMessageSent(int err) 
{
	LOG_TRACE(__FUNCTION__);
}

void WebRtcPeerConnection::OnServerConnectionFailure() 
{
	LOG_TRACE(__FUNCTION__);
}

void WebRtcPeerConnection::Close()
{
    LOG_TRACE(__FUNCTION__);
    commandThr_->Send(RTC_FROM_HERE, this, static_cast<uint32_t>(PeerConnectionMessages::DoCloseCapturer));
}

void WebRtcPeerConnection::Close_r()
{
	if (videoCapturer_ != nullptr)
	{
		videoCapturer_->Stop();
	}
	// Initiate closing peer connection
	peer_connection_->Close();
}

void WebRtcPeerConnection::CreatePeerConnectionFactory()
{
	if (!peer_connection_factory_.get())
	{
		peer_connection_factory_ = webrtc::CreatePeerConnectionFactory();
	}

    if (!peer_connection_factory_.get())
    {
        throw WebRtcException("Failed to initialize PeerConnectionFactory");
    }
}
