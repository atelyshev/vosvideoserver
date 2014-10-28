#include "stdafx.h"
#include <boost/format.hpp>
#include <talk/base/ssladapter.h>
#include "VosVideo.Communication/TypeInfoWrapper.h"
#include "VosVideo.Data/WebRtcIceCandidateMsg.h"
#include "VosVideo.Data/DeletePeerConnectionRequestMsg.h"
#include "VosVideo.Data/WebsocketConnectionClosedMsg.h"
#include "VosVideo.Data/LiveVideoOfferMsg.h"
#include "VosVideo.Data/CameraConfMsg.h"
#include "VosVideo.Data/ShutdownCameraProcessRequestMsg.h"
#include "VosVideo.Camera/CameraPlayer.h"

#include "VosVideo.Camera/CameraException.h"
#include "WebRtcManager.h"
#include "WebRtcException.h"

using namespace std;
using namespace util;
using namespace concurrency;
using namespace vosvideo::vvwebrtc;
using namespace vosvideo::communication;
using namespace vosvideo::data;
using namespace vosvideo::camera;

using boost::format;
using boost::wformat;

WebRtcManager::WebRtcManager(std::shared_ptr<vosvideo::communication::PubSubService> pubsubService, 
									   std::shared_ptr<vosvideo::communication::InterprocessQueueEngine> queueEng) :
pubSubService_(pubsubService), queueEng_(queueEng), inShutdown_(false)
{
	vector<TypeInfoWrapper> interestedTypes;

	TypeInfoWrapper typeInfo = typeid(CameraConfMsg);	
	interestedTypes.push_back(typeInfo);

	typeInfo = typeid(WebRtcIceCandidateMsg);	
	interestedTypes.push_back(typeInfo);

	typeInfo = typeid(LiveVideoOfferMsg);
	interestedTypes.push_back(typeInfo);

	typeInfo = typeid(DeletePeerConnectionRequestMsg);
	interestedTypes.push_back(typeInfo);

	typeInfo = typeid(WebsocketConnectionClosedMsg);
	interestedTypes.push_back(typeInfo);

	typeInfo = typeid(ShutdownCameraProcessRequestMsg);
	interestedTypes.push_back(typeInfo);

	pubSubService_->Subscribe(interestedTypes, *this);

	// Prepare timer
	auto callback = new call<WebRtcManager*>([this](WebRtcManager*)
	{
		if (this->player_->GetState() == PlayerState::Stopped)
		{
			Shutdown();
		}
	});
	isaliveTimer_ = new Concurrency::timer<WebRtcManager*>(isaliveTimeout_, 0, callback, true);

	// First check if peer conn can be created
	CreatePeerConnectionFactory();
	physicalSocketServer_ = new talk_base::PhysicalSocketServer();
	mainThread_ = new talk_base::AutoThread(physicalSocketServer_);
	mainThread_->Start();
}

WebRtcManager::~WebRtcManager()
{
}

void WebRtcManager::OnMessageReceived(const shared_ptr<ReceivedData> receivedMessage)
{	
	// Dont accept any connections if in Shutdown
	if (inShutdown_)
	{
		return;
	}

	wstring srvPeer;
	wstring clientPeer;
	receivedMessage->GetFromPeer(clientPeer);
	receivedMessage->GetToPeer(srvPeer);

	// We cant make sure that SDP comes first, just create entry and then init is 
	// once something comes (SDP or ICE). Generally speaking SDP is not interesting for us
	lock_guard<std::mutex> lock(mutex_);

	// Time to stop camera and close all connections
	if(dynamic_pointer_cast<ShutdownCameraProcessRequestMsg>(receivedMessage))
	{
		Shutdown();
		return;
	}

	if(dynamic_pointer_cast<CameraConfMsg>(receivedMessage))
	{
		shared_ptr<CameraConfMsg> cameraDto = dynamic_pointer_cast<CameraConfMsg>(receivedMessage);
		cameraDto->GetCameraIds(activeDeviseId_, deviceName_);
		if(dynamic_pointer_cast<CameraConfMsg>(receivedMessage))
		{
			shared_ptr<CameraConfMsg> cameraConf = dynamic_pointer_cast<CameraConfMsg>(receivedMessage);			
			// COM pointer can not use shared_ptr
			player_ = new CameraPlayer();
			HRESULT hr = player_->OpenURL(*cameraConf.get());		

			if (hr != S_OK)
			{
				LOG_CRITICAL("Failed to create camera");
			}
			isaliveTimer_->start();
		}
		return;
	}

	wstring clientPeerKey = str(wformat(L"%1%-%2%") % clientPeer % activeDeviseId_);
	talk_base::scoped_refptr<WebRtcPeerConnection> conn;
	WebRtcPeerConnectionMap::iterator iter;

	if ((iter = peer_connections_.find(clientPeerKey)) != peer_connections_.end())
	{
		LOG_TRACE("Found peer connection with key:" << StringUtil::ToString(clientPeerKey));
		conn = iter->second;
	}

	if(dynamic_pointer_cast<LiveVideoOfferMsg>(receivedMessage))
	{
		shared_ptr<LiveVideoOfferMsg> liveVideoDto = dynamic_pointer_cast<LiveVideoOfferMsg>(receivedMessage);

		LOG_TRACE("Create new peer connection with key:" << StringUtil::ToString(clientPeerKey));
		conn = new talk_base::RefCountedObject<WebRtcPeerConnection>(clientPeer, srvPeer, player_, peer_connection_factory_, queueEng_);
		conn->SetCurrentThread(mainThread_);

		// Check if peer with camera id doesnt exists. 
		// If exists current should be moved to deleted collection
		if (peer_connections_.find(clientPeerKey) != peer_connections_.end())
		{
			DeletePeerConnection(clientPeerKey);
		}
		// Add SDP
		peer_connections_.insert(make_pair(clientPeerKey, conn));
		// Process connection
		shared_ptr<SdpOffer> sdpOffer = dynamic_pointer_cast<SdpOffer>(receivedMessage);
		conn->InitSdp(sdpOffer);
	}
	else if (dynamic_pointer_cast<WebRtcIceCandidateMsg>(receivedMessage))
	{
		WebRtcDeferredIceMap::iterator iter;

		if (conn == nullptr)
		{
			LOG_DEBUG("Ice candidate doesnt have corresponding SDP. Add to deferred ICE container.");
			if ((iter = deferredIce_.find(clientPeerKey)) == deferredIce_.end())
			{
				vector<shared_ptr<ReceivedData> > data;
				data.push_back(receivedMessage);
				deferredIce_.insert(make_pair(clientPeerKey, data));
			}
			else
			{
				iter->second.push_back(receivedMessage);
			}
		}
		else
		{
			conn->InitIce(dynamic_pointer_cast<WebRtcIceCandidateMsg>(receivedMessage));
			if ((iter = deferredIce_.find(clientPeerKey)) != deferredIce_.end())
			{
				for (int i = 0; i < iter->second.size(); ++i)
				{
					shared_ptr<ReceivedData> savedMessage = iter->second[i];
					conn->InitIce(dynamic_pointer_cast<WebRtcIceCandidateMsg>(savedMessage));
				}
				deferredIce_.erase(iter);
			}
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

void WebRtcManager::Shutdown()
{
	inShutdown_ = true;
	isaliveTimer_->stop();
	DeleteAllPeerConnections();

	// Wait no more then 10 seconds
	for (int i = 0; i < 10; i++)
	{
		boost::this_thread::sleep(boost::posix_time::milliseconds(1000));
		if (CheckFinishingPeerConnections() == 0)
		{
			break;
		}
	}

	player_->Stop();
	player_->Shutdown();
	player_->Release();
	queueEng_->StopReceive();
}

void WebRtcManager::DeleteAllPeerConnections()
{
	LOG_TRACE("Query for deletion all peer connections");	

	for (auto iter = peer_connections_.begin(); iter != peer_connections_.end(); ++iter)
	{
		// command close active streams and remove from collection after
		iter->second->Close();
		LOG_TRACE("Query for deletion peer connection with key:" << StringUtil::ToString(iter->first));
		finishing_peer_connections_.push_back(iter->second);
	}

	peer_connections_.clear();
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
			LOG_TRACE("Query for deletion peer connection with key:" << StringUtil::ToString(iter->first));
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
