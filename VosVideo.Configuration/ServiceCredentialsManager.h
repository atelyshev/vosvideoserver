#pragma once
#include <wincred.h>

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
			void SetCredentials(const std::wstring& userName, const std::wstring& password);
			bool HasCredentials();
			std::tuple<std::wstring, std::wstring> GetCredentials();

			// verify if credentials fine
			bool VerifyCredentials(const std::wstring& userName, const std::wstring& password);
			bool VerifyCredentials();
		private:
			bool VerifyCredentialsWithService(const std::wstring& userName, const std::wstring& password);

			wchar_t* targetName_ = L"vosvideo.com/account";
		};

	}
}