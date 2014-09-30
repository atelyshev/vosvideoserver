#include "stdafx.h"
#include "SendData.h"

using namespace std;
using namespace vosvideo::data;

std::wstring SendData::objName_ = L"RtbcOutMsg"; 

void SendData::GetOutMsgText(wstring& msgText)
{
	msgText = msgText_;
}