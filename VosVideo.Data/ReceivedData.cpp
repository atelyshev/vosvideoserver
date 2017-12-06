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
	_parser = parser;
}

wstring ReceivedData::GetPayload()
{
	wstring payload;

	if(_parser)
	{
		_parser->GetPayload(payload);
	}

	return payload;
}

wstring ReceivedData::ToString() const 
{
	wstring msg;

	if(_parser)
	{
		msg = _parser->GetMessage();
	}

	return msg;
}

web::json::value ReceivedData::ToJsonValue() const
{
	web::json::value jObj;
	if(_parser)
	{
		_parser->GetPayload(jObj);
	}
	return jObj;
}

std::wstring ReceivedData::GetFromPeer()
{
	return (_parser != nullptr ? _parser->GetFromPeer() : L"");
}

std::wstring ReceivedData::GetToPeer()
{
	return (_parser != nullptr ? _parser->GetToPeer() : L"");
}
