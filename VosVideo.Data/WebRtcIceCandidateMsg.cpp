#include "stdafx.h"
#include "DtoParseException.h"
#include "WebRtcIceCandidateMsg.h"

using namespace std;
using namespace vosvideo::data;

WebRtcIceCandidateMsg::WebRtcIceCandidateMsg()
{
}

WebRtcIceCandidateMsg::~WebRtcIceCandidateMsg()
{
}

void WebRtcIceCandidateMsg::FromJsonValue(const web::json::value& obj )
{
}

wstring WebRtcIceCandidateMsg::GetIceCandidate()
{
	auto jmessage = ToJsonValue();
	return jmessage[0].serialize();
}

web::json::value WebRtcIceCandidateMsg::GetMediaInfo()
{
	auto jmessage = ToJsonValue();
	auto arr = jmessage.as_array();
	web::json::value mi;

	for (const auto& a : arr)
	{
		if (a.has_field(U("media_info")))
		{
			mi = a.at(U("media_info"));
			break;
		}
	}
	return mi;
}
