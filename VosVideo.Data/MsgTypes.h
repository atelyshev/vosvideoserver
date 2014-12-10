#pragma once

namespace vosvideo
{
	namespace data
	{
		enum class MsgType
		{
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
			ShutdownCameraProcessMsg,
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