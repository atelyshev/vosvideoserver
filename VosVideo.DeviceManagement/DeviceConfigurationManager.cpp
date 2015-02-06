#include "stdafx.h"
#include <boost/format.hpp>
#include <vosvideocommon/SeverityLoggerMacros.h>
#include "VosVideo.Data/DeviceConfigurationMsg.h"
#include "VosVideo.Data/DeviceDiscoveryRequestMsg.h"
#include "VosVideo.Data/DeviceDiscoveryResponseMsg.h"
#include "VosVideo.Data/CameraConfMsg.h"
#include "VosVideo.Communication/TypeInfoWrapper.h"
#include "VosVideo.CameraPlayer/WebCameraHelperBase.h"
#include "VosVideo.GSCameraPlayer/GSWebCameraHelper.h"
#include "DeviceConfigurationManager.h"

using namespace std;
using boost::wformat;
using boost::io::group;
using namespace vosvideo::data;
using namespace vosvideo::cameraplayer;
using namespace vosvideo::communication;
using namespace vosvideo::devicemanagement;

DeviceConfigurationManager::DeviceConfigurationManager(shared_ptr<CommunicationManager> communicationManager, 
													   shared_ptr<PubSubService> pubSubService,
													   const std::wstring& accountId, 
													   const std::wstring& siteId)
													   : communicationManager_(communicationManager), 
													   pubSubService_(pubSubService),
													   accountId_(accountId),
													   siteId_(siteId),
													   devReqInProgress_(false),
													   devDiscoveryInProgress_(false)
{
	// Subscribe for update requests
	vector<TypeInfoWrapper> interestedTypes;
	TypeInfoWrapper typeInfo = typeid(DeviceConfigurationMsg);
	interestedTypes.push_back(typeInfo);

	typeInfo = typeid(DeviceDiscoveryRequestMsg);
	interestedTypes.push_back(typeInfo);
	pubSubService_->Subscribe(interestedTypes, *this);
}


DeviceConfigurationManager::~DeviceConfigurationManager()
{
}

void DeviceConfigurationManager::OnMessageReceived(const shared_ptr<vosvideo::data::ReceivedData> receivedMessage)
{
	if (dynamic_pointer_cast<DeviceConfigurationMsg>(receivedMessage))
	{
		RequestDeviceConfigurationAsync();
	}
	else if (dynamic_pointer_cast<DeviceDiscoveryRequestMsg>(receivedMessage))
	{
		RunDeviceDiscoveryAsync(receivedMessage);
	}
}

boost::signals2::connection DeviceConfigurationManager::ConnectToDeviceUpdateSignal(boost::signals2::signal<void (web::json::value& confs)>::slot_function_type subscriber)
{
	return deviceUpdateSignal_.connect(subscriber);
}

// Runs local and network discovery
// Mostly interested in IP camera and Web Cam
void DeviceConfigurationManager::RunDeviceDiscoveryAsync(const shared_ptr<vosvideo::data::ReceivedData> msg)
{
	if(devDiscoveryInProgress_) 
	{
		throw std::runtime_error("Device discovery in progress please wait unit it is done");
	}

	devDiscoveryInProgress_ = true;

	wstring srvPeer;
	wstring clientPeer;
	msg->GetFromPeer(clientPeer);
	msg->GetToPeer(srvPeer);
	concurrency::task<void> devDiscoveryTask
	(
		[this, clientPeer, srvPeer]() 
		{ 
			GSWebCameraHelper wph;
			WebCameraHelperBase::WebCamsList wcl;
			wph.CreateVideoCaptureDevices(wcl);
			web::json::value jobjVect;
			int i = 0;
			for (WebCameraHelperBase::WebCamsList::const_iterator iter = wcl.begin(); iter != wcl.end(); ++iter)
			{
				web::json::value jObj;
				jObj[L"modeltype"] = web::json::value::number(static_cast<int>(CameraType::WEBCAM));
				jObj[L"friendlyname"] = web::json::value::string(iter->FriendlyName);
				jObj[L"url"] = web::json::value::string(iter->SymLink);
				jobjVect[i++] = jObj;
			}

			string respRtbc;
			shared_ptr<DeviceDiscoveryResponseMsg> devInfo(new DeviceDiscoveryResponseMsg(jobjVect));
			CommunicationManager::CreateWebsocketMessageString(srvPeer, clientPeer, devInfo, respRtbc);
			communicationManager_->WebsocketSend(respRtbc);
			this->devDiscoveryInProgress_ = false;
		} 
	);
}

concurrency::task<web::json::value> DeviceConfigurationManager::RequestDeviceConfigurationAsync()
{
	if(devReqInProgress_) 
	{
		throw std::runtime_error("Device configuration in progress please wait unit it is done");
	}

	devReqInProgress_ = true;
	LOG_TRACE("Device configuration message received. Updating current configuration.");

	wstring camerasUri = str(wformat(L"/device?format=json&AccountId=%1%&SiteId=%2%") % accountId_ % siteId_);	
	auto devToHttpServerTask = communicationManager_->HttpGet(camerasUri);

	devToHttpServerTask.then([&](web::json::value& resp)
	{
		devReqInProgress_ = false;
		ParseDeviceConfiguration(resp);
	});
	return devToHttpServerTask;
}

void DeviceConfigurationManager::ParseDeviceConfiguration(web::json::value& resp)
{
	auto lst = resp.at(U("DeviceList"));
	deviceUpdateSignal_(lst);
}
