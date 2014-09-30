// VosVideo.Data.Test.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <winerror.h>
#include <cpprest/json.h>
#include "vosVideo.Data/RtbcDeviceErrorOutMsg.h"

using namespace std;
using namespace vosvideo::data;

void TestRtbcErrorOutMsg()
{
	wstring msg = L"test error";
	RtbcDeviceErrorOutMsg errMsg(1, msg, E_ABORT);
	wstring jsonStr;
	errMsg.GetAsJsonString(jsonStr);

	utility::stringstream_t ss;
	ss << jsonStr;
	web::json::value::parse(jsonStr);
}

TEST(ParseOutMsg, ParseRtbcErrorOutMsg)
{
	EXPECT_NO_THROW(TestRtbcErrorOutMsg());
}

int _tmain(int argc, _TCHAR* argv[])
{
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}

