#pragma once

#include <atomic>
#include <Mfidl.h>
#include <Mferror.h>

namespace vosvideo
{
	namespace cameraplayer
	{
		class NetCredentialManager : public IMFNetCredentialManager, IMFAsyncCallback 
		{
			std::atomic<long> m_nRef;
			std::wstring _username;
			std::wstring _pass;

			IMFNetCredentialCache   *pCredentialCache_;

		public:
			NetCredentialManager(std::wstring& username, std::wstring& pass);

			~NetCredentialManager();

			STDMETHODIMP QueryInterface(REFIID riid, void** ppv);

			STDMETHODIMP_(ULONG) AddRef();

			STDMETHODIMP_(ULONG) Release();

			STDMETHODIMP BeginGetCredentials(
				MFNetCredentialManagerGetParam* pParam,
				IMFAsyncCallback* pCallback,
				IUnknown* pState );

			STDMETHODIMP EndGetCredentials(IMFAsyncResult* pResult, IMFNetCredential** ppCred);

			STDMETHODIMP SetGood(IMFNetCredential* pCred, BOOL fGood);

			STDMETHODIMP GetParameters(DWORD* pdwFlags, DWORD* pdwQueue);

			STDMETHODIMP Invoke(IMFAsyncResult* pResult);
		};
	}
}