#pragma once
#include <ppltasks.h>
#include <boost/signals2.hpp>
#include <boost/bind.hpp>
#include "VosVideo.Communication/CommunicationManager.h"
#include "DeviceConfigurationRequest.h"
#include "DeviceConfigurationResponse.h"


namespace vosvideo
{
	namespace devicemanagement
	{
		enum class DeviceConfigurationFlag
		{
			REMOVED,
			ADDED,
			UPDATED,
			NOCHANGE
		};

		class DeviceConfigurationManager : public vosvideo::communication::MessageReceiver
		{
		public:
			DeviceConfigurationManager(std::shared_ptr<vosvideo::communication::CommunicationManager> communicationManager, 
								   	   std::shared_ptr<vosvideo::communication::PubSubService> pubsubService, 
									   const std::wstring& accountId, 
									   const std::wstring& siteId);
			virtual ~DeviceConfigurationManager();

			virtual void OnMessageReceived(std::shared_ptr<vosvideo::data::ReceivedData> receivedMessage) override;
			concurrency::task<web::json::value> RequestDeviceConfigurationAsync();
			void RunDeviceDiscoveryAsync(std::shared_ptr<vosvideo::data::ReceivedData> msg);

			// Signals
			boost::signals2::connection ConnectToDeviceUpdateSignal(boost::signals2::signal<void (web::json::value& confs)>::slot_function_type subscriber);
//			boost::signals2::connection ConnectToDeviceStartTestSignal(boost::signals2::signal<void (web::json::value& confs)>::slot_function_type subscriber);
//			boost::signals2::connection ConnectToDeviceStopTestSignal(boost::signals2::signal<void (web::json::value& confs)>::slot_function_type subscriber);

		protected:
			void ParseDeviceConfiguration(web::json::value& resp);

		private:
			// Updates real camera configuration
			boost::signals2::signal<void (web::json::value& confs)> deviceUpdateSignal_;

			std::shared_ptr<vosvideo::communication::PubSubService> pubSubService_;
			std::shared_ptr<vosvideo::communication::CommunicationManager> communicationManager_;
			bool devReqInProgress_ = false;
			bool devDiscoveryInProgress_ = false;
			DeviceConfigurationResponse devConfResponse_;
			std::wstring accountId_;
			std::wstring siteId_;
		};
	}
}
