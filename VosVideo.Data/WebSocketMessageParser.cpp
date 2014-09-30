#include "stdafx.h"
#include <vosvideocommon/StringUtil.h>
#include "DtoParseException.h"
#include "WebSocketMessageParser.h"

using namespace std;
using namespace util;
using namespace vosvideo::data;

WebSocketMessageParser::WebSocketMessageParser(const string& msg)
{
	// Now it is time to use json parser
	wstring wmsg;
	StringUtil::ToWstring(msg, wmsg);	
	originalMsg_ = wmsg;
	web::json::value jpayload = web::json::value::parse(wmsg);
	messageType_ = GetMessageType(jpayload);

	if(messageType_  != MsgType::CameraConfMsg)
	{
		for (web::json::value::iterator iter = jpayload.begin(); iter != jpayload.end(); iter++)
		{
			if (iter->first.as_string() == U("fp"))
			{
				fromPeer_ = iter->second.as_string();
			}
			else if (iter->first.as_string() == U("tp"))
			{
				toPeer_ = iter->second.as_string();
			}
			else if (iter->first.as_string() == U("m"))
			{
				jpayload_ = iter->second;
			}
		}
	}
	else
	{
		jpayload_ = jpayload;
	}
}

WebSocketMessageParser::~WebSocketMessageParser()
{
}

MsgType WebSocketMessageParser::GetMessageType(web::json::value& jpayload)
{
	for (web::json::value::iterator iter = jpayload.begin(); iter != jpayload.end(); iter++)
	{
		if (iter->first.as_string() == U("mt"))
		{
			return static_cast<MsgType>(iter->second.as_integer());
		}
	}

	throw DtoParseException("Message type not found");
}

MsgType WebSocketMessageParser::GetMessageType()
{
	return messageType_;
}

void WebSocketMessageParser::GetPayload(std::wstring& message)
{
	message = jpayload_.to_string();
}

void WebSocketMessageParser::GetPayload(web::json::value& jpayload)
{
	jpayload = jpayload_;
}

void WebSocketMessageParser::GetMessage(std::wstring& message)
{
	message = originalMsg_;
}

void WebSocketMessageParser::GetToPeer(std::wstring& toPeer)
{
	toPeer = toPeer_;
}

void WebSocketMessageParser::GetFromPeer(std::wstring& fromPeer)
{
	fromPeer = fromPeer_;
}

