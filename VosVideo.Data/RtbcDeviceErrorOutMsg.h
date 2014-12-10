#pragma once
#include "SendData.h"

namespace vosvideo
{
	namespace data
	{
		class RtbcDeviceErrorOutMsg : public SendData
		{
		public:
			RtbcDeviceErrorOutMsg(int32_t cameraId, const std::wstring& msgText, int32_t hr = 0 );
			~RtbcDeviceErrorOutMsg();

			virtual void GetAsJsonString(std::wstring& jsonStr);

		private:
			const static MsgType msgType = MsgType::RtbcDeviceErrorOutMsg;
			int32_t deviceId_;
			int32_t hr_;
		};
	}
}


