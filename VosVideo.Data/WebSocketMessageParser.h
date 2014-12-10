#pragma once
#include <cpprest/json.h>
#include "MsgTypes.h"

namespace vosvideo
{
	namespace data
	{
		class WebSocketMessageParser
		{
		public:
			WebSocketMessageParser(const std::string& msg);
			virtual ~WebSocketMessageParser();

			MsgType GetMessageType();
			void GetFromPeer(std::wstring& fromPeer);
			void GetToPeer(std::wstring& toPeer);

			void GetPayload(std::wstring& message);
			void GetPayload(web::json::value& jmessage);
			void GetMessage(std::wstring& message);

		private:
			MsgType GetMessageType(web::json::value& jpayload);

			web::json::value jpayload_;
			MsgType messageType_;
			std::wstring fromPeer_;
			std::wstring toPeer_;
			std::wstring originalMsg_;
		};
	}
}