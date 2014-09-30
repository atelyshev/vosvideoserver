#include "stdafx.h"
#include "defaults.h"
#include "WsPeerConnectionClient.h"
#include "PeerConnectionClientManager.h"

using vosvideo::vvwebrtc::PeerConnectionClientManager;

PeerConnectionClientManager::PeerConnectionClientManager() : 
	client_(new WsPeerConnectionClient())
{	
	client_->Connect("192.168.1.22", 81, GetPeerName());
}


PeerConnectionClientManager::~PeerConnectionClientManager()
{
}
