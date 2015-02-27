#include "stdafx.h"
#include "VosVideo.Configuration/ConfigurationManager.h"
#include "DirectoryChangesNotifier.h"

using namespace std;
using namespace boost::signals2;
using namespace vosvideo::archive;

DirectoryChangesNotifier::DirectoryChangesNotifier(std::shared_ptr<vosvideo::configuration::ConfigurationManager> configManager) : 
	configManager_(configManager)
{
	winMonitor_.reset(new WinDirectoryMonitor(configManager_, this));
}


DirectoryChangesNotifier::~DirectoryChangesNotifier()
{
}

