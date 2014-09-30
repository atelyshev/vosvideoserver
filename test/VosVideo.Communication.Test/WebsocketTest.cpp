#include "stdafx.h"
#include "WebsocketTest.h"


using namespace vosvideo::communication;

void CreateWebSocketClient();
void WebsocketSend();
std::shared_ptr<CommunicationManager> CreateCommManager();

TEST(VosVideoCommunicationWebsocket, Can_Create)
{
	EXPECT_NO_THROW(CreateWebSocketClient());
}

//TEST(VosVideoCommunicationWebsocket, Connect)
//{
//	
//}

TEST(VosVideoCommunicationWebsocket, Send)
{
	EXPECT_NO_THROW(WebsocketSend());
}


void CreateWebSocketClient(){
	CreateCommManager();
}

void WebsocketSend(){
	auto commManager = CreateCommManager();
	commManager->WebsocketSend("test message");
}


