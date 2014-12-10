#include "stdafx.h"

#include <VosVideoCommon/ComHelper.h>
#include "CredentialOp.h"

using vosvideo::cameraplayer::CredentialOp;

CredentialOp::CredentialOp(IMFNetCredential *pCredential) :
	nRef_(1), 
	dwFlags_(0), 
	pCredential_(pCredential)
{
	pCredential_->AddRef();
}

CredentialOp::~CredentialOp()
{
	SafeRelease(pCredential_);
}

STDMETHODIMP CredentialOp::QueryInterface(REFIID riid, void** ppv)
{
	static const QITAB qit[] = 
	{
		QITABENT(CredentialOp, IUnknown),
		{ 0 }
	};
	return QISearch(this, qit, riid, ppv);
}      

STDMETHODIMP_(ULONG) CredentialOp::AddRef()
{
	return ++nRef_;
}

STDMETHODIMP_(ULONG) CredentialOp::Release()
{
	LONG cRef = --nRef_;
	if (cRef == 0)
	{
		delete this;
	}
	return cRef;
}
