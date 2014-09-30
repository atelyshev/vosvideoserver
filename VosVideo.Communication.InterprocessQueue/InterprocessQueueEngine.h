#pragma once
#include <mutex>
#include <thread>  
#include <boost/interprocess/ipc/message_queue.hpp>
#include "VosVideo.Communication/InterprocessCommEngine.h"

namespace vosvideo
{
	namespace communication
	{
		class InterprocessQueueEngine final : public InterprocessCommEngine
		{
		public:
			InterprocessQueueEngine(std::shared_ptr<PubSubService> pubsubService, const std::wstring& queueNamePrefix);
			virtual ~InterprocessQueueEngine();

			virtual void OpenAsParent();
			virtual void OpenAsChild();
			virtual void Send(const std::wstring& msg);
			virtual void Send(const std::string& msg);
			virtual void Receive();
			virtual void ReceiveAsync();
			virtual void StopReceive();
			virtual void Close();

		private:
			std::shared_ptr<boost::interprocess::message_queue> mqFromParent_;
			std::shared_ptr<boost::interprocess::message_queue> mqToParent_;
			bool openAsParent_;
			std::string queueToParentName_;
			std::string queueFromParentName_;
			const static int maxMsgSize_ = 20 * 1024;
			std::thread receiveThr_;
			bool isReceiveThr_;
			std::mutex mutex_;
			static std::string stopMsg_;
		};
	}
}
