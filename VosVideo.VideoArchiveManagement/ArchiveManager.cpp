#include "stdafx.h"
#include <windows.h>
#include <boost/date_time/posix_time/posix_time_types.hpp>
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
}

ArchiveManager::~ArchiveManager()
{
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
	LOG_TRACE("Archive was changed");
}
