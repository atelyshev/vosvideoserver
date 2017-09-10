#include "stdafx.h"
#include "SdpAnswerMsg.h"

using namespace std;
using namespace vosvideo::data;

SdpAnswerMsg::SdpAnswerMsg()
{
}

SdpAnswerMsg::SdpAnswerMsg(const std::wstring& fromPeer, const std::wstring& toPeer, const std::wstring& sdp, int devId)
{
	jObj_[L"fp"] = web::json::value::string(fromPeer);
	jObj_[L"tp"] = web::json::value::string(toPeer);
	jObj_[L"mt"] = web::json::value::number(static_cast<int>(vosvideo::data::MsgType::SdpAnswerMsg));
	jObj_[L"m"] = web::json::value::array();
	jObj_[L"m"][0] = web::json::value::parse(sdp);
	web::json::value jDev;
	jDev[L"DeviceId"] = web::json::value::string(to_wstring(devId));
	web::json::value jMedia;
	jMedia[L"media_info"] = jDev;
	jObj_[L"m"][1] = jMedia;
}

SdpAnswerMsg::~SdpAnswerMsg()
{
}

void SdpAnswerMsg::FromJsonValue(const web::json::value& obj )
{
}


wstring SdpAnswerMsg::ToString() const
{
	if (jObj_.is_null())
	{
		return parser_->GetMessage();
	}
	return jObj_.serialize();
}
