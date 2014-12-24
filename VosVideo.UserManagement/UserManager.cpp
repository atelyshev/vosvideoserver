#include "stdafx.h"
#include <boost/format.hpp>
#include <vosvideocommon/StringUtil.h>
#include <vosvideocommon/SeverityLogger.h>
#include <vosvideocommon/SeverityLoggerMacros.h>
#include "VosVideo.Data/WebsocketConnectionOpenedMsg.h"
#include "VosVideo.Data/WebsocketConnectionClosedMsg.h"
#include "VosVideo.Communication/TypeInfoWrapper.h"
#include "VosVideo.Communication/HttpClientException.h"
#include "UserManager.h"

using namespace std;
using namespace util;
using boost::wformat;
using namespace vosvideo::usermanagement;
using namespace vosvideo::communication;
using namespace vosvideo::configuration;
using namespace vosvideo::data;

UserManager::UserManager(std::shared_ptr<CommunicationManager> communicationManager, 
						 std::shared_ptr<vosvideo::configuration::ConfigurationManager> configurationManager,
						 std::shared_ptr<vosvideo::communication::PubSubService> pubsubService
						 ) 
						 : communicationManager_(communicationManager), 
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
}


UserManager::~UserManager()
{
}

wstring& UserManager::GetAccountId()
{
	return userAccountId_;
}

concurrency::task<LogInResponse> UserManager::LogInAsync(LogInRequest const& loginRequest)
{
	if(logInInProgress_) 
	{
		throw std::runtime_error("A log in is in progress please wait unit it is done");
	}

	logInInProgress_ = true;
	LOG_DEBUG("Login requested");

	web::json::value jsonVal;
	loginRequest.ToJsonValue(jsonVal);

	// Authenticate by REST task
	auto loginToHttpServerTask = communicationManager_->HttpPost(L"/Account/AuthenticateReturnToken", jsonVal);

	// Finally open connection to websocket server
	auto webSockAuthTask = loginToHttpServerTask.then
		(
		[&](web::json::value& resp)
	{
		std::wstring websocketServerUri = configurationManager_->GetWebsocketUri();
		Peer token;
		GetTokenFromJson(resp, token);
		SetAccountIdFromJson(resp);
		logInResponse_.SetPeer(token);
		LOG_DEBUG(L"Peer token received: " + logInResponse_);

		wstring connUri = str(wformat(L"%1%?t=%2%&ct=1&s=%3%") % websocketServerUri % token.GetPeerId() % configurationManager_->GetSiteId()); 
		LOG_DEBUG(L"Reply with connection string: " << StringUtil::ToString(connUri));
		communicationManager_->WebsocketConnect(connUri);
	}
	);

	webSockAuthTask.wait();

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

void UserManager::SetAccountIdFromJson( web::json::value& jval)
{
	userAccountId_ = GetByKeyFromJson(jval, L"AccountId");
}

void UserManager::GetTokenFromJson( web::json::value& jval, vosvideo::communication::Peer& peer)
{
	wstring val = GetByKeyFromJson(jval, L"Token");
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