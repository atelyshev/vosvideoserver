#pragma once
#include "PubSubService.h"

namespace vosvideo
{
	namespace communication
	{
		class InterprocessCommEngine
		{
		public:
			InterprocessCommEngine(std::shared_ptr<PubSubService> pubsubService);
			virtual ~InterprocessCommEngine();

			// From parent process need to call this method
			virtual void OpenAsParent() = 0;
			// From child process need to call this method
			virtual void OpenAsChild() = 0;

			virtual void Send(const std::wstring& msg) = 0;
			// Publish data via pubsubService
			virtual void Receive() = 0;
			virtual void ReceiveAsync() = 0;
			// Stop Receive loop
			virtual void StopReceive() = 0;
			// Remove queue if parent
			virtual void Close() = 0;

		protected:
			std::shared_ptr<PubSubService> pubSubService_;
		};
	}
}