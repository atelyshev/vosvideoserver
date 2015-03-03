#include "stdafx.h"
#include <windows.h>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/filesystem.hpp>
#include <vosvideocommon/StringUtil.h>
#include "VosVideo.Communication/TypeInfoWrapper.h"
#include "VosVideo.Data/ArchiveCatalogRequestMsg.h"
#include "DirectoryChangesNotifier.h"
#include "ArchiveManager.h"

using namespace std;
using namespace util;
using namespace vosvideo::communication;
using namespace vosvideo::archive;
using namespace vosvideo::data;
using namespace boost::filesystem;

ArchiveManager::ArchiveManager(std::shared_ptr<vosvideo::configuration::ConfigurationManager> configManager, 
							   shared_ptr<vosvideo::communication::PubSubService> pubsubService) :
	configManager_(configManager),
	pubSubService_(pubsubService)
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
	changesNotifier_.reset(new DirectoryChangesNotifier(configManager_));
	changesNotifier_->ConnectToChangesSignal(boost::bind(&ArchiveManager::OnArchiveChanged, this, _1));
	ReadVideoCatalogAsync(configManager_->GetArchivePath());
}

ArchiveManager::~ArchiveManager()
{
}

pplx::task<void> ArchiveManager::ReadVideoCatalogAsync(const wstring& path)
{
	wstring ext = L".webm";

	return Concurrency::create_task([=]
	{
		if (exists(path) && is_directory(path))
		{
			directory_iterator end_iter;

			for (directory_iterator dir_iter(path); dir_iter != end_iter; ++dir_iter)
			{
				if (is_regular_file(dir_iter->status()))
				{
					if (dir_iter->path().extension().generic_wstring() == ext)
					{						
						auto videoFile = videoDiscoverer_.Discover(dir_iter->path().generic_wstring());
						AddToCatalog(videoFile);
					}
				}
			}
		}
	});
}

void ArchiveManager::AddToCatalog(shared_ptr<VideoFile> videoFile)
{
	auto entry = videoCatalog_.find(videoFile->GetId());

	if (entry != videoCatalog_.end())
	{
		entry->second.insert(make_pair(videoFile->StartTime(), videoFile));
	}
	else
	{
		VideoEntriesMap sameIdMap;
		sameIdMap.insert(make_pair(videoFile->StartTime(), videoFile));
		videoCatalog_.insert(make_pair(videoFile->GetId(), sameIdMap));
	}
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

void ArchiveManager::OnArchiveChanged(const wstring& path)
{
	AddToCatalog(videoDiscoverer_.Discover(path));
	LOG_TRACE("Archive was changed, new file added " << path);
}
