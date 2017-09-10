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

void LiveVideoOfferMsg::FromJsonValue(const web::json::value& obj )
{
}

wstring LiveVideoOfferMsg::GetSdpOffer()
{
	auto jmessage = ToJsonValue();
	return jmessage[0].serialize();
}

web::json::value LiveVideoOfferMsg::GetMediaInfo()
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
