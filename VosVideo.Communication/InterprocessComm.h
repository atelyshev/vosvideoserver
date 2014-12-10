#pragma once
#include "InterprocessCommEngine.h"

namespace vosvideo
{
	namespace communication
	{
		class InterprocessComm final
		{
		public:
			InterprocessComm(std::shared_ptr<InterprocessCommEngine> engine);
			~InterprocessComm();

			void OpenAsParent();
			void OpenAsChild();

			void Send(const std::wstring& msg);
			void Receive();
			void ReceiveAsync();

		private:
			std::shared_ptr<InterprocessCommEngine> engine_;
		};
	}
}
