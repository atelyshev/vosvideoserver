#pragma once
#include "VosVideo.Configuration/ConfigurationManager.h"
#include "ReadDirectoryChanges.h"

namespace vosvideo
{
	namespace archive
	{
		class DirectoryChangesNotifier;

		class WinDirectoryMonitor
		{
		public:
			WinDirectoryMonitor(std::shared_ptr<vosvideo::configuration::ConfigurationManager> configManager,
				DirectoryChangesNotifier* changesNotifier);
			~WinDirectoryMonitor();

			void DoMonitorArchiveDirectory();

		private:
			void ChangeDiskSpace(const std::wstring& wstrDir);

			static const uint64_t reservedDiskSpace_ = 10737418240;

			DirectoryChangesNotifier* changesNotifier_;
			std::shared_ptr<vosvideo::configuration::ConfigurationManager> configManager_;
			CReadDirectoryChanges changes_;
			bool inRemovingState_;
			HANDLE endLocalDirectoryEvent_;
			HANDLE thrHandle_;
		};
	}
}
