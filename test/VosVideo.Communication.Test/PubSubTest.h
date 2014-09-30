#pragma once
#include <windows.h>
#include <gtest\gtest.h>
#include "VosVideo.Communication/PubSubService.h"
#include "VosVideo.Data/ReceivedData.h"

using namespace std;
using namespace vosvideo::communication;
using namespace vosvideo::data;

class SubscriptionTypeStub
{

};


class MessageReceiverStub final : public MessageReceiver
{
	public:
		MessageReceiverStub() : messageReceived_(false){}

		bool GetMessageReceived(){return messageReceived_;}

		void SetWaitHandle(HANDLE hWaitHandle)
		{
			hWaitHandle_ = hWaitHandle;
		}

		virtual void OnMessageReceived(const std::shared_ptr<vosvideo::data::ReceivedData> receivedMessage)
		{
			messageReceived_ = true;
			SetEvent(hWaitHandle_);
		}
	private:
		HANDLE hWaitHandle_;
		bool messageReceived_;

};

class ReceivedDataStub final : public ReceivedData
{
	public:
		ReceivedDataStub(){}
		virtual ~ReceivedDataStub(){}

		virtual void GetMessage(std::wstring& payload){}
		virtual void GetMessage(web::json::value& jmessage){}
		virtual void ToJsonValue(web::json::value& obj) const{}
		virtual void FromJsonValue(web::json::value& obj){}
		virtual std::wstring ToString() const {return L"";}
};
