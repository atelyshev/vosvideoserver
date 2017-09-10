#include "stdafx.h"
#include "ShutdownCameraProcessRequestMsg.h"

using namespace std;
using namespace vosvideo::data;

ShutdownCameraProcessRequestMsg::ShutdownCameraProcessRequestMsg()
{
	jObj_[L"mt"] = web::json::value::number(static_cast<int>(vosvideo::data::MsgType::ShutdownCameraProcessRequestMsg));
}


ShutdownCameraProcessRequestMsg::~ShutdownCameraProcessRequestMsg()
{
}

void ShutdownCameraProcessRequestMsg::FromJsonValue(const web::json::value& obj )
{
}

wstring ShutdownCameraProcessRequestMsg::ToString() const
{
	return jObj_.serialize();
}
