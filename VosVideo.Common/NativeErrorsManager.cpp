#include "stdafx.h"

#include <boost/assign/list_inserter.hpp>
#include <boost/assign/list_of.hpp>
#include "NativeErrorsManager.h"

using namespace std;
using namespace boost::assign;
using namespace util;

const NativeErrorsManager::NativeErrorCodesMap NativeErrorsManager::errorCodes_ = 
{
	{0xC00D0035, L"The specified device could not be found"},
	{E_ACCESSDENIED, L"Access is denied."},
	{E_ABORT, L"Operation aborted."},
	{WAIT_TIMEOUT, L"Operation aborted. Timeout."},
	{MF_E_UNSUPPORTED_BYTESTREAM_TYPE, L"The byte stream type of the given URL is unsupported."},
	{REGDB_E_CLASSNOTREG, L"Class not registered, Use regsvr32 to register RTBC COM objects."},
	{ERROR_PATH_NOT_FOUND, L"The system cannot find the path specified. In case of Web Camera link is incorrect."},
	// WinHTTP errors were mult by -1 and we need to follow this logic here, but only for winHTTP
	{ERROR_WINHTTP_TIMEOUT * -1, L"The request has timed out."},
	{ERROR_WINHTTP_CANNOT_CONNECT * -1, L"Connection to the device failed."} 
};


const wstring& NativeErrorsManager::ToString(int32_t hr)
{
	auto iter = errorCodes_.find(hr);
	if(iter != errorCodes_.end())
	{
		return iter->second;
	}

	return L"";
}

