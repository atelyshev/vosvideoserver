#pragma once

namespace vosvideo
{
	namespace data
	{
		enum class MsgType
		{
			EmptyMsg = 0,
			ConnectionOpenedMsg = 1,
			ConnectionClosedMsg,
			LiveVideoOfferMsg,
			LiveVideoPauseMsg,
			IceCandidateOfferMsg,
			DeviceConfigurationMsg,
			DeletePeerConnectionMsg,
			ArchiveCatalogRequestMsg,
			DeviceDiscoveryMsg,
			CameraConfMsg,
			ShutdownCameraProcessRequestMsg,
			SdpAnswerMsg = 101,
			IceCandidateAnswerMsg,
			LiveVideoErrorMsg,
			ArchiveCatalogAnswerMsg,
			RtbcInfoMsg,
			RtbcDeviceErrorOutMsg,
			DeviceDiscoveryOutMsg
		};
	}
}