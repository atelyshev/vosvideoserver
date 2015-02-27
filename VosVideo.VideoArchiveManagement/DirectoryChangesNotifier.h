#pragma once
#include "WinDirectoryMonitor.h"
#include "ChangesNotifier.h"

namespace vosvideo
{
	namespace archive
	{
		class DirectoryChangesNotifier final : public ChangesNotifier
		{
		public:
			DirectoryChangesNotifier(std::shared_ptr<vosvideo::configuration::ConfigurationManager> configManager);
			~DirectoryChangesNotifier();

		private:
			std::shared_ptr<vosvideo::configuration::ConfigurationManager> configManager_;
			std::shared_ptr<vosvideo::archive::WinDirectoryMonitor> winMonitor_;
		};
	}
}
