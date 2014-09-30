#pragma once

#include <atlbase.h>
#include <atomic>
#include <Mfidl.h>
#include <Mferror.h>

// Holds state information for the GetCredentials operation, so that work can 
// be moved to a work-queue thread.

namespace vosvideo
{
	namespace camera
	{
		struct CredentialOp : public IUnknown
		{
			std::atomic<long>   nRef_;
			IMFNetCredential    *pCredential_;
			DWORD               dwFlags_;

			CredentialOp(IMFNetCredential *pCredential);

			~CredentialOp();

			STDMETHODIMP QueryInterface(REFIID riid, void** ppv);

			STDMETHODIMP_(ULONG) AddRef();

			STDMETHODIMP_(ULONG) Release();
		};
	}
}