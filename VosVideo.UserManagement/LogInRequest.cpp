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

void LogInRequest::ToJsonValue(web::json::value& obj) const
{
	obj[U("UserName")] = web::json::value::string(username_);
	obj[U("Password")] = web::json::value::string(password_);
}

void vosvideo::usermanagement::LogInRequest::FromJsonValue(web::json::value& obj )
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


