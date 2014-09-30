#include "stdafx.h"
#include "Peer.h"

using vosvideo::communication::Peer;

Peer::Peer(std::wstring peerId) : peerId_(peerId)
{
}


Peer::~Peer(void)
{
}

std::wstring const& Peer::GetPeerId() const
{
	return peerId_;
}


std::wstring vosvideo::communication::operator+(std::wstring const& leftStr, Peer const& peer){
	return peer + leftStr;
}

std::wstring vosvideo::communication::Peer::operator+(std::wstring const& str) const
{
	return str + L" PeerId : " +  peerId_;
}

