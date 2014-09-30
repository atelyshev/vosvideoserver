#include "stdafx.h"
#include <boost/format.hpp>
#include <vosvideocommon/SeverityLoggerMacros.h>
#include <vosvideocommon/StringUtil.h>
#include <talk/base/ssladapter.h>

#include "VosVideo.Communication/TypeInfoWrapper.h"
#include "VosVideo.Data/WebRtcIceCandidateMsg.h"
#include "VosVideo.Data/DeletePeerConnectionRequestMsg.h"
#include "VosVideo.Data/WebsocketConnectionClosedMsg.h"
#include "VosVideo.Data/CameraConfMsg.h"
#include "VosVideo.Camera/CameraPlayer.h"
#include "VosVideo.Camera/CameraException.h"
#include "WebRtcManager.h"
#include "WebRtcException.h"

using namespace std;
using namespace util;
using namespace vosvideo::vvwebrtc;
using namespace vosvideo::communication;
using namespace vosvideo::data;
using namespace vosvideo::camera;

using boost::format;
using boost::wformat;

WebRtcManager::WebRtcManager(std::shared_ptr<vosvideo::communication::CommunicationManager> communicationManager, 
							 std::shared_ptr<vosvideo::camera::CameraDeviceManager> deviceManager,
							 shared_ptr<vosvideo::communication::PubSubService> pubsubService) :
    communicationManager_(communicationManager), 
	pubSubService_(pubsubService), 
	deviceManager_(deviceManager)
{
	// First check if peer conn can be created
	CreatePeerConnectionFactory();
	vector<TypeInfoWrapper> interestedTypes;

	TypeInfoWrapper typeInfo = typeid(WebRtcIceCandidateMsg);	
	interestedTypes.push_back(typeInfo);

	typeInfo = typeid(LiveVideoOfferMsg);
	interestedTypes.push_back(typeInfo);

	typeInfo = typeid(DeletePeerConnectionRequestMsg);
	interestedTypes.push_back(typeInfo);

	typeInfo = typeid(WebsocketConnectionClosedMsg);
	interestedTypes.push_back(typeInfo);

	pubSubService_->Subscribe(interestedTypes, *this);

	physicalSocketServer_ = new talk_base::PhysicalSocketServer();
	mainThread_ = new talk_base::AutoThread(physicalSocketServer_);
	mainThread_->Start();
}

WebRtcManager::~WebRtcManager()
{
}

void WebRtcManager::Shutdown()
{
	// Signal all opened peer connections
	WebRtcPeerConnectionMap::iterator iter;
	for (iter = peer_connections_.begin(); iter != peer_connections_.end(); ++iter)
	{
		// command close active streams and remove from collection after
		iter->second->Close();
		finishing_peer_connections_.push_back(iter->second);
	}
	peer_connections_.clear();

	// Quite inefficient way but it is only happens on exit
	LOG_TRACE("Closing all peer connections");
	int count = 10;
	while (CheckFinishingPeerConnections())
	{
		LOG_TRACE("Not all peer connection in FINISHED state. Waiting for 1 sec.");
		Sleep(1000);
		if (--count)
			break;
	}

	// Now we can stop the rest SAFELY!!!
	mainThread_->Stop();
	peer_connection_factory_ = nullptr;
}


void WebRtcManager::OnMessageReceived(const shared_ptr<ReceivedData> receivedMessage)
{	
	wstring srvPeer;
	wstring clientPeer;
	receivedMessage->GetFromPeer(clientPeer);
	receivedMessage->GetToPeer(srvPeer);
	WebRtcPeerConnectionMap::iterator iter;

	// We cant make sure that SDP comes first, just create entry and then init is 
	// once something comes (SDP or ICE). Generally speaking SDP is not interesting for us
	lock_guard<std::mutex> lock(mutex_);
	talk_base::scoped_refptr<WebRtcPeerConnection> conn;

	// Get peer_connection all the time it asked BUT!!! create only when SDP came
	int devId = -1;
	wstring clientPeerKey;
	CreateClientPeerKey(receivedMessage, clientPeerKey, devId);

	if ((iter = peer_connections_.find(clientPeerKey)) != peer_connections_.end())
	{
		LOG_TRACE("Found peer connection with key:" << clientPeerKey);
		conn = iter->second;
	}

	if(dynamic_pointer_cast<LiveVideoOfferMsg>(receivedMessage))
	{
		if (deviceManager_->IsVideoCaptureDeviceExists(devId))
		{
			shared_ptr<SendData> lastErr;
			if (!deviceManager_->IsVideoCaptureDeviceReady(devId, lastErr))
			{
				if (lastErr)
				{
					string respRtbc;
					CommunicationManager::CreateWebsocketMessageString(srvPeer, clientPeer, lastErr, respRtbc);
					communicationManager_->WebsocketSend(respRtbc);
					wstring wlastErr;
					lastErr->GetOutMsgText(wlastErr);
					LOG_TRACE("Will open empty peer connection, reason: " << wlastErr);
				}
				return;
			}

			LOG_TRACE("Create new peer connection with key:" << clientPeerKey);
			conn = new talk_base::RefCountedObject<WebRtcPeerConnection>(clientPeer, srvPeer, nullptr, peer_connection_factory_, nullptr);
			conn->SetCurrentThread(mainThread_);
			conn->SetDeviceManager(deviceManager_, devId, true);
			// Check if peer with camera id doesnt exists. 
			// If exists current should be moved to deleted collection
			if (peer_connections_.find(clientPeerKey) != peer_connections_.end())
			{
				DeletePeerConnection(clientPeerKey);
			}
			peer_connections_.insert(make_pair(clientPeerKey, conn));
			// Process connection
			shared_ptr<SdpOffer> sdpOffer = dynamic_pointer_cast<SdpOffer>(receivedMessage);
			conn->InitSdp(sdpOffer);
		}
		else
		{
			LOG_ERROR("Camera failed to start and was not added as part of pool of capturing devices.");
		}
	}
	else if (dynamic_pointer_cast<WebRtcIceCandidateMsg>(receivedMessage))
	{
		if (conn != nullptr)
		{
			conn->InitIce(dynamic_pointer_cast<WebRtcIceCandidateMsg>(receivedMessage));
		}
		else
		{
			LOG_ERROR("Peer Connection object is NULL. ICE candidate has no matching SDP. Ignoring ICE candidate");
		}
	}
	else if(dynamic_pointer_cast<WebsocketConnectionClosedMsg>(receivedMessage))
	{
		web::json::value jsonMsg;
		receivedMessage->ToJsonValue(jsonMsg);
		web::json::value::iterator jsonPeer = jsonMsg.begin();
		wstring fromPeer = jsonPeer->second.as_string();
		DeletePeerConnection(fromPeer);
	}
	else if(dynamic_pointer_cast<DeletePeerConnectionRequestMsg>(receivedMessage))
	{
		wstring fromPeer;
		receivedMessage->GetFromPeer(fromPeer);
		DeletePeerConnection(fromPeer);
	}
}

void WebRtcManager::CreateClientPeerKey(const shared_ptr<ReceivedData> receivedMessage, wstring& clientPeerKey, int& devId)
{
	shared_ptr<MediaInfo> mediaInfo = dynamic_pointer_cast<MediaInfo>(receivedMessage);
	if (!mediaInfo)
		return;

	wstring clientPeer;
	receivedMessage->GetFromPeer(clientPeer);

	web::json::value jCamConf;
	mediaInfo->GetMediaInfo(jCamConf);

	CameraDeviceManager::GetDeviceIdFromJson(devId, jCamConf);

	clientPeerKey = str(wformat(L"%1%-%2%") % clientPeer % devId);
}

void WebRtcManager::CreatePeerConnectionFactory()
{
	if (!talk_base::InitializeSSL() || !talk_base::InitializeSSLThread())
	{
		throw WebRtcException("Failed to initialize SSL");
	}

	peer_connection_factory_  = webrtc::CreatePeerConnectionFactory();

	if (!peer_connection_factory_.get()) 
	{
		throw WebRtcException("Failed to initialize PeerConnectionFactory");
	}
}

void WebRtcManager::DeletePeerConnection(const wstring& fromPeer)
{
	LOG_TRACE("Query for deletion peer connections from peer id:" << fromPeer);
	WebRtcPeerConnectionMap::iterator iter = peer_connections_.lower_bound(fromPeer);

	while (iter != peer_connections_.end())
	{
		if (iter->first.compare(0, fromPeer.length(), fromPeer) == 0)
		{
			// command close active streams and remove from collection after
			iter->second->Close();
			LOG_TRACE("Query for deletion peer connection with key:" << iter->second);
			finishing_peer_connections_.push_back(iter->second);
			iter = peer_connections_.erase(iter);
		}
		else
		{
			++iter;
		}
	}

	// Periodically check for garbage
	CheckFinishingPeerConnections();
}

int WebRtcManager::CheckFinishingPeerConnections()
{
	WebRtcPeerConnectionVector::iterator iter = finishing_peer_connections_.begin();

	while (iter != finishing_peer_connections_.end())
	{
		talk_base::scoped_refptr<WebRtcPeerConnection> ptr = (*iter);
		if (ptr->IsPeerConnectionFinished() == true)
		{
			// Just remove from vector, it will die automatically
			iter = finishing_peer_connections_.erase(iter);
		}else
		{
			++iter;
		}
	}

	return finishing_peer_connections_.size();
}
