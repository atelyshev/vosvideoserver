#include "stdafx.h"
#include "DeviceDiscoveryResponseMsg.h"

using namespace std;
using namespace util;
using namespace vosvideo::data;

DeviceDiscoveryResponseMsg::DeviceDiscoveryResponseMsg(web::json::value jobjVect) : 
	jobjVect_(jobjVect)
{
	// Trick, local static goes to base class static
	msgType_ = msgType;
}


DeviceDiscoveryResponseMsg::~DeviceDiscoveryResponseMsg()
{
}

void DeviceDiscoveryResponseMsg::GetAsJsonString(std::wstring& jsonStr)
{
	stringstream stream;
	jobjVect_.serialize(stream);
	jsonStr = StringUtil::ToWstring(stream.str());
}