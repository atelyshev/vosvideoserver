#include "stdafx.h"
#include "IceCandidateResponseMsg.h"

using namespace std;
using namespace vosvideo::data;

IceCandidateResponseMsg::IceCandidateResponseMsg()
{
}


IceCandidateResponseMsg::~IceCandidateResponseMsg()
{
}

void IceCandidateResponseMsg::FromJsonValue( web::json::value& obj )
{
}

wstring IceCandidateResponseMsg::ToString() const
{
	wstring msg;
	parser_->GetMessage(msg);
	return msg;
}
