#pragma once
#include <Poco/Process.h>
#include "VosVideo.Communication/InterprocessComm.h"
#include "VosVideo.Data/CameraConfMsg.h"

namespace vosvideo
{
	namespace camera
	{
		class CameraPlayerProcess
		{
		public:
			CameraPlayerProcess(std::shared_ptr<vosvideo::communication::PubSubService> pubsubService, vosvideo::data::CameraConfMsg& conf);
			~CameraPlayerProcess();

			void Reconnect();
			void Send(const std::wstring& msg);
			// Stops the process
			void Shutdown();

		private:
			// Check if spawned process alive
			bool IsAlive();
			void Init();
			void Receive();
			void ReceiveAsync();

			std::shared_ptr<vosvideo::communication::PubSubService> pubSubService_;
			std::shared_ptr<vosvideo::communication::InterprocessComm> duplexChannel_;
			vosvideo::data::CameraConfMsg conf_;
			int32_t pid_;
		};
	}
}
