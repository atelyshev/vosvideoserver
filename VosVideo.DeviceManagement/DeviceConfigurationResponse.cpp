#include "stdafx.h"
#include "DeviceConfigurationResponse.h"

using namespace std;
using namespace vosvideo::devicemanagement;


DeviceConfigurationResponse::DeviceConfigurationResponse()
{

}

DeviceConfigurationResponse::~DeviceConfigurationResponse()
{
}


void DeviceConfigurationResponse::ToJsonValue(web::json::value& obj) const
{
}

void DeviceConfigurationResponse::FromJsonValue(web::json::value& obj)
{
}

std::wstring DeviceConfigurationResponse::ToString() const
{
	return L"";
}

wstring vosvideo::devicemanagement::operator+(wstring const& leftStr, DeviceConfigurationResponse const& rightResp)
{
	return rightResp + leftStr;
}

wstring DeviceConfigurationResponse::operator+(wstring const& str) const
{
	return str;
}

