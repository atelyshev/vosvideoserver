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

		class MsgTypeAsText
		{
		public:
			std::string AsText(MsgType type)
			{
				auto it = _enumToText.find(type);
				return (it != _enumToText.end() ? it->second : "Error: Type not found");
			}

		private:
			const std::unordered_map<MsgType, std::string> _enumToText =
			{
				{ MsgType::EmptyMsg, "EmptyMsg" },
				{ MsgType::ConnectionOpenedMsg, "ConnectionOpenedMsg" },
				{ MsgType::ConnectionClosedMsg, "ConnectionClosedMsg" },
				{ MsgType::LiveVideoOfferMsg, "LiveVideoOfferMsg" },
				{ MsgType::LiveVideoPauseMsg, "LiveVideoPauseMsg" },
				{ MsgType::IceCandidateOfferMsg, "IceCandidateOfferMsg" },
				{ MsgType::DeviceConfigurationMsg, "DeviceConfigurationMsg" },
				{ MsgType::DeletePeerConnectionMsg, "DeletePeerConnectionMsg" },
				{ MsgType::ArchiveCatalogRequestMsg , "ArchiveCatalogRequestMsg "},
				{ MsgType::DeviceDiscoveryMsg, "DeviceDiscoveryMsg" },
				{ MsgType::CameraConfMsg, "CameraConfMsg" },
				{ MsgType::ShutdownCameraProcessRequestMsg, "ShutdownCameraProcessRequestMsg" },
				{ MsgType::SdpAnswerMsg, "SdpAnswerMsg" },
				{ MsgType::IceCandidateAnswerMsg, "IceCandidateAnswerMsg" },
				{ MsgType::LiveVideoErrorMsg, "LiveVideoErrorMsg" },
				{ MsgType::ArchiveCatalogAnswerMsg, "ArchiveCatalogAnswerMsg" },
				{ MsgType::RtbcInfoMsg, "RtbcInfoMsg" },
				{ MsgType::RtbcDeviceErrorOutMsg, "RtbcDeviceErrorOutMsg" },
				{ MsgType::DeviceDiscoveryOutMsg, "DeviceDiscoveryOutMsg" }
			};
		};
	}
}