#pragma once
#include <memory>
#include "WebsocketClientEngine.h"

namespace vosvideo
{
	namespace communication
	{
		class WebsocketClient final
		{
		public:
			friend class CommunicationManager;

			WebsocketClient(std::shared_ptr<WebsocketClientEngine> websocketClientEngine);
			~WebsocketClient(void);
			void Connect(std::wstring const & url) const;
			void Send(const std::string &msg);
			void Close();
		private :
			std::shared_ptr<WebsocketClientEngine> engine_;			
		};
	}
}
