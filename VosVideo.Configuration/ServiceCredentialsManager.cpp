#include "stdafx.h"
#include <vosvideocommon/StringUtil.h>
#include <vosvideocommon/SystemUtil.h>

#include "CredentialsException.h"
#include "ServiceCredentialsManager.h"

using namespace std;
using namespace util;
using vosvideo::configuration::ServiceCredentialsManager;

LPWSTR ServiceCredentialsManager::targetName_ = L"vosvideo.com/account"; 

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
		GetCredentials(userName, password);
	}
	catch(CredentialsException)
	{
		return false;
	}
	
	return true;
}

void ServiceCredentialsManager::SetCredentials(wstring& userName, wstring& password)
{
	DWORD cbCreds = password.length() * sizeof(wchar_t) + 1;

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

void ServiceCredentialsManager::GetCredentials(wstring& userName, wstring& password)
{
	PCREDENTIALW pcred;

	if(!::CredReadW (targetName_, CRED_TYPE_GENERIC, 0, &pcred))
	{
		string errMsg = "Failed to access credentials. " + SystemUtil::GetLastErrorMsg();
		throw CredentialsException(errMsg);
	}

	password = wstring((wchar_t*)pcred->CredentialBlob);
	userName = pcred->UserName;
	// must free memory allocated by CredRead()!
	::CredFree (pcred);
}

bool ServiceCredentialsManager::VerifyCredentials()
{
	wstring userName;
	wstring password;

	// Retrieve credentials from local repository
	GetCredentials(userName, password);
	// Verify with REST service
	VerifyCredentialsWithService(userName, password);
	return true;
}

bool ServiceCredentialsManager::VerifyCredentials(wstring& userName, wstring& password)
{
	// Verify with REST service
	VerifyCredentialsWithService(userName, password);
	return true;
}

bool ServiceCredentialsManager::VerifyCredentialsWithService(wstring& userName, wstring& password)
{
	// TODO: implement verification with REST service
	return true;
}
