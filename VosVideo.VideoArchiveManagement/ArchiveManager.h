#pragma once
#include "VosVideo.Configuration/ConfigurationManager.h"
#include "VosVideo.Communication/PubSubService.h"
#include "ChangesNotifier.h"
#include "VideoCatalogEntry.h"

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

		protected:
			void OnArchiveChanged(const std::wstring& path);

			VideoCatalogMap videoCatalog_;
			std::shared_ptr<vosvideo::communication::PubSubService> pubSubService_;
			std::shared_ptr<vosvideo::configuration::ConfigurationManager> configManager_;
			std::shared_ptr<vosvideo::archive::ChangesNotifier> changesNotifier_;
			//			std::shared_ptr<std::thread> directoryWatcher_;
		};
	}
}

