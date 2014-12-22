#include "stdafx.h"
#include "IceCandidateResponseMsg.h"

using namespace std;
using namespace vosvideo::data;

IceCandidateResponseMsg::IceCandidateResponseMsg()
{
}

IceCandidateResponseMsg::IceCandidateResponseMsg(const std::wstring& fromPeer, const std::wstring& toPeer, const std::wstring& ice, int devId)
{
	jObj_[L"fp"] = web::json::value::string(fromPeer);
	jObj_[L"tp"] = web::json::value::string(toPeer);
	jObj_[L"mt"] = web::json::value::number(static_cast<int>(vosvideo::data::MsgType::IceCandidateAnswerMsg));
	jObj_[L"m"] = web::json::value::array();
	jObj_[L"m"][0] = web::json::value::parse(ice);
	web::json::value jDev;
	jDev[L"DeviceId"] = web::json::value::string(to_wstring(devId));
	web::json::value jMedia;
	jMedia[L"media_info"] = jDev;
	jObj_[L"m"][1] = jMedia;
}

IceCandidateResponseMsg::~IceCandidateResponseMsg()
{
}

void IceCandidateResponseMsg::FromJsonValue( web::json::value& obj )
{
}

wstring IceCandidateResponseMsg::ToString() const
{
	if (jObj_.is_null())
	{
		wstring msg;
		parser_->GetMessage(msg);
		return msg;
	}
	return jObj_.serialize();
}
