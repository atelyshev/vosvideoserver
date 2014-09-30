#include "stdafx.h"
#include "PeerConnectionClientBase.h"

using vosvideo::vvwebrtc::PeerConnectionClientBase;
using vosvideo::vvwebrtc::PeersMap;
using vosvideo::vvwebrtc::PeerConnectionClientObserver;

PeerConnectionClientBase::PeerConnectionClientBase() :
	callback_(NULL),
	state_(State::NOT_CONNECTED),
	my_id_(-1) 
{
}

PeerConnectionClientBase::~PeerConnectionClientBase()
{
}

int PeerConnectionClientBase::id() const 
{
	return my_id_;
}

bool PeerConnectionClientBase::is_connected() const 
{
	return my_id_ != -1;
}

const PeersMap& PeerConnectionClientBase::peers() const 
{
	return peers_;
}

void PeerConnectionClientBase::RegisterObserver(PeerConnectionClientObserver* callback) 
{
	ASSERT(!callback_);
	callback_ = callback;
}

