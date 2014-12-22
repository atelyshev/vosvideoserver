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
	sdpOffer = jmessage[0].serialize();
}

void LiveVideoOfferMsg::GetMediaInfo(web::json::value& mi)
{
	web::json::value jmessage;
	ToJsonValue(jmessage);

	auto arr = jmessage.as_array();
	for (web::json::array::iterator iter1 = arr.begin(); iter1 != arr.end(); iter1++)
	{
		auto lvl2 = *iter1;
		if (lvl2.has_field(U("media_info")))
		{
			mi = lvl2.at(U("media_info"));
			return;
		}
	}
}
