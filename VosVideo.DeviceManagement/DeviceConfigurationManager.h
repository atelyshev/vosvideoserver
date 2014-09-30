#pragma once
#include <ppltasks.h>
#include <boost/signal.hpp>
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
			~DeviceConfigurationManager();

			virtual void OnMessageReceived(const std::shared_ptr<vosvideo::data::ReceivedData> receivedMessage);
			concurrency::task<web::json::value> RequestDeviceConfigurationAsync();
			void RunDeviceDiscoveryAsync(const std::shared_ptr<vosvideo::data::ReceivedData> msg);

			// Signals
			boost::signals::connection ConnectToDeviceUpdateSignal(boost::signal<void (web::json::value& confs)>::slot_function_type subscriber);
			boost::signals::connection ConnectToDeviceStartTestSignal(boost::signal<void (web::json::value& confs)>::slot_function_type subscriber);
			boost::signals::connection ConnectToDeviceStopTestSignal(boost::signal<void (web::json::value& confs)>::slot_function_type subscriber);

		protected:
			void ParseDeviceConfiguration(web::json::value& resp);

		private:
			// Updates real camera configuration
			boost::signal<void (web::json::value& confs)> deviceUpdateSignal_;

			std::shared_ptr<vosvideo::communication::PubSubService> pubSubService_;
			std::shared_ptr<vosvideo::communication::CommunicationManager> communicationManager_;
			bool devReqInProgress_;
			bool devDiscoveryInProgress_;
			DeviceConfigurationResponse devConfResponse_;
			std::wstring accountId_;
			std::wstring siteId_;
		};
	}
}
