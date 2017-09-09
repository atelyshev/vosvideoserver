#pragma once
#include "SendData.h"

namespace vosvideo
{
	namespace data
	{
		class DeviceDiscoveryResponseMsg : public SendData
		{
		public:
			DeviceDiscoveryResponseMsg(web::json::value jobjVect);
			virtual ~DeviceDiscoveryResponseMsg();

			void GetAsJsonString(std::wstring& jsonStr);

		private:
			static const MsgType msgType = MsgType::DeviceDiscoveryOutMsg;
			web::json::value jobjVect_;
		};
	}
}
