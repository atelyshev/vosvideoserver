#include "stdafx.h"
#include "StringUtil.h"

using namespace std;
using namespace util;

wstring StringUtil::ToWstring(const string& str)
{
	wstring wstr;
	return wstr.assign(str.begin(), str.end());
}

string StringUtil::ToString(const wstring& wstr )
{
	string str;
	return str.assign(wstr.begin(), wstr.end());
}

string StringUtil::IntToHex(int32_t i)
{
	stringstream sstream;
	sstream << "0x" 
		<< std::setfill ('0') << std::setw(sizeof(int32_t)*2) 
		<< std::hex << i;
	return sstream.str();
}

