#pragma once

#include <webrtc/base/sigslot.h>
#include <webrtc/base/stringencode.h>
#include <webrtc/media/base/device.h>
#include <webrtc/media/base/videocapturer.h>
#include <agents.h>
#include "VosVideo.Configuration/ConfigurationManager.h"
#include "VosVideo.DeviceManagement/DeviceConfigurationManager.h"
#include "VosVideo.UserManagement/UserManager.h"
#include "VosVideo.Data/CameraConfMsg.h"
#include "VosVideo.Communication/CommunicationManager.h"
#include "VosVideo.CameraPlayer/CameraPlayerBase.h"
#include "CameraPlayerProcess.h"
#include "CameraException.h"

namespace vosvideo
{
	namespace camera
	{
		class CameraDeviceManager : /*public cricket::DeviceManager,  */public vosvideo::communication::MessageReceiver
		{
		public:
			CameraDeviceManager();
			CameraDeviceManager(std::shared_ptr<vosvideo::communication::CommunicationManager> commManager,
								std::shared_ptr<vosvideo::devicemanagement::DeviceConfigurationManager> devConfMgr,
								std::shared_ptr<vosvideo::communication::PubSubService> pubsubService, 
							    std::shared_ptr<vosvideo::usermanagement::UserManager> userMgr,
							    std::shared_ptr<vosvideo::configuration::ConfigurationManager> configMgr);

			virtual ~CameraDeviceManager();

			// Initialization
			virtual bool Init();
			virtual void Shutdown();
			virtual void Terminate();

			// Check if device with given number available
			virtual bool IsVideoCaptureDeviceExists(int devId);
			virtual bool IsVideoCaptureDeviceReady(int devId, std::shared_ptr<vosvideo::data::SendData>& lastErrMsg);
			// Check if device with given number exists and returns WebRTC deice representation
			virtual bool GetVideoCaptureDevice(int devId, cricket::Device& dev);
			virtual bool GetVideoCaptureDevices(std::vector<cricket::Device>* devs);
			virtual cricket::VideoCapturer* CreateVideoCapturer(const cricket::Device& device) const;
			// This method adds camera into manager and immediately starts it according passed configuration
			// Throws exception in case problem occurred
			void AddIpCam(web::json::value& camParms);
//			void RemoveIpCam(int camId);

			virtual void OnMessageReceived(std::shared_ptr<vosvideo::data::ReceivedData> receivedMessage);

			// It knows how to find device Id from Json 
			static void GetDeviceIdFromJson(int& camId, const web::json::value& camParms);

		private:
			// This map contains collection of running topologies. Topology is always up, 
			// but WebRTC can "take attention" of topology passing callback
			using CameraPlayersMap = std::unordered_map<int, vosvideo::cameraplayer::CameraPlayerBase* >;
			using CameraPlayerProcessMap = std::unordered_map<int, std::shared_ptr<CameraPlayerProcess> >;
			using CameraConfsMap = std::unordered_map<int, std::pair<vosvideo::data::CameraConfMsg, vosvideo::devicemanagement::DeviceConfigurationFlag>>;

			std::shared_ptr<vosvideo::communication::CommunicationManager> commManager_;
			std::shared_ptr<vosvideo::devicemanagement::DeviceConfigurationManager> devConfMgr_;
			std::shared_ptr<vosvideo::usermanagement::UserManager> userMgr_;
			std::shared_ptr<vosvideo::configuration::ConfigurationManager> configMgr_;
			std::shared_ptr<vosvideo::communication::PubSubService> pubSubService_;

			virtual bool GetAudioDevices(bool input, std::vector<cricket::Device>* devs);
//			virtual bool GetDefaultVideoCaptureDevice(cricket::Device* device);
			// Shortcut for real notification
			void NotifyAllUsers(const vosvideo::data::CameraConfMsg& conf, const CameraException& e);

			// Signal reactions
			void OnCameraUpdate(web::json::value& resp);
//			void OnCameraStartTest(web::json::value& resp);
//			void OnCameraStopTest(web::json::value& resp);

			void DeletePlayerProcess(int devId);
			void CreatePlayerProcess(vosvideo::data::CameraConfMsg& conf);
			void CreateCameraConfFromJson(int& camId, vosvideo::data::CameraConfMsg& conf, const web::json::value& camParms);
			// Try to recreate camera id it has status stopped. It gives us chance dynamically add-remove cameras
			void ReconnectCamera();
			void PassMessage(web::json::value& json, const std::wstring& payload );
			Concurrency::timer<CameraDeviceManager*>* reconnectTimer_ = nullptr; 
			CameraPlayersMap cameraPlayers_;
			CameraPlayerProcessMap cameraProcess_;
			CameraConfsMap cameraConfs_;
			std::mutex mutex_;
			const static int reconnectTimeout_ = 60000; // 1 min
		};
	}
}
