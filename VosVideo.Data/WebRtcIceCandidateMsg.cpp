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
	iceCandidate = jmessage[0].to_string();
}

void WebRtcIceCandidateMsg::GetMediaInfo(web::json::value& mi)
{
	web::json::value jmessage;
	ToJsonValue(jmessage);

	for (web::json::value::iterator iter1 = jmessage.begin(); iter1 != jmessage.end(); iter1++)
	{
		auto lvl2 = *iter1;
		for (web::json::value::iterator iter2 = lvl2.second.begin(); iter2 != lvl2.second.end(); iter2++)
		{
			if (iter2->first.as_string() == U("media_info"))
			{
				mi = iter2->second;
				break;
			}
		}
	}
}
