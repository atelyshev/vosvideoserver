#pragma once
#include "VosVideo.Communication/WebsocketClientEngine.h"
#include "VosVideo.Communication/PubSubService.h"

using vosvideo::communication::WebsocketClientEngine;

class WebsocketClientEngineStub :
	public WebsocketClientEngine
{
public:
	WebsocketClientEngineStub(std::shared_ptr<vosvideo::communication::PubSubService> communicationMessageBroker);
	~WebsocketClientEngineStub(void);
	virtual void Connect(std::wstring const& url);
	virtual void Send(std::string const& msg);
	virtual void Close();
};

