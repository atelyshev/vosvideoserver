#include "stdafx.h"
#include "DtoParseException.h"
#include "WebSocketMessageParser.h"

using namespace std;
using namespace util;
using namespace vosvideo::data;

WebSocketMessageParser::WebSocketMessageParser(const string& msg)
{
	// Now it is time to use json parser
	originalMsg_ = StringUtil::ToWstring(msg);
	web::json::value jpayload = web::json::value::parse(originalMsg_);
	messageType_ = GetMessageType(jpayload);

	if(messageType_  != MsgType::CameraConfMsg && 
		messageType_ != MsgType::ShutdownCameraProcessRequestMsg)
	{
		fromPeer_ = jpayload.at(U("fp")).as_string();
		toPeer_ = jpayload.at(U("tp")).as_string();
		jpayload_ = jpayload.at(U("m"));
	}
	else
	{
		jpayload_ = jpayload;
	}
}

MsgType WebSocketMessageParser::GetMessageType(const web::json::value& jpayload)
{
	return static_cast<MsgType>(jpayload.at(U("mt")).as_integer());
}

MsgType WebSocketMessageParser::GetMessageType()
{
	return messageType_;
}

void WebSocketMessageParser::GetPayload(std::wstring& message)
{
	message = jpayload_.serialize();
}

void WebSocketMessageParser::GetPayload(web::json::value& jpayload)
{
	jpayload = jpayload_;
}

std::wstring WebSocketMessageParser::GetMessage()
{
	return originalMsg_;
}

std::wstring WebSocketMessageParser::GetToPeer()
{
	return toPeer_;
}

std::wstring WebSocketMessageParser::GetFromPeer()
{
	return fromPeer_;
}

