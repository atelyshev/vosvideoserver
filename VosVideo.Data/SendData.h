#pragma once
#include <cpprest/json.h>
#include "MsgTypes.h"

namespace vosvideo
{
	namespace data
	{
		class SendData 
		{
		public:
			SendData(){}
			virtual ~SendData(){}
			MsgType GetMsgType() {return msgType_;}
			void GetOutMsgText(std::wstring& msgText);
			virtual void GetAsJsonString(std::wstring& jsonStr) = 0;

		protected:
			MsgType msgType_;
			std::wstring msgText_;
			// Represents object name used at the beginning json string
			static std::wstring objName_;
		};
	}
}

