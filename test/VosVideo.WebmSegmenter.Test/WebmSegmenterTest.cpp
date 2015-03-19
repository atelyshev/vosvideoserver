// VosVideo.WebmSegmenter.Test.cpp : Defines the entry point for the console application.
//
#include "stdafx.h"
#include <winerror.h>
#include <gtest/gtest.h>
#include "VosVideo.MediaFile/WebmSegmenter.h"

using namespace std;
using namespace vosvideo::mediafile;

TEST(WebmSegmenterInit, WebmSegmenterTest)
{
	WebmSegmenter wmseg(L"c:\\temp\\small.webm");
	shared_ptr<MediaFileManifest> manifest = wmseg.GetManifest();
	auto jsonObj = manifest->ToJsonObject();
}

int _tmain(int argc, _TCHAR* argv[])
{
	testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}

