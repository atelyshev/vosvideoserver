#include "stdafx.h"
#include "StringUtil.h"
#include "SystemUtil.h"

using namespace util;

std::string SystemUtil::GetLastErrorMsg()
{
	LPVOID lpMsgBuf;
	DWORD dw = GetLastError();

	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		nullptr,
		dw,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR) &lpMsgBuf,
		0, nullptr );

	std::wstring wstr((wchar_t*)lpMsgBuf);
	std::string str;

	return StringUtil::ToString(wstr);
}
