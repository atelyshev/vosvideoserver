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
		msg = parser_->GetMessage();
	}

	return msg;
}

web::json::value ReceivedData::ToJsonValue() const
{
	web::json::value jObj;
	if(parser_)
	{
		parser_->GetPayload(jObj);
	}
	return jObj;
}

void ReceivedData::GetFromPeer(std::wstring& fromPeer)
{
	if(parser_)
	{
		fromPeer = parser_->GetFromPeer();
	}
}

void ReceivedData::GetToPeer(std::wstring& toPeer)
{
	if(parser_)
	{
		toPeer = parser_->GetToPeer();
	}
}
