#include "stdafx.h"
#include "VosVideo.Common/SystemUtil.h"

#include "CredentialsException.h"
#include "ServiceCredentialsManager.h"

using namespace std;
using namespace util;
using vosvideo::configuration::ServiceCredentialsManager;

ServiceCredentialsManager::ServiceCredentialsManager()
{
}


ServiceCredentialsManager::~ServiceCredentialsManager()
{
}

bool ServiceCredentialsManager::HasCredentials()
{
	wstring userName;
	wstring password;
	try
	{
		tie(userName, password) = GetCredentials();
	}
	catch(CredentialsException)
	{
		return false;
	}
	
	return true;
}

void ServiceCredentialsManager::SetCredentials(const wstring& userName, const wstring& password)
{
	uint32_t cbCreds = password.length() * sizeof(wchar_t) + 1;

	CREDENTIALW cred = {0};
	cred.Type = CRED_TYPE_GENERIC;
	cred.TargetName = targetName_;
	cred.CredentialBlobSize = cbCreds;
	cred.CredentialBlob = (LPBYTE) password.c_str();
	cred.Persist = CRED_PERSIST_LOCAL_MACHINE;
	cred.UserName = (LPWSTR)userName.c_str();

	if (!::CredWriteW (&cred, 0))
	{
		string errMsg = SystemUtil::GetLastErrorMsg();
		throw CredentialsException(errMsg);
	}
}

tuple<wstring, wstring> ServiceCredentialsManager::GetCredentials()
{
	PCREDENTIALW pcred;

	if(!::CredReadW (targetName_, CRED_TYPE_GENERIC, 0, &pcred))
	{
		string errMsg = "Failed to access credentials. " + SystemUtil::GetLastErrorMsg();
		loggers::EventLogLogger::WriteError(util::StringUtil::ToWstring(errMsg));
		throw CredentialsException(errMsg);
	}

	auto password = wstring((wchar_t*)pcred->CredentialBlob);
	auto userName = pcred->UserName;
	// must free memory allocated by CredRead()!
	::CredFree (pcred);
	return make_tuple(userName, password);
}

bool ServiceCredentialsManager::VerifyCredentials()
{
	wstring userName;
	wstring password;

	// Retrieve credentials from local repository
	tie(userName, password) = GetCredentials();
	// Verify with REST service
	VerifyCredentialsWithService(userName, password);
	return true;
}

bool ServiceCredentialsManager::VerifyCredentials(const wstring& userName, const wstring& password)
{
	// Verify with REST service
	VerifyCredentialsWithService(userName, password);
	return true;
}

bool ServiceCredentialsManager::VerifyCredentialsWithService(const wstring& userName, const wstring& password)
{
	// TODO: implement verification with REST service
	return true;
}
