#include "stdafx.h"
#include "DeviceDiscoveryRequestMsg.h"

using namespace std;
using namespace vosvideo::data;

DeviceDiscoveryRequestMsg::DeviceDiscoveryRequestMsg()
{
}


DeviceDiscoveryRequestMsg::~DeviceDiscoveryRequestMsg()
{
}

web::json::value DeviceDiscoveryRequestMsg::ToJsonValue() const
{
	web::json::value jObj;
	return jObj;
}

void DeviceDiscoveryRequestMsg::FromJsonValue(const web::json::value& obj)
{
}

wstring DeviceDiscoveryRequestMsg::ToString() const
{
	return L"";
}
