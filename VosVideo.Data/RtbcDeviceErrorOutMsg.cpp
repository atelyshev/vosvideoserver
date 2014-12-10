#include "stdafx.h"
#include <boost/format.hpp>
#include <vosvideocommon/NativeErrorsManager.h>
#include "RtbcDeviceErrorOutMsg.h"

using namespace std;
using namespace util;
using namespace vosvideo::data;
using boost::wformat;

RtbcDeviceErrorOutMsg::RtbcDeviceErrorOutMsg(int32_t deviceId, const std::wstring& msgText, int32_t hr) : 
	deviceId_(deviceId),
	hr_(hr)
{
	// Base class cant be init in initialization list
	// Trick, local static goes to base class static
	msgType_ = msgType;
	msgText_ = msgText;
}

RtbcDeviceErrorOutMsg::~RtbcDeviceErrorOutMsg()
{
}

void RtbcDeviceErrorOutMsg::GetAsJsonString(std::wstring& jsonStr)
{
	jsonStr = str(wformat(L"{\"%1%\":{\"DeviceId\":%3%, \"UserMsg\":\"%4%\", \"SystemMsg\":\"%5%\"}}") 
		% objName_ % (int)msgType_ % deviceId_ % msgText_ % NativeErrorsManager::ToString(hr_)); 
}