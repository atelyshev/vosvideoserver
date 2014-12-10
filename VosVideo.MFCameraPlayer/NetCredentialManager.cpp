#include "stdafx.h"

#include <Mfapi.h>
#include <nserror.h>
#include <VosVideoCommon/ComHelper.h>
#include "NetCredentialManager.h"
#include "CredentialOp.h"

using namespace std;
using vosvideo::cameraplayer::NetCredentialManager;

NetCredentialManager::NetCredentialManager(wstring& username, wstring& pass) : 
	m_nRef(1), 
	pCredentialCache_(NULL),
	_username(username),
	_pass(pass)
{ 
}

NetCredentialManager::~NetCredentialManager()
{
	SafeRelease(pCredentialCache_);
}

STDMETHODIMP NetCredentialManager::QueryInterface(REFIID riid, void** ppv)
{
	static const QITAB qit[] = 
	{
		QITABENT(NetCredentialManager, IMFNetCredentialManager),
		QITABENT(NetCredentialManager, IMFAsyncCallback),
		{ 0 }
	};
	return QISearch(this, qit, riid, ppv);
}      

STDMETHODIMP_(ULONG) NetCredentialManager::AddRef()
{
	return ++m_nRef;
}

STDMETHODIMP_(ULONG) NetCredentialManager::Release()
{
	LONG cRef = --m_nRef;
	if (cRef == 0)
	{
		delete this;
	}
	return cRef;
}

STDMETHODIMP NetCredentialManager::BeginGetCredentials(
	MFNetCredentialManagerGetParam* pParam,
	IMFAsyncCallback* pCallback,
	IUnknown* pState)
{
	if (!pParam || !pCallback)
	{
		return E_POINTER;
	}

	DWORD dwAuthenticationFlags = 0;
	DWORD dwRequirementFlags = 0;

	if (pParam->hrOp == NS_E_PROXY_ACCESSDENIED)
	{
		dwAuthenticationFlags |= MFNET_AUTHENTICATION_PROXY;
	}

	if (pParam->fAllowLoggedOnUser)
	{
		dwAuthenticationFlags |= MFNET_AUTHENTICATION_LOGGED_ON_USER;
	}

	if (pParam->fClearTextPackage)
	{
		dwAuthenticationFlags |= MFNET_AUTHENTICATION_CLEAR_TEXT;
	}

	IMFNetCredential *pCredential =  NULL;
	IMFAsyncResult* pResult = nullptr;
	CredentialOp *pOp = nullptr;
	HRESULT hr = S_OK;

	do 
	{
		if (pCredentialCache_ == nullptr)
		{
			hr = MFCreateCredentialCache(&pCredentialCache_);
			BREAK_ON_FAIL(hr);
		}

		hr = pCredentialCache_->GetCredential(
			pParam->pszUrl, 
			pParam->pszRealm, 
			dwAuthenticationFlags, 
			&pCredential, 
			&dwRequirementFlags
			);

		BREAK_ON_FAIL(hr);

		if( ( dwRequirementFlags & REQUIRE_PROMPT ) == 0 )
		{
			// The credential is good to use. Prompting the user is not required.
			hr = S_OK;
			break;
		}

		// The credential requires prompting the user. 
		pOp = new (std::nothrow) CredentialOp(pCredential);

		if (pOp == nullptr)
		{
			hr = E_OUTOFMEMORY;
			BREAK_ON_FAIL(hr);
		}

		// Set flags. Use these to inform the user if the credentials will
		// be sent in plaintext or saved in the credential cache.

		if (pParam->fClearTextPackage)
		{
			// Notify the user that credentials will be sent in plaintext.
			pOp->dwFlags_ |= MFNET_CREDENTIAL_ALLOW_CLEAR_TEXT;
		}

		if(dwRequirementFlags & REQUIRE_SAVE_SELECTED )
		{
			// Credentials will be saved in the cache by default.
			pOp->dwFlags_ |= MFNET_CREDENTIAL_SAVE;
		}

		// NOTE: The application should enable to user to deselect these two flags;
		// for example, through check boxes in the prompt dialog.


		// Now queue the work item.

		hr = MFCreateAsyncResult(pOp, pCallback, pState, &pResult);
		BREAK_ON_FAIL(hr);

		hr = MFPutWorkItem(MFASYNC_CALLBACK_QUEUE_LONG_FUNCTION, this, pResult);
	} while (false);

	SafeRelease(pResult);
	SafeRelease(pCredential);
	SafeRelease(pOp);
	return hr;
}

STDMETHODIMP NetCredentialManager::EndGetCredentials(	IMFAsyncResult* pResult, 	IMFNetCredential** ppCred)
{
	if (!pResult || !ppCred)
	{
		return E_POINTER;
	}

	*ppCred = nullptr;

	IUnknown *pUnk = nullptr;

	// Check the result of the asynchronous operation.
	HRESULT hr = pResult->GetStatus();

	if (FAILED(hr))
	{
		// The operation failed.
		goto done;
	}

	hr = pResult->GetObject(&pUnk);
	if (FAILED(hr))
	{
		goto done;
	}

	CredentialOp *pOp = static_cast<CredentialOp*>(pUnk);

	*ppCred = pOp->pCredential_;
	pOp->pCredential_ = nullptr;
done:
	SafeRelease(pUnk);
	return hr;
}

STDMETHODIMP NetCredentialManager::SetGood(IMFNetCredential* pCred, BOOL fGood)
{
	if (!pCred)
	{
		return E_POINTER;
	}

	return pCredentialCache_->SetGood(pCred, fGood);
}


STDMETHODIMP NetCredentialManager::GetParameters(DWORD* pdwFlags, DWORD* pdwQueue)
{
	return E_NOTIMPL;
}

STDMETHODIMP NetCredentialManager::Invoke(IMFAsyncResult* pResult)
{
	IUnknown *pState = nullptr;
	IMFAsyncResult *pGetCredentialsResult = nullptr;
	IUnknown *pOpState = nullptr;

	CredentialOp *pOp = nullptr;   // not AddRef'd

	HRESULT hr = pResult->GetState(&pState);

	if (SUCCEEDED(hr))
	{
		hr = pState->QueryInterface(IID_PPV_ARGS(&pGetCredentialsResult));
	}

	if (SUCCEEDED(hr))
	{
		hr = pGetCredentialsResult->GetObject(&pOpState);
	}

	if (SUCCEEDED(hr))
	{
		pOp = static_cast<CredentialOp*>(pOpState);
		pOp->pCredential_->SetUser((byte*)_username.c_str(), (_username.size() + 1) * 2, FALSE);
		pOp->pCredential_->SetPassword((byte *)_pass.c_str(), (_pass.size() + 1) * 2, FALSE);
	}

	if (SUCCEEDED(hr) && pCredentialCache_)
	{
		// Update with options set by the user.
		hr = pCredentialCache_->SetUserOptions(
			pOp->pCredential_, 
			pOp->dwFlags_
			);
	}

	if (pGetCredentialsResult)
	{
		pGetCredentialsResult->SetStatus(hr);
		MFInvokeCallback(pGetCredentialsResult);
	}

	SafeRelease(pState);
	SafeRelease(pGetCredentialsResult);
	SafeRelease(pOpState);
	return S_OK;
}
