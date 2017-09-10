#include "stdafx.h"
#include "DeletePeerConnectionRequestMsg.h"

using namespace std;
using namespace vosvideo::data;

DeletePeerConnectionRequestMsg::DeletePeerConnectionRequestMsg()
{
}

DeletePeerConnectionRequestMsg::~DeletePeerConnectionRequestMsg()
{
}

web::json::value DeletePeerConnectionRequestMsg::ToJsonValue() const
{
	web::json::value jObj;
	return jObj;
}

void DeletePeerConnectionRequestMsg::FromJsonValue(const web::json::value& obj )
{
}

