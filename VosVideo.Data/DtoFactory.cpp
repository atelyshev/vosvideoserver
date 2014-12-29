#include "stdafx.h"
#include "DtoFactory.h"
#include "CameraConfMsg.h"
#include "WebsocketConnectionOpenedMsg.h"
#include "WebsocketConnectionClosedMsg.h"
#include "WebRtcIceCandidateMsg.h"
#include "DeviceConfigurationMsg.h"
#include "DeviceDiscoveryRequestMsg.h"
#include "LiveVideoOfferMsg.h"
#include "SdpAnswerMsg.h"
#include "IceCandidateResponseMsg.h"
#include "DeletePeerConnectionRequestMsg.h"
#include "ArchiveCatalogRequestMsg.h"
#include "ShutdownCameraProcessRequestMsg.h"

using namespace boost;
using namespace std;

namespace vosvideo
{
	namespace data
	{
		DtoFactory::DtoFactory()
		{
			// Add more factories into factory collection
			factories_[MsgType::ConnectionOpenedMsg] = boost::factory<WebsocketConnectionOpenedMsg*>();
			factories_[MsgType::ConnectionClosedMsg] = boost::factory<WebsocketConnectionClosedMsg*>();
			factories_[MsgType::LiveVideoOfferMsg] = boost::factory<LiveVideoOfferMsg*>();
			factories_[MsgType::IceCandidateOfferMsg] = boost::factory<WebRtcIceCandidateMsg*>();
			factories_[MsgType::DeviceConfigurationMsg] = boost::factory<DeviceConfigurationMsg*>();
			factories_[MsgType::DeletePeerConnectionMsg] = boost::factory<DeletePeerConnectionRequestMsg*>();
			factories_[MsgType::ArchiveCatalogRequestMsg] = boost::factory<ArchiveCatalogRequestMsg*>();			
			factories_[MsgType::DeviceDiscoveryMsg] = boost::factory<DeviceDiscoveryRequestMsg*>();
			factories_[MsgType::CameraConfMsg] =  boost::factory<CameraConfMsg*>();
			factories_[MsgType::SdpAnswerMsg] =  boost::factory<SdpAnswerMsg*>();			
			factories_[MsgType::IceCandidateAnswerMsg] =  boost::factory<IceCandidateResponseMsg*>();			
			factories_[MsgType::ShutdownCameraProcessRequestMsg] = boost::factory<ShutdownCameraProcessRequestMsg*>();
		}

		DtoFactory::~DtoFactory()
		{
		}

		std::shared_ptr<ReceivedData> DtoFactory::Create(MsgType dtoType)
		{
			return std::shared_ptr<ReceivedData>(factories_[dtoType]());
		}
	}
}