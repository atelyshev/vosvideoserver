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

void WebRtcIceCandidateMsg::FromJsonValue( web::json::value& obj )
{
}

void WebRtcIceCandidateMsg::GetIceCandidate(wstring& iceCandidate)
{
	web::json::value jmessage;
	ToJsonValue(jmessage);
	iceCandidate = jmessage[0].serialize();
}

void WebRtcIceCandidateMsg::GetMediaInfo(web::json::value& mi)
{
	web::json::value jmessage;
	ToJsonValue(jmessage);

	auto arr = jmessage.as_array();
	for (web::json::array::iterator iter1 = arr.begin(); iter1 != arr.end(); ++iter1)
	{
		auto lvl2 = *iter1;
		if (lvl2.has_field(U("media_info")))
		{
			mi = lvl2.at(U("media_info"));
			return;
		}
	}
}
