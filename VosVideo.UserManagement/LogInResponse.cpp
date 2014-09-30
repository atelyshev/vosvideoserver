#include "stdafx.h"
#include "LoginResponse.h"

using namespace std;
using namespace vosvideo::usermanagement;


LogInResponse::LogInResponse():peer_(L"NoPeerId")
{

}

LogInResponse::~LogInResponse()
{
}


void LogInResponse::ToJsonValue(web::json::value& obj) const
{
}

void LogInResponse::FromJsonValue(web::json::value& obj)
{
}

std::wstring LogInResponse::ToString() const
{
	return L"";
}

void LogInResponse::SetPeer(vosvideo::communication::Peer& peer)
{
	peer_ = peer;
}

vosvideo::communication::Peer const& vosvideo::usermanagement::LogInResponse::GetPeer() const
{
	return peer_;
}

wstring vosvideo::usermanagement::operator+(wstring const& leftStr, LogInResponse const& rightResp)
{
	return rightResp + leftStr;
}

wstring LogInResponse::operator+(std::wstring const& str) const
{
	return str + peer_;
}

