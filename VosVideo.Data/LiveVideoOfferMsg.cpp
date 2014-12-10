#include "stdafx.h"
#include "LiveVideoOfferMsg.h"

using namespace std;
using namespace vosvideo::data;

LiveVideoOfferMsg::LiveVideoOfferMsg()
{
}

LiveVideoOfferMsg::~LiveVideoOfferMsg()
{
}

void LiveVideoOfferMsg::FromJsonValue( web::json::value& obj )
{
}

void LiveVideoOfferMsg::GetSdpOffer(wstring& sdpOffer)
{
	web::json::value jmessage;
	ToJsonValue(jmessage);
	sdpOffer = jmessage[0].to_string();
}

void LiveVideoOfferMsg::GetMediaInfo(web::json::value& mi)
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
