// VosVideo.Communication.Test.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <memory>
#include "TestUtils.h"
#include "VosVideo.Communication/CommunicationManager.h"
#include "VosVideo.Test.Common/HttpClientEngineStub.h"
#include "VosVideo.Test.Common/WebsocketClientEngineStub.h"
#include "PubSubTest.h"
#include "WebsocketTest.h"

using namespace std;

shared_ptr<CommunicationManager> CreateCommManager();
shared_ptr<CommunicationManager> CreateTest();
web::json::value HttpGetTest();


TEST(VosVideoCommunication, Create)
{
	EXPECT_NO_THROW(CreateTest());
}

TEST(VosVideoCommunication, HttpGet)
{
	EXPECT_NO_THROW(HttpGetTest());
}

int _tmain(int argc, _TCHAR* argv[])
{
	testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}

shared_ptr<CommunicationManager> CreateTest()
{	
	shared_ptr<CommunicationManager> commManager = CreateCommManager();
	return commManager;
}

web::json::value HttpGetTest()
{
	shared_ptr<CommunicationManager> commManager = CreateCommManager();
	auto respTask = commManager->HttpGet(L"fakepath");
	web::json::value resp = respTask.get();
	return resp;
}





