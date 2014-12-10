#pragma once
#include "stdafx.h"
#include "ReceivedData.h"
#include "JsonObjectBase.h"

using namespace std;
using namespace vosvideo::data;

ReceivedData::ReceivedData()
{
}

ReceivedData::~ReceivedData()
{
}

void ReceivedData::Init(std::shared_ptr<WebSocketMessageParser> parser)
{
	parser_ = parser;
}

wstring ReceivedData::GetPayload()
{
	wstring payload;

	if(parser_)
	{
		parser_->GetPayload(payload);
	}

	return payload;
}

wstring ReceivedData::ToString() const 
{
	wstring msg;

	if(parser_)
	{
		parser_->GetMessage(msg);
	}

	return msg;
}

void ReceivedData::ToJsonValue(web::json::value& jmessage) const
{
	if(parser_)
	{
		parser_->GetPayload(jmessage);
	}
}

void ReceivedData::GetFromPeer(std::wstring& fromPeer)
{
	if(parser_)
	{
		parser_->GetFromPeer(fromPeer);
	}
}

void ReceivedData::GetToPeer(std::wstring& toPeer)
{
	if(parser_)
	{
		parser_->GetToPeer(toPeer);
	}
}
