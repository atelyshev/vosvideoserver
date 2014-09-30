#pragma once
#include <windows.h>
#include <wincred.h>
#include <tchar.h>

// Use cases: 
// set new credentials: SetCredentials calls VerifyCredentials pass user and password, verify with REST service and save them if confirmed
// check credentials: when server starts, VerifyCredentials without parameters calls GetCredentials and verify with REST service
namespace vosvideo
{
	namespace configuration
	{
		class ServiceCredentialsManager
		{
		public:
			ServiceCredentialsManager();
			virtual ~ServiceCredentialsManager();

			// Locally saves in password storage
			void SetCredentials(std::wstring& userName, std::wstring& password);
			bool HasCredentials();
			void GetCredentials(std::wstring& userName, std::wstring& password);

			// verify if credentials fine
			bool VerifyCredentials(std::wstring& userName, std::wstring& password);
			bool VerifyCredentials();
		private:
			bool VerifyCredentialsWithService(std::wstring& userName, std::wstring& password);

			static LPWSTR targetName_;
		};

	}
}