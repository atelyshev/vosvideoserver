// VosVideo.UserManagement.Test.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <gtest\gtest.h>
#include "VosVideo.UserManagement/UserManager.h"
#include "VosVideo.Communication/CommunicationManager.h"
#include "VosVideo.Configuration/ConfigurationManager.h"
#include "VosVideo.Test.Common/HttpClientEngineStub.h"
#include "VosVideo.Test.Common/WebsocketClientEngineStub.h"

using namespace vosvideo::usermanagement;
using namespace vosvideo::communication;
using namespace vosvideo::configuration;

std::shared_ptr<CommunicationManager> CreateCommManager();
LogInResponse UserManagerLoginTest();

TEST(VosVideoUserManagement, LoginTest)
{
	EXPECT_NO_THROW(UserManagerLoginTest());
}

int _tmain(int argc, _TCHAR* argv[])
{
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}

LogInResponse UserManagerLoginTest()
{
	std::shared_ptr<ConfigurationManager> configManager(new ConfigurationManager());
	std::shared_ptr<CommunicationManager> commManager = CreateCommManager();
	std::shared_ptr<PubSubService> commMessageBroker(new PubSubService());
	UserManager userManager(commManager, configManager, commMessageBroker);
	LogInRequest loginRequest(L"testuser", L"testpassword");

	concurrency::task<LogInResponse> loginResponseTask = userManager.LogInAsync(loginRequest);
	LogInResponse loginResponse = loginResponseTask.get();
	return loginResponse;
}

std::shared_ptr<CommunicationManager> CreateCommManager()
{
	std::shared_ptr<HttpClientEngineStub> httpClientEngine(new HttpClientEngineStub());
	std::shared_ptr<HttpClient> httpClient(new HttpClient(httpClientEngine));
	std::shared_ptr<PubSubService> communicationPubSub;
	std::shared_ptr<WebsocketClientEngineStub> websocketClientEngine(new WebsocketClientEngineStub(communicationPubSub));
	std::shared_ptr<WebsocketClient> websocketClient(new WebsocketClient(websocketClientEngine));
	std::shared_ptr<CommunicationManager> commManager = std::shared_ptr<CommunicationManager>(new CommunicationManager(httpClient, websocketClient));
	return commManager;
}

