#pragma once
#include "VosVideo.Configuration/ConfigurationManager.h"
#include "VosVideo.Communication/PubSubService.h"
#include "VideoFileDiscoverer.h"
#include "ChangesNotifier.h"

namespace vosvideo
{
	namespace archive
	{
		class MediaWatcher : public vosvideo::communication::MessageReceiver
		{
		public:
			// Proposed structure: <camera_name <beginning_time, VideoFile>>
			typedef std::unordered_map<std::wstring, std::map<uint64_t, std::shared_ptr<VideoFile>> > VideoCatalogMap;
			typedef std::map<uint64_t, std::shared_ptr<VideoFile>> VideoEntriesMap;

			MediaWatcher(std::shared_ptr<vosvideo::configuration::ConfigurationManager> configManager,
				std::shared_ptr<vosvideo::communication::PubSubService> pubsubService);
			virtual ~MediaWatcher();
			virtual void GetCatalog();
			virtual void GetCameraCatalog(uint32_t cameraId);
			virtual void OnMessageReceived(std::shared_ptr<vosvideo::data::ReceivedData> receivedMessage);

		protected:
			void OnArchiveChanged(const std::wstring& path);
			pplx::task<void> ReadVideoCatalogAsync(const std::wstring& path);
			void AddToCatalog(std::shared_ptr<VideoFile> vf);

			VideoCatalogMap videoCatalog_;

			VideoFileDiscoverer videoDiscoverer_;
			std::shared_ptr<vosvideo::communication::PubSubService> pubSubService_;
			std::shared_ptr<vosvideo::configuration::ConfigurationManager> configManager_;
			std::shared_ptr<vosvideo::archive::ChangesNotifier> changesNotifier_;
		};
	}
}

