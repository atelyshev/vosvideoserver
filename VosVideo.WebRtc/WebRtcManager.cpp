#include "stdafx.h"
#include <boost/format.hpp>
#include <webrtc/base/ssladapter.h>
#include <Unknwn.h>
#include "VosVideo.Communication/TypeInfoWrapper.h"
#include "VosVideo.Data/WebRtcIceCandidateMsg.h"
#include "VosVideo.Data/DeletePeerConnectionRequestMsg.h"
#include "VosVideo.Data/WebsocketConnectionClosedMsg.h"
#include "VosVideo.Data/LiveVideoOfferMsg.h"
#include "VosVideo.Data/CameraConfMsg.h"
#include "VosVideo.Data/ShutdownCameraProcessRequestMsg.h"
#include "VosVideo.CameraPlayer/CameraPlayerBase.h"
#include "VosVideo.Camera/CameraPlayerFactory.h"
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
using namespace vosvideo::cameraplayer;


using boost::format;
using boost::wformat;

WebRtcManager::WebRtcManager(
    std::shared_ptr<vosvideo::communication::PubSubService> pubsubService, 
	std::shared_ptr<vosvideo::communication::InterprocessQueueEngine> queueEng) 
    : pubSubService_(pubsubService), queueEng_(queueEng), inShutdown_(false)
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

    rtc::AutoThread auto_thread;
    physicalSocketServer_ = new rtc::PhysicalSocketServer();
    mainThread_ = new rtc::Thread(physicalSocketServer_);
    rtc::InitializeSSL();
    mainThread_->Start();
}

WebRtcManager::~WebRtcManager()
{
}

void WebRtcManager::OnMessageReceived(std::shared_ptr<ReceivedData> receivedMessage)
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
			player_ = CameraPlayerFactory::CreateCameraPlayer();
			auto hr = player_->OpenURL(*cameraConf.get());		

			if (hr != 0)
			{
				LOG_CRITICAL("Failed to create camera");
			}
			isaliveTimer_->start();
		}
		return;
	}

	wstring clientPeerKey = str(wformat(L"%1%-%2%") % clientPeer % activeDeviseId_);
	rtc::scoped_refptr<WebRtcPeerConnection> conn;
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
        conn = new rtc::RefCountedObject<WebRtcPeerConnection>(clientPeer, srvPeer, player_, queueEng_);
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
		auto jsonMsg = receivedMessage->ToJsonValue();
		auto fromPeer = jsonMsg.at(U("p")).as_string();
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

		//Check if it derives from IUnknown
	IUnknown* iUnknownPlayer = dynamic_cast<IUnknown*>(player_);
	if(iUnknownPlayer)
		iUnknownPlayer->Release();

	queueEng_->StopReceive();
}

void WebRtcManager::DeleteAllPeerConnections()
{
	LOG_TRACE("Query for deletion all peer connections");	

	for (const auto& pc : peer_connections_)
	{
		// command close active streams and remove from collection after
		pc.second->Close();
		LOG_TRACE("Query for deletion peer connection with key:" << StringUtil::ToString(pc.first));
		finishing_peer_connections_.push_back(pc.second);
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
		rtc::scoped_refptr<WebRtcPeerConnection> ptr = (*iter);
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
	if (!rtc::InitializeSSL() || !rtc::InitializeSSLThread())
	{
		throw WebRtcException("Failed to initialize SSL");
	}

	peer_connection_factory_  = webrtc::CreatePeerConnectionFactory();

	if (!peer_connection_factory_.get()) 
	{
		throw WebRtcException("Failed to initialize PeerConnectionFactory");
	}
}
