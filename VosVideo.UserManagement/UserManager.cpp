#include "stdafx.h"
#include <boost/format.hpp>

#include "VosVideo.Data/WebsocketConnectionOpenedMsg.h"
#include "VosVideo.Data/WebsocketConnectionClosedMsg.h"
#include "VosVideo.Communication/TypeInfoWrapper.h"
#include "VosVideo.Communication/HttpClientException.h"
#include "UserManager.h"

using namespace std;
using namespace util;
using boost::wformat;
using namespace concurrency;
using namespace vosvideo::usermanagement;
using namespace vosvideo::communication;
using namespace vosvideo::configuration;
using namespace vosvideo::data;

UserManager::UserManager(std::shared_ptr<CommunicationManager> communicationManager, 
						 std::shared_ptr<ConfigurationManager> configurationManager,
						 std::shared_ptr<PubSubService> pubsubService) :
						 communicationManager_(communicationManager), 
						 configurationManager_(configurationManager), 
						 pubSubService_(pubsubService), 
						 logInInProgress_(false)
{
	std::vector<TypeInfoWrapper> interestedTypes;

	TypeInfoWrapper typeInfo = typeid(WebsocketConnectionOpenedMsg);
	interestedTypes.push_back(typeInfo);

	typeInfo = typeid(WebsocketConnectionClosedMsg);
	interestedTypes.push_back(typeInfo);

	pubSubService_->Subscribe(interestedTypes, *this);	
	communicationManager->ConnectToWsConnectionProblemSignal(boost::bind(&UserManager::ReLoginAsync, this));
	communicationManager->ConnectToRestConnectionProblemSignal(boost::bind(&UserManager::ReAuthAsync, this));
}


UserManager::~UserManager()
{
}

wstring& UserManager::GetAccountId()
{
	return userAccountId_;
}

void UserManager::ReLoginAsync()
{
	try
	{
		logInInProgress_ = false;
		LogInAsync(loginRequest_);
	}
	catch (...)
	{
		LOG_DEBUG("Relogin failed.");
		ReLoginAsync();
	}
}

void UserManager::ReAuthAsync()
{
	LOG_DEBUG("Login requested");
	web::json::value jsonVal;
	loginRequest_.ToJsonValue(jsonVal);
	communicationManager_->HttpPost(L"/auth?format=json", jsonVal).wait();
}

concurrency::task<LogInResponse> UserManager::LogInAsync(LogInRequest const& loginRequest)
{
	if(logInInProgress_) 
	{
		throw std::runtime_error("Login is in progress please wait until it is done");
	}

	loginRequest_ = loginRequest;
	logInInProgress_ = true;
	LOG_DEBUG("Login requested");

	web::json::value jsonVal;
	loginRequest.ToJsonValue(jsonVal);

	// Authenticate by REST task
	auto loginToRestServiceTask = communicationManager_->HttpPost(L"/auth?format=json", jsonVal);
	auto getUserProfileTask = loginToRestServiceTask.then([&](web::json::value& resp)
	{
			return communicationManager_->HttpGet(L"/user?format=json&NeedLinkedDevices=false&UserName=" + loginRequest.GetUserName());
	});

	auto getTokenTask = getUserProfileTask.then([&](web::json::value& resp)
	{
		SetAccountIdFromUserJson(resp);
		wstring tokenUri = str(wformat(L"/token/temp?format=json&AccountId=%1%&UserName=%2%") % userAccountId_ % loginRequest.GetUserName());
		return communicationManager_->HttpGet(tokenUri);
	});

	// Finally open connection to websocket server
	auto webSockAuthTask = getTokenTask.then([&](web::json::value& resp)
	{
		std::wstring websocketServerUri = configurationManager_->GetWebsocketUri();
		Peer token;
		GetTokenFromJson(resp, token);
		logInResponse_.SetPeer(token);
		LOG_DEBUG(L"Peer token received: " + logInResponse_);

		wstring connUri = str(wformat(L"%1%?t=%2%&ct=1&s=%3%") % websocketServerUri % token.GetPeerId() % configurationManager_->GetSiteId()); 
		LOG_DEBUG(L"Reply with connection string: " << StringUtil::ToString(connUri));
		communicationManager_->WebsocketConnect(connUri);
	});

	webSockAuthTask.then([=](task<void> end_task)
	{
		try
		{
			end_task.get();
		}
		catch (...)
		{
			throw;
		}
	}).wait();

	concurrency::task<LogInResponse> loginToWebsocketServerTask(wsOpenedCompletionEvent_);

	return loginToWebsocketServerTask;
}

void UserManager::OnMessageReceived(const shared_ptr<vosvideo::data::ReceivedData> receivedMessage)
{
	wsOpenedCompletionEvent_.set(logInResponse_);
	logInInProgress_ = false;
	set<wstring> clientPeers;

	wstring fromPeer;
	receivedMessage->GetFromPeer(fromPeer);
	//If we can't find a peer sender of the message we will ignore it
	if (fromPeer == L"")
		return;

	if(dynamic_pointer_cast<WebsocketConnectionOpenedMsg>(receivedMessage))
	{
		shared_ptr<WebsocketConnectionOpenedMsg> openedMsg = dynamic_pointer_cast<WebsocketConnectionOpenedMsg>(receivedMessage);
		web::json::value jsonMsg;
		receivedMessage->ToJsonValue(jsonMsg);
		GetClientPeersFromJson(jsonMsg, clientPeers);
		lock_guard<mutex> lock(mutex_);

		clientPeers_ = clientPeers;
	}
	else if(dynamic_pointer_cast<WebsocketConnectionClosedMsg>(receivedMessage))
	{
		shared_ptr<WebsocketConnectionClosedMsg> closedMsg = dynamic_pointer_cast<WebsocketConnectionClosedMsg>(receivedMessage);
		web::json::value jsonMsg;
		receivedMessage->ToJsonValue(jsonMsg);
		GetClientPeersFromJson(jsonMsg, clientPeers);
		lock_guard<mutex> lock(mutex_);

		for (set<wstring>::iterator iter = clientPeers.begin(); iter != clientPeers.end(); ++iter)
		{
			clientPeers_.erase(*iter);
		}
	}
}

// Broadcast message to all connected peers 
void UserManager::NotifyAllUsers(shared_ptr<SendData> outMsg)
{
	lock_guard<mutex> lock(mutex_);

	for (set<wstring>::iterator iter = clientPeers_.begin(); iter != clientPeers_.end(); ++iter)
	{
		string respRtbc;
		CommunicationManager::CreateWebsocketMessageString(logInResponse_.GetPeer().GetPeerId(), *iter, outMsg, respRtbc);
		communicationManager_->WebsocketSend(respRtbc);
	}
}

std::wstring UserManager::GetByKeyFromJson(web::json::value& jval, wstring key)
{
	if (!jval.has_field(key))
	{
		LOG_CRITICAL("No " << StringUtil::ToString(key) << " was found in response.");
		return L"";
	}
	return jval.at(key).as_string();
}

void UserManager::SetAccountIdFromUserJson( web::json::value& jval)
{
	for (auto& firstVal : jval.at(U("UserList")).as_array())
	{
		for (auto& p : firstVal.as_object())
		{
			if (p.first == L"AccountId")
			{
				userAccountId_ = p.second.serialize();
				return;
			}
		}
	}
}

void UserManager::GetTokenFromJson( web::json::value& jval, vosvideo::communication::Peer& peer)
{
	wstring val = GetByKeyFromJson(jval, L"Id");
	val.erase(std::remove(val.begin(), val.end(), '-'), val.end());
	peer = vosvideo::communication::Peer(val);
}	

void UserManager::GetClientPeersFromJson(web::json::value& jval, set<wstring>& clientPeers)
{
	if (jval.type() == web::json::value::Array)
	{
		auto arr = jval.as_array();
		for (web::json::array::iterator iter = arr.begin(); iter != arr.end(); ++iter)
		{
			GetClientPeerFromJson(*iter, clientPeers);
		}
	}
	else
	{
		GetClientPeerFromJson(jval, clientPeers);
	}
}

void UserManager::GetClientPeerFromJson(web::json::value& jval, set<wstring>& clientPeers)
{
	wstring peerId;
	wstring connType;

	peerId = jval.at(U("p")).as_string();
	connType = jval.at(U("ct")).as_string();
	//U("s"), Site id, irrelevant for client 

	if (connType == L"0") // Interested only in clients
	{
		clientPeers.insert(peerId);
	}
}