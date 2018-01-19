#include "stdafx.h"
#include "VosVideo.Data/DtoParseException.h"
#include "VosVideo.Data/WebSocketMessageParser.h"
#include "VosVideo.Data/DtoFactory.h"
#include "VosVideo.Data/CameraConfMsg.h"
#include "InterprocessQueueEngine.h"

using namespace std;
using namespace util;
using namespace boost::interprocess;
using namespace vosvideo::data;
using namespace vosvideo::communication;

string InterprocessQueueEngine::stopMsg_ = "stop";


InterprocessQueueEngine::InterprocessQueueEngine(std::shared_ptr<PubSubService> pubsubService, const std::wstring& queueNamePrefix) : 
	InterprocessCommEngine(pubsubService), openAsParent_(true), isReceiveThr_(false)
{
	queueToParentName_ = StringUtil::ToString(queueNamePrefix);
	queueFromParentName_ = queueToParentName_;
	queueToParentName_ += "_to_parent";
	queueFromParentName_ += "_from_parent";
}


InterprocessQueueEngine::~InterprocessQueueEngine()
{
	if (isReceiveThr_)
	{
		StopReceive();
		receiveThr_.join();
	}
}

void InterprocessQueueEngine::OpenAsParent()
{
	try
	{
		// Queues is persistent object, make sure that they closed and removed
		Close();

		LOG_TRACE("Create queues from parent process: " <<  queueFromParentName_ << " and " << queueToParentName_);
		mqFromParent_.reset(new boost::interprocess::message_queue(boost::interprocess::create_only, queueFromParentName_.c_str(), 1000, maxMsgSize_));
		mqToParent_.reset(new boost::interprocess::message_queue(boost::interprocess::create_only, queueToParentName_.c_str(), 1000, maxMsgSize_));
	}
	catch(interprocess_exception &ex)
	{
		LOG_CRITICAL(ex.what());
		throw;
	}
}

void InterprocessQueueEngine::OpenAsChild()
{
	openAsParent_ = false;
	try
	{
		LOG_TRACE("Open queues from child process: " << queueFromParentName_  << " and " << queueToParentName_);
		mqFromParent_.reset(new message_queue(open_only, queueFromParentName_.c_str()));
		LOG_TRACE("Queue: " << queueFromParentName_  << " successfully opened");
		mqToParent_.reset(new message_queue(open_only, queueToParentName_.c_str()));
		LOG_TRACE("Queue: " << queueToParentName_ << " successfully opened");
	}
	catch(interprocess_exception &ex)
	{
		LOG_CRITICAL(ex.what());
		throw;
	}
}

void InterprocessQueueEngine::Send(const std::string& smsg)
{
	lock_guard<std::mutex> lock(mutex_);
	try
	{
		LOG_TRACE("Pass message with size: " << smsg.size() << " and body: " << smsg);

		if (openAsParent_)
		{
			LOG_TRACE("Send message from parent process to child.");
			mqFromParent_->send(smsg.data(), smsg.size(), 0);
		}
		else
		{
			LOG_TRACE("Send message from child process to parent.");
			mqToParent_->send(smsg.data(), smsg.size(), 0);
		}
	}
	catch(interprocess_exception &ex)
	{
		LOG_CRITICAL(ex.what());
	}
}

void InterprocessQueueEngine::Send(const std::wstring& wmsg)
{
	string smsg = StringUtil::ToString(wmsg);
	Send(smsg);
}

void InterprocessQueueEngine::ReceiveAsync()
{
	LOG_TRACE("Started to receive messages async.");
	isReceiveThr_=true;

	receiveThr_ = std::thread([this]
	{
		this->Receive();
	});
}

// Send STOP message to self
void InterprocessQueueEngine::StopReceive()
{
	if (openAsParent_)
	{
		LOG_TRACE("Send Stop Receive message from child process to parent.");
		mqToParent_->send(stopMsg_.data(), stopMsg_.size(), 0);
	}
	else
	{
		LOG_TRACE("Send Stop Receive message from parent process to child.");
		mqFromParent_->send(stopMsg_.data(), stopMsg_.size(), 0);
	}
}

void InterprocessQueueEngine::Receive()
{
	string smsg;
	DtoFactory dtoFactory;
	LOG_TRACE("Started to receive messages from " << queueToParentName_ << ", " << queueFromParentName_);

	for(;;)
	{
		smsg.resize(maxMsgSize_);
		uint32_t msgRealSize;
		uint32_t prio;
		std::string sender;
		try
		{
			if (openAsParent_)
			{
				mqToParent_->receive(&smsg[0], smsg.size(), msgRealSize, prio);
				sender = "child";
			}
			else
			{
				mqFromParent_->receive(&smsg[0], smsg.size(), msgRealSize, prio);
				sender = "parent";
			}
		}
		catch(interprocess_exception &ex)
		{
			LOG_CRITICAL(ex.what());
		}
		smsg.resize(msgRealSize);
		LOG_TRACE("Message from " << sender << " process: " << smsg << " size: " << msgRealSize);

		// Check for STOP
		if (smsg == stopMsg_)
		{
			LOG_TRACE("Exit from Receive loop.");
			break;
		}

		std::shared_ptr<WebSocketMessageParser> msgParser(new WebSocketMessageParser(smsg));
		auto dto = dtoFactory.Create(msgParser->GetMessageType());
		dto->Init(msgParser);

		pubSubService_->Publish(dto);
	}
	LOG_TRACE("Finished to receive messages from " << queueToParentName_ << ", " << queueFromParentName_);
}

void InterprocessQueueEngine::Close()
{
	if (openAsParent_)
	{
		LOG_TRACE("In parent process remove interprocess queues: " << queueFromParentName_ << " and " << queueToParentName_);
		message_queue::remove(queueFromParentName_.c_str());
		LOG_TRACE("Removed queue: " <<  queueFromParentName_);
		message_queue::remove(queueToParentName_.c_str());
		LOG_TRACE("Removed queue: " <<  queueToParentName_);
	}
}
