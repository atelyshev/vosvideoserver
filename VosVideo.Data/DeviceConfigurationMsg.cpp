#include "stdafx.h"
#include "DeviceConfigurationMsg.h"

using namespace std;
using namespace vosvideo::data;

DeviceConfigurationMsg::DeviceConfigurationMsg()
{
}

DeviceConfigurationMsg::~DeviceConfigurationMsg()
{
}

web::json::value DeviceConfigurationMsg::ToJsonValue() const
{
	web::json::value jObj;
	return jObj;
}

void DeviceConfigurationMsg::FromJsonValue(const web::json::value& obj)
{
}

wstring DeviceConfigurationMsg::ToString() const
{
	return L"";
}
