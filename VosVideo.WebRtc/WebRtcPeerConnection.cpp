#include "stdafx.h"
#include <talk/app/webrtc/mediaconstraintsinterface.h>
#include <talk/media/devices/devicemanager.h>
#include <vosvideocommon/StringUtil.h>

#include "VosVideo.Camera/CameraVideoCapturer.h"
#include "VosVideo.Data/CameraConfMsg.h"
#include "VosVideo.Data/SdpAnswerMsg.h"
#include "VosVideo.Data/IceCandidateResponseMsg.h"
#include "VosVideo.Communication/CommunicationManager.h"
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

// Names used for a IceCandidate JSON object.
const char kCandidateSdpMidName[] = "sdpMid";
const char kCandidateSdpMlineIndexName[] = "sdpMLineIndex";
const char kCandidateSdpName[] = "candidate";

// Names used for a SessionDescription JSON object.
const char kSessionDescriptionTypeName[] = "type";
const char kSessionDescriptionSdpName[] = "sdp";


WebRtcPeerConnection::WebRtcPeerConnection(wstring clientPeer,
										   wstring srvPeer,
										   vosvideo::camera::CameraPlayer* player,
										   talk_base::scoped_refptr<webrtc::PeerConnectionFactoryInterface> peer_connection_factory, 
										   std::shared_ptr<vosvideo::communication::InterprocessQueueEngine> queueEng): 
	clientPeer_(clientPeer),
	srvPeer_(srvPeer),
	player_(player),
	peer_connection_factory_(peer_connection_factory),
	queueEng_(queueEng),
	isPeerConnectionFinished_(false),
	isShutdownOnClose_(false)
{
}

WebRtcPeerConnection::~WebRtcPeerConnection()
{
	peer_connection_ = nullptr;
}

void WebRtcPeerConnection::SetCurrentThread(talk_base::Thread* commandThr)
{
	commandThr_ = commandThr;
}

void WebRtcPeerConnection::SetDeviceManager(std::shared_ptr<vosvideo::camera::CameraDeviceManager> deviceManager, int devId, bool isShutdownOnClose)
{
	deviceManager_ = deviceManager;
	isShutdownOnClose_ = isShutdownOnClose;
}

void WebRtcPeerConnection::OnMessage(talk_base::Message* message) 
{
	switch (message->message_id) 
	{
	case PeerConnectionMessages::DoInitSdp:
		{
			shared_ptr<talk_base::TypedMessageData<string>> data(static_cast<talk_base::TypedMessageData<string>*>(message->pdata));
			string wrappedMessage = data->data();
			InitSdp_r(wrappedMessage);		
			break;
		}
	case PeerConnectionMessages::DoInitIce:
		{
			shared_ptr<talk_base::TypedMessageData<Json::Value>> 
				data(static_cast<talk_base::TypedMessageData<Json::Value>*>(message->pdata));
			Json::Value wrappedMessage = data->data();
			InitIce_r(wrappedMessage);		
			break;
		}
	case PeerConnectionMessages::DoAddStreams:
		{
			AddStreams_r();
			break;
		}
	case PeerConnectionMessages::DoOnSuccess:
		{
			shared_ptr<TypedMessagePtr<webrtc::SessionDescriptionInterface>> 
				data(static_cast<TypedMessagePtr<webrtc::SessionDescriptionInterface>*>(message->pdata));
			auto wrappedMessage = dynamic_cast<webrtc::SessionDescriptionInterface*>(data->data());
			OnSuccess_r(wrappedMessage);
			break;
		}
	case PeerConnectionMessages::DoOnIceCandidate:
		{
			shared_ptr<TypedMessagePtr<webrtc::IceCandidateInterface>> 
				data(static_cast<TypedMessagePtr<webrtc::IceCandidateInterface>*>(message->pdata));
			auto wrappedMessage = dynamic_cast<webrtc::IceCandidateInterface*>(data->data());
			OnIceCandidate_r(wrappedMessage);
			break;
		}
	case PeerConnectionMessages::DoOnSignalChange:
		{
			shared_ptr<talk_base::TypedMessageData<webrtc::PeerConnectionInterface::SignalingState>> 
				data(static_cast<talk_base::TypedMessageData<webrtc::PeerConnectionInterface::SignalingState>*>(message->pdata));
			webrtc::PeerConnectionInterface::SignalingState wrappedMessage = data->data();
			OnSignalingChange_r(wrappedMessage);
			break;
		}
	case PeerConnectionMessages::DoCloseCapturer:
		{
			Close_r();
			break;
		}
	}
}

void WebRtcPeerConnection::InitSdp(shared_ptr<SdpOffer> sdp)
{
	wstring wpayload;
	sdp->GetSdpOffer(wpayload);
	string sdpPayload;
	StringUtil::ToString(wpayload, sdpPayload);
	commandThr_->Post(this, static_cast<uint32>(PeerConnectionMessages::DoInitSdp), 
		new talk_base::TypedMessageData<string>(sdpPayload));
}

void WebRtcPeerConnection::InitSdp_r(const string& sdpPayload) 
{
	webrtc::PeerConnectionInterface::IceServers servers;
	webrtc::PeerConnectionInterface::IceServer server;

	server.uri = GetPeerConnectionString();
	servers.push_back(server);
	MediaConstraints pcmc;
	pcmc.SetAllowDtlsSctpDataChannels();
	peer_connection_ = peer_connection_factory_->CreatePeerConnection(servers, &pcmc, nullptr, this);

	if (!peer_connection_.get()) 
	{
		peer_connection_ = nullptr;
		throw WebRtcException("CreatePeerConnection failed");
	}

	AddStreams_r();

	ProcessSdpMessage(sdpPayload);
}

void WebRtcPeerConnection::InitIce(const std::shared_ptr<vosvideo::data::WebRtcIceCandidateMsg> iceMsg)
{
	wstring wpayload;
	iceMsg->GetIceCandidate(wpayload);
	string payload;
	StringUtil::ToString(wpayload, payload);

	Json::Reader reader;
	Json::Value jmessage;
	if (!reader.parse(payload, jmessage)) 
	{
		throw WebRtcException("Received unknown message: " + payload);
	}

	commandThr_->Post(this, static_cast<uint32>(PeerConnectionMessages::DoInitIce), 
		new talk_base::TypedMessageData<Json::Value>(jmessage));
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
	GetStringFromJsonObject(jmessage, kSessionDescriptionTypeName, &type);

	if (type.empty())
	{
		int sdp_mlineindex = 0;
		string sdp_mid;
		string ice;

		if (!GetStringFromJsonObject(jmessage, kCandidateSdpMidName, &sdp_mid) ||
			!GetIntFromJsonObject(jmessage, kCandidateSdpMlineIndexName, &sdp_mlineindex) ||
			!GetStringFromJsonObject(jmessage, kCandidateSdpName, &ice)) 
		{
			LOG_ERROR("Can't parse received ICE candidate message.");
			return;
		}

		talk_base::scoped_ptr<webrtc::IceCandidateInterface> candidate(webrtc::CreateIceCandidate(sdp_mid, sdp_mlineindex, ice));
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
	TypedMessagePtr<webrtc::SessionDescriptionInterface>* wrapper = new TypedMessagePtr<webrtc::SessionDescriptionInterface>(desc);
	commandThr_->Post(this, static_cast<uint32>(PeerConnectionMessages::DoOnSuccess), wrapper);
}

void WebRtcPeerConnection::OnSuccess_r(webrtc::SessionDescriptionInterface* desc)
{
	peer_connection_->SetLocalDescription(DummySetSessionDescriptionObserver::Create(), desc);
	Json::StyledWriter writer;
	Json::Value jmessage;
	jmessage[kSessionDescriptionTypeName] = desc->type();
	std::string sdp;
	desc->ToString(&sdp);
	jmessage[kSessionDescriptionSdpName] = sdp;
	sdp = writer.write(jmessage);

	wstring wsdp;
	StringUtil::ToWstring(sdp, wsdp);
	SdpAnswerMsg sdpAnswer(srvPeer_, clientPeer_, wsdp, player_->GetDeviceId());
	wstring wrespSdp = sdpAnswer.ToString();
	string respSdp;
	StringUtil::ToString(wrespSdp , respSdp);

	queueEng_->Send(respSdp);
}

void WebRtcPeerConnection::OnIceCandidate(const webrtc::IceCandidateInterface* candidate) 
{
	TypedMessagePtr<const webrtc::IceCandidateInterface>* wrapper = new TypedMessagePtr<const webrtc::IceCandidateInterface>(candidate);
	// Cant use Post, caller will free memory immediately and application will crash
	commandThr_->Send(this, static_cast<uint32>(PeerConnectionMessages::DoOnIceCandidate), wrapper);
}

void WebRtcPeerConnection::OnIceCandidate_r(webrtc::IceCandidateInterface* icecandidate)
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

	wstring wice;
	StringUtil::ToWstring(candidateStr, wice);
	IceCandidateResponseMsg iceAnswer(srvPeer_, clientPeer_, wice, player_->GetDeviceId());
	wstring wrespIce = iceAnswer.ToString();
	string respIce;
	StringUtil::ToString(wrespIce , respIce);

	queueEng_->Send(respIce);
}

void WebRtcPeerConnection::OnSignalingChange(webrtc::PeerConnectionInterface::SignalingState new_state)
{
	commandThr_->Post(this, static_cast<uint32>(PeerConnectionMessages::DoOnSignalChange), 
		new talk_base::TypedMessageData<webrtc::PeerConnectionInterface::SignalingState>(new_state));
}

void WebRtcPeerConnection::OnSignalingChange_r(webrtc::PeerConnectionInterface::SignalingState new_state)
{
	if(peer_connection_->signaling_state() == webrtc::PeerConnectionInterface::SignalingState::kClosed)
	{			
		for(MediaStreamMap::iterator iter = active_streams_.begin(); iter != active_streams_.end(); ++iter)
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
	LOG(LERROR) << error;
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
	GetStringFromJsonObject(jmessage, kSessionDescriptionTypeName, &type);

	if (!type.empty()) 
	{
		std::string sdp;
		if (!GetStringFromJsonObject(jmessage, kSessionDescriptionSdpName, &sdp)) 
		{
			throw WebRtcException("Can't parse received session description message.");
		}
		webrtc::SessionDescriptionInterface* session_description(webrtc::CreateSessionDescription(type, sdp));
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

//	talk_base::scoped_refptr<webrtc::AudioTrackInterface> audio_track(
//		peer_connection_factory_->CreateAudioTrack(kAudioLabel, peer_connection_factory_->CreateAudioSource(NULL)));

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

	talk_base::scoped_refptr<webrtc::VideoTrackInterface> 
		video_track(peer_connection_factory_->CreateVideoTrack(kVideoLabel,peer_connection_factory_->CreateVideoSource(videoCapturer_, NULL)));

	talk_base::scoped_refptr<webrtc::MediaStreamInterface> stream = peer_connection_factory_->CreateLocalMediaStream(kStreamLabel);

//	stream->AddTrack(audio_track);
	stream->AddTrack(video_track);

	if (!peer_connection_->AddStream(stream, NULL)) 
	{
		LOG_ERROR("Adding stream to PeerConnection failed");
	}	
	active_streams_.insert(MediaStreamPair(stream->label(), stream));
}

cricket::VideoCapturer* WebRtcPeerConnection::OpenVideoCaptureDevice()
{
	int devId = player_->GetDeviceId();
	cricket::Device device = cricket::Device(to_string(devId), devId);
//	if (!deviceManager_->GetVideoCaptureDevice(devId_, dev))
//	{
//		LOG_ERROR("Failed to create Capture Device for camera id: " << devId_);
//		return NULL;
//	}

	CameraVideoCapturer* capturer = new CameraVideoCapturer();
	if (!capturer->Init(devId, player_)) 
	{
		delete capturer;
		return NULL;
	}
	return capturer;

	return deviceManager_->CreateVideoCapturer(device);
}

// Called when a remote stream is added
void WebRtcPeerConnection::OnAddStream(webrtc::MediaStreamInterface* stream) 
{
	LOG_TRACE(stream->label());

	stream->AddRef();
}

void WebRtcPeerConnection::OnRemoveStream(webrtc::MediaStreamInterface* stream) 
{
	LOG_TRACE(stream->label());
	stream->AddRef();
}

void WebRtcPeerConnection::OnError() 
{
	LOG_ERROR(__FUNCTION__);
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
	return isPeerConnectionFinished_;
}

void WebRtcPeerConnection::OnPeerDisconnected(int id) 
{
}

void WebRtcPeerConnection::OnMessageFromPeer(int peer_id, const std::string& message) 
{
}

void WebRtcPeerConnection::OnMessageSent(int err) 
{
	LOG_TRACE(__FUNCTION__);
}

void WebRtcPeerConnection::OnServerConnectionFailure() 
{
	LOG_TRACE(__FUNCTION__);
}

// Don use it will be removed
void WebRtcPeerConnection::OnStateChange(webrtc::PeerConnectionObserver::StateType state_changed)
{
	LOG_TRACE(__FUNCTION__);
}

void WebRtcPeerConnection::Close()
{
	commandThr_->Post(this, static_cast<uint32>(PeerConnectionMessages::DoCloseCapturer));
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

