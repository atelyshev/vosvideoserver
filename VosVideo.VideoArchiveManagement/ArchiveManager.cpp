#include "stdafx.h"
#include <windows.h>
#include <boost/filesystem.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include "VosVideo.Communication/TypeInfoWrapper.h"
#include "VosVideo.Data/ArchiveCatalogRequestMsg.h"
#include "ArchiveManager.h"

using namespace std;
using namespace vosvideo::communication;
using namespace vosvideo::archive;
using namespace vosvideo::data;

DWORD WINAPI LocalDirectoryMonitorStarter( LPVOID lpParam )
{
	ArchiveManager* directoryWatcher =  static_cast<ArchiveManager*>(lpParam);
	directoryWatcher->DoMonitorArchiveDirectory();
	return 0;
}

ArchiveManager::ArchiveManager(std::shared_ptr<vosvideo::configuration::ConfigurationManager> configManager, 
							   shared_ptr<vosvideo::communication::PubSubService> pubsubService) :
	configManager_(configManager),
	pubSubService_(pubsubService),
	inRemovingState(false)
{
	vector<TypeInfoWrapper> interestedTypes;

	TypeInfoWrapper typeInfo = typeid(ArchiveCatalogRequestMsg);	
	interestedTypes.push_back(typeInfo);

	//typeInfo = typeid(LiveVideoOfferMsg);
	//interestedTypes.push_back(typeInfo);

	//typeInfo = typeid(DeletePeerConnectionMsg);
	//interestedTypes.push_back(typeInfo);

	//typeInfo = typeid(WebsocketConnectionClosedMsg);
	//interestedTypes.push_back(typeInfo);

	pubSubService_->Subscribe(interestedTypes, *this);	
//	directoryWatcher_.reset(new std::thread(&ArchiveManager::DoMonitorArchiveDirectory, this ));
	CreateThread(NULL, 0, &LocalDirectoryMonitorStarter, this, 0, nullptr);
}

ArchiveManager::~ArchiveManager()
{
	SetEvent(endLocalDirectoryEvent_);
	CloseHandle(endLocalDirectoryEvent_);
}

void ArchiveManager::OnMessageReceived(const std::shared_ptr<vosvideo::data::ReceivedData> receivedMessage)
{
	wstring srvPeer;
	wstring clientPeer;
	receivedMessage->GetFromPeer(clientPeer);
	receivedMessage->GetToPeer(srvPeer);

	if(dynamic_pointer_cast<ArchiveCatalogRequestMsg>(receivedMessage))
	{
		//receivedMessage->
	}
}

void ArchiveManager::GetCatalog()
{

}

void ArchiveManager::GetCameraCatalog(uint32_t cameraId)
{

}

// dwAction meaning
//FILE_ACTION_ADDED            : "Added";
//FILE_ACTION_REMOVED          : "Deleted";
//FILE_ACTION_MODIFIED         : "Modified";
//FILE_ACTION_RENAMED_OLD_NAME : "Renamed From"
//FILE_ACTION_RENAMED_NEW_NAME : "Renamed To"

void ArchiveManager::DoMonitorArchiveDirectory()
{
	if (configManager_->GetArchivePath() == L"")
	{
		return;
	}

	const DWORD dwNotificationFlags =
		FILE_NOTIFY_CHANGE_LAST_WRITE
	  | FILE_NOTIFY_CHANGE_CREATION
	  | FILE_NOTIFY_CHANGE_FILE_NAME;

	CReadDirectoryChanges changes;
	changes.AddDirectory(configManager_->GetArchivePath(), false, dwNotificationFlags);
	endLocalDirectoryEvent_= CreateEvent( 
		NULL,               // default security attributes
		TRUE,               // manual-reset event
		FALSE,              // initial state is nonsignaled
		TEXT("WriteEvent")  // object name
		); 
	const HANDLE handles[] = { endLocalDirectoryEvent_, changes.GetWaitHandle() };

	bool bTerminate = false;

	while (!bTerminate)
	{
		DWORD rc = ::WaitForMultipleObjectsEx(_countof(handles), handles, false, INFINITE, true);
		switch (rc)
		{
		case WAIT_OBJECT_0:
			// signaled to finish thread
			bTerminate = true;
			break;

		case WAIT_OBJECT_0 + 1:
			//We've received a notification in the queue.
			{
				uint32_t dwAction;
				wstring wstrFilename;
				if (!changes.CheckOverflow())
				{
					changes.Pop(dwAction, wstrFilename);
					if (dwAction == FILE_ACTION_MODIFIED)
					{
						AdjustDiskSpace(wstrFilename);
					}
				}
			}
			break;
		case WAIT_IO_COMPLETION:
			//Nothing to do.
			break;
		}
	}

	changes.Terminate();
}

void ArchiveManager::AdjustDiskSpace(const wstring& wstrDir)
{
	if (inRemovingState == true)
	{
		inRemovingState = false;
		return;
	}

	LOG_TRACE("Created new file in video archive: " << wstrDir);

	boost::filesystem::path p(wstrDir);
	if (!p.empty())
	{
		boost::filesystem::path::iterator it = p.begin();
		boost::filesystem::space_info si = boost::filesystem::space(*it);

		// Check if time to remove old files
		if (si.available < reservedDiskSpace_)
		{
			LOG_TRACE("Available disk space is less then 10G, will remove oldest file");
			boost::posix_time::ptime nowTime(boost::posix_time::second_clock::local_time());
			boost::filesystem::directory_iterator dirIter(p), dirIterEnd;
			map<time_t, boost::filesystem::path> files;

			while ( dirIter != dirIterEnd )
			{
				if ( boost::filesystem::exists( *dirIter ) && !boost::filesystem::is_directory( *dirIter ) )
				{
					time_t t = boost::filesystem::last_write_time( *dirIter );
					auto p = dirIter->path();
					files.insert(make_pair(t, dirIter->path()));
				}
				++dirIter;
			}

			if (!files.empty())
			{
				LOG_TRACE("File was removed: " << files.begin()->second);
				boost::filesystem::remove(files.begin()->second);
				inRemovingState = true;
			}
		}
	}
}