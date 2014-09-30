#include "stdafx.h"
#include <stdint.h>
#include <VosVideoCommon/StringUtil.h>
#include "InterprocessComm.h"
#include "InterprocessCommException.h"

using namespace std;
using namespace util;
using namespace vosvideo::communication;

InterprocessComm::InterprocessComm(std::shared_ptr<InterprocessCommEngine> engine) : 
	engine_(engine)
{
}

InterprocessComm::~InterprocessComm()
{
}

void InterprocessComm::OpenAsChild()
{
	engine_->OpenAsChild();
}

void InterprocessComm::OpenAsParent()
{
	engine_->OpenAsParent();
}

void InterprocessComm::Send(const std::wstring& msg)
{
	engine_->Send(msg);
}

void InterprocessComm::Receive()
{
	engine_->Receive();
}

void InterprocessComm::ReceiveAsync()
{
	engine_->ReceiveAsync();
}

