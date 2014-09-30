#pragma once
#include <memory>
#include <map>
#include <unordered_map>
#include <thread>

#include "VosVideo.Configuration/ConfigurationManager.h"
#include "VosVideo.Communication/PubSubService.h"
#include "VideoCatalogEntry.h"
#include "ReadDirectoryChanges.h"


namespace vosvideo
{
	namespace archive
	{
		class ArchiveManager : public vosvideo::communication::MessageReceiver
		{
		public:
			typedef std::unordered_map<int, std::map<std::wstring, std::shared_ptr<VideoCatalogEntry> > > VideoCatalogMap;

			ArchiveManager(std::shared_ptr<vosvideo::configuration::ConfigurationManager> configManager, 
				std::shared_ptr<vosvideo::communication::PubSubService> pubsubService);
			virtual ~ArchiveManager();
			virtual void GetCatalog();
			virtual void GetCameraCatalog(uint32_t cameraId);
			virtual void OnMessageReceived(const std::shared_ptr<vosvideo::data::ReceivedData> receivedMessage);
			void DoMonitorArchiveDirectory();

		protected:
			void AdjustDiskSpace(const std::wstring& wstrDir);

			VideoCatalogMap videoCatalog_;
			std::shared_ptr<vosvideo::communication::PubSubService> pubSubService_;
			std::shared_ptr<vosvideo::configuration::ConfigurationManager> configManager_;
			std::shared_ptr<std::thread> directoryWatcher_;
			CReadDirectoryChanges changes_;
			HANDLE endLocalDirectoryEvent_; 
			static const uint64_t reservedDiskSpace_ = 10737418240;
			bool inRemovingState;
		};
	}
}

