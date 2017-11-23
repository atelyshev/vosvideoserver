// VosVideo.Camera.Test.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "VosVideo.DeviceManagement/DeviceConfigurationManager.h"
#include "VosVideo.Communication/CommunicationManager.h"
#include "VosVideo.Configuration/ConfigurationManager.h"
#include "VosVideo.Test.Common/HttpClientEngineStub.h"
#include "VosVideo.Test.Common/WebsocketClientEngineStub.h"

using namespace std;
using namespace vosvideo::communication;
using namespace vosvideo::configuration;
using namespace vosvideo::devicemanagement;

std::wstring camJson = L"{\"DeviceList\":[{\"AccountId\":2,\"DeviceId\":8,\"DeviceName\":\"test\",\"IpAddress\":\"0.0.0.0\",\"DeviceUserName\":\"testuser\",\"DevicePassword\":\"testpass\",\"Port\":80,\"RecordingMode\":1,\"IsActive\":true,\"UriType\":0,\"CustomUri\":\"\",\"ManufacturerId\":132,\"ManufacturerName\":\"Loftek\",\"SiteId\":1,\"SiteName\":\"SITE1\",\"ModelId\":963,\"ModelName\":\"CXS 2200\",\"MjpegUri\":\"/videostream.cgi?resolution=32\",\"HttpPort\":80,\"RtspPort\":554}]}";

class TestDeviceConfigurationManager : public DeviceConfigurationManager
{
public:
	TestDeviceConfigurationManager(std::shared_ptr<vosvideo::communication::CommunicationManager> communicationManager, 
								   std::shared_ptr<vosvideo::communication::PubSubService> pubsubService, 
								   const std::wstring& accountId,
								   const std::wstring& siteId) : 
	DeviceConfigurationManager(communicationManager, pubsubService, accountId, siteId)
	{
		utility::stringstream_t ss;
		ss << camJson;
		camResp_ = web::json::value::parse(ss);
	}

	void TestParseDeviceConfiguration()
	{
		ParseDeviceConfiguration(camResp_);
	}

	int TestGetCameraConfigurations()
	{
		return camResp_.size();
	}

	protected:
		web::json::value camResp_;
};



TEST(VosVideoDeviceManager, CreateDeviceManager)
{
	std::shared_ptr<vosvideo::communication::CommunicationManager> communicationManager;
	std::shared_ptr<PubSubService> communicationPubSub(new PubSubService());

	wstring accountId = L"1";
	wstring siteId = L"1";
	TestDeviceConfigurationManager tdm(communicationManager, communicationPubSub, accountId, siteId);
	tdm.TestParseDeviceConfiguration();
	EXPECT_EQ(1, tdm.TestGetCameraConfigurations());
}

int main(int argc, char* argv[])
{
	testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}

