#pragma once
#include "ConnectionProblemNotifier.h"
#include "PubSubService.h"

namespace vosvideo
{
	namespace communication
	{
		class WebsocketClientEngine : public ConnectionProblemNotifier
		{
		public:
			WebsocketClientEngine(std::shared_ptr<PubSubService> pubsubService);
			virtual ~WebsocketClientEngine();
			virtual void Connect(std::wstring const& url) = 0;
			virtual void Send(const std::string &msg) = 0;
			virtual void Close() = 0;
		protected:
			std::shared_ptr<PubSubService> pubSubService_;
		};
	}
}


