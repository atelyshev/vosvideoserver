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
		NULL,
		dw,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR) &lpMsgBuf,
		0, NULL );

	std::wstring wstr((wchar_t*)lpMsgBuf);
	std::string str;

	return StringUtil::ToString(wstr);
}
