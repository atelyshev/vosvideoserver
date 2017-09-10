#include "stdafx.h"
#include "LogInRequest.h"

using namespace std;
using vosvideo::usermanagement::LogInRequest;

LogInRequest::LogInRequest() : username_(L""), password_(L"")
{

}

LogInRequest::LogInRequest(const wstring& username, const wstring& password) : 
	username_(username), password_(password)
{
}


LogInRequest::~LogInRequest(void)
{
}

web::json::value LogInRequest::ToJsonValue() const
{
	web::json::value jObj;
	jObj[U("UserName")] = web::json::value::string(username_);
	jObj[U("Password")] = web::json::value::string(password_);
	return jObj;
}

void vosvideo::usermanagement::LogInRequest::FromJsonValue(const web::json::value& obj )
{
}

wstring LogInRequest::ToString() const
{
	return L"";
}

wstring const& LogInRequest::GetUserName() const
{
	return username_;
}


