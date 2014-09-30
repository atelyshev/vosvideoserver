#include "stdafx.h"
#include "SdpAnswerMsg.h"

using namespace std;
using namespace vosvideo::data;

SdpAnswerMsg::SdpAnswerMsg()
{
}

SdpAnswerMsg::~SdpAnswerMsg()
{
}

void SdpAnswerMsg::FromJsonValue( web::json::value& obj )
{
}

wstring SdpAnswerMsg::ToString() const
{
	wstring msg;
	parser_->GetMessage(msg);
	return msg;
}
