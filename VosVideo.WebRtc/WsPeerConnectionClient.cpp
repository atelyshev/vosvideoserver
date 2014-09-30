#include "stdafx.h"
#include <iosfwd>
#include "WsPeerConnectionClient.h"
#include <talk/base/logging.h>
#include <talk/base/stringutils.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/Net/HTTPClientSession.h>
#include <Poco/URI.h>
#include <Poco/StreamCopier.h>
#include "defaults.h"


using Poco::Net::HTTPClientSession;
using Poco::Net::HTTPRequest;
using Poco::Net::HTTPResponse;
using Poco::StreamCopier;

using websocketpp::client;

const std::string kFromPeerId = "fromPeerId";
const std::string kToPeerId = "toPeerId";
const std::string kType = "type";
const std::string kOffer = "offer";
const std::string kCandidate = "candidate";
const std::string kSdpMLineIndex = "sdpMLineIndex";
const std::string kSdpMid = "sdpMid";
const std::string kLabel = "label";
const std::string kId = "id";


inline int ConvertStringToInt(const std::string& str);

using vosvideo::webrtc::WsPeerConnectionClient;

WsPeerConnectionClient::WsPeerConnectionClient()
{
	ws_client_handler_ = boost::shared_ptr<WsClientHandler>(new WsClientHandler());
	endpoint_ = std::unique_ptr<client>(new client(ws_client_handler_));
	endpoint_->alog().unset_level(websocketpp::log::alevel::ALL);
	endpoint_->elog().unset_level(websocketpp::log::elevel::ALL);

	endpoint_->elog().set_level(websocketpp::log::elevel::RERROR);
	endpoint_->elog().set_level(websocketpp::log::elevel::FATAL);
}

WsPeerConnectionClient::~WsPeerConnectionClient()
{
}


void WsPeerConnectionClient::Connect(const std::string& server, int port, const std::string& client_name) 
{
	ASSERT(!server.empty());
	ASSERT(!client_name.empty());

	if (state_ != PeerConnectionClientBase::State::NOT_CONNECTED) 
	{
		LOG(WARNING) << "The client must not be connected before you can call Connect()";
		callback_->OnServerConnectionFailure();
		return;
	}

	if (server.empty() || client_name.empty()) 
	{
		callback_->OnServerConnectionFailure();
		return;
	}

	if (port <= 0)
		port = kDefaultServerPort;

	client_name_ = client_name;
	DoConnect(server, port, client_name);
}

void WsPeerConnectionClient::DoConnect(const std::string& server, const int port, const std::string& client_name)
{
	//http://localhost/vosvideo/api/webrtcpeer
	state_ = PeerConnectionClientBase::State::SIGNING_IN;
	HTTPClientSession httpClientSession(server, port);
	HTTPRequest request(HTTPRequest::HTTP_GET, "/vosvideo/api/webrtcpeer");
	httpClientSession.sendRequest(request);
	HTTPResponse response;
	std::istream& rs = httpClientSession.receiveResponse(response);
	std::ostringstream ostr;
	StreamCopier::copyStream(rs, ostr);

	std::string strPeerId = ostr.str();
	try
	{
		//Convert the strPeerId into an int
		my_id_ = ConvertStringToInt(strPeerId);
		//We got a valid peer id back from the http server, now we will connect to the websocket server	
		ConnectWithWebsocketServer(server, strPeerId);
	}
	catch (boost::bad_lexical_cast const&)
	{
		LOG(WARNING) << "Error while getting the peerid";
		my_id_ = -1;
		Close();
	}
}


bool WsPeerConnectionClient::SendToPeer(int peer_id, const std::string& message) 
{
	LOG(INFO) << "Sending message to peer";
	Json::Reader reader;
	Json::Value jmessageR;

	if(!reader.parse(message, jmessageR))
	{
		LOG(WARNING) << "Received unknown message. " << message;
		return false;
	}

	std::string type;
	std::string sdp;
	std::string candidate;
	GetStringFromJsonObject(jmessageR, kType, &type);
	GetStringFromJsonObject(jmessageR, kCandidate, &candidate);

	if(type == kOffer)
	{
		//Send the offer wrapped into an offer request (with toPeerId)
		SendOfferRequest(jmessageR, peer_id);
		return true;
	}
	else if(!candidate.empty())
	{
		//Send ICE Candidate message
		SendIceCandidate(jmessageR);
		return true;
	}
	return false;  
}

bool WsPeerConnectionClient::SendHangUp(int peer_id) 
{
	//TODO: Implement
	return false;
	//return SendToPeer(peer_id, kByeMessage);
}

bool WsPeerConnectionClient::IsSendingMessage() 
{
	//TODO: Implement
	return false;
	//return state_ == CONNECTED &&
	//       control_socket_->GetState() != talk_base::Socket::CS_CLOSED;
}

bool WsPeerConnectionClient::SignOut() 
{
	Close();
	return true;
}


void WsPeerConnectionClient::OnMessage(talk_base::Message* msg) 
{
	//TODO: Implement
	//// ignore msg; there is currently only one supported message ("retry")
	//DoConnect();
}

void WsPeerConnectionClient::ConnectWithWebsocketServer(const std::string& server, const std::string strPeerId)
{
	std::string serverWithPeer = "ws://192.168.1.22:1234/websockets?peerId=" + strPeerId + "&username=" + client_name_;
	try
	{
		conn_ = endpoint_->get_connection(serverWithPeer);
		endpoint_->connect(conn_);
		_wsThread = boost::shared_ptr<boost::thread>(new boost::thread(boost::bind(&client::run, endpoint_, false)));
		SubscribeToWsClientEvents();
	}
	catch (const websocketpp::exception& e)
	{
		LOG(WARNING) << e.m_msg;
		Close();
	}
	catch(...)
	{
		LOG(WARNING) << "Unidentified exception occurred while connecting to the Websocket server";
		Close();
	}
}


void WsPeerConnectionClient::SubscribeToWsClientEvents()
{
	ws_client_handler_->SignalOpenEvent.connect(this, &WsPeerConnectionClient::OnWebsocketOpen);
	ws_client_handler_->SignalFailEvent.connect(this, &WsPeerConnectionClient::OnWebsocketFail);
	ws_client_handler_->SignalMessageEvent.connect(this, &WsPeerConnectionClient::OnWebsocketMessage);
	ws_client_handler_->SignalCloseEvent.connect(this, &WsPeerConnectionClient::OnWebsocketClose);
}


void WsPeerConnectionClient::Close()
{
	my_id_ = -1;
	peers_.clear();
	state_ = PeerConnectionClientBase::State::NOT_CONNECTED;
	callback_->OnDisconnected();
}

void WsPeerConnectionClient::OnWebsocketOpen()
{
	LOG(INFO) << "Websocket opened";
	state_ = PeerConnectionClientBase::State::CONNECTED;
	callback_->OnSignedIn();		
}

void WsPeerConnectionClient::OnWebsocketFail()
{
	LOG(WARNING) << "Websocket failed";
	Close();
}

void WsPeerConnectionClient::OnWebsocketMessage(const std::string& message)
{
	LOG(INFO) << "Websocket message";	
	Json::Reader reader;
	Json::Value jMessage;
	if(!reader.parse(message, jMessage)){
		LOG(INFO) << "Received unknown message";	
	}
	std::string strPeerId;
	std::string type;
	GetStringFromJsonObject(jMessage, kFromPeerId, &strPeerId);
	GetStringFromJsonObject(jMessage, kType, &type);

	try
	{
		if(!strPeerId.empty())
		{
			//Convert the strPeerId into an int
			int peerId = ConvertStringToInt(strPeerId);
			callback_->OnMessageFromPeer(peerId, message);
		}else{
			callback_->OnMessageFromPeer(-1, message);
		}
	}
	catch (boost::bad_lexical_cast const&)
	{
		LOG(WARNING) << "Error while getting the peerid";
	}
}

void WsPeerConnectionClient::OnWebsocketClose()
{
	LOG(INFO) << "Websocket closed";	
	Close();
}

void WsPeerConnectionClient::SendOfferRequest(const Json::Value& root, int peer_id)
{
	Json::StyledWriter writer;
	Json::Value jmessageW;		

	jmessageW[kType] = "offerrequest";
	jmessageW[kOffer] = root;
	jmessageW[kFromPeerId] = my_id_;
	jmessageW[kToPeerId] = peer_id;

	std::string offerRequestMessage = writer.write(jmessageW);
	ws_client_handler_->send(offerRequestMessage);
}

void WsPeerConnectionClient::SendIceCandidate(const Json::Value& root)
{
	Json::StyledWriter writer;
	Json::Value jmessageW;		

	jmessageW[kType] = "candidate";
	jmessageW[kLabel] = root[kSdpMLineIndex].asString();
	jmessageW[kId] = root[kSdpMid].asString();
	jmessageW[kCandidate] = root[kCandidate].asString();

	std::string candidateMessage = writer.write(jmessageW);
	ws_client_handler_->send(candidateMessage);
}

int ConvertStringToInt(const std::string& str)
{
	//Convert the str into an int
	int num = boost::lexical_cast<int>(str);
	return num;
}

