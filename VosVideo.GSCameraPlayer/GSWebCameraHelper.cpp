#include "stdafx.h"
#include "GSWebCameraHelper.h"

using vosvideo::cameraplayer::GSWebCameraHelper;

GSWebCameraHelper::GSWebCameraHelper(){
}

GSWebCameraHelper::~GSWebCameraHelper(){
}

HRESULT GSWebCameraHelper::CreateVideoCaptureDevices(GSWebCameraHelper::WebCamsList& webCams){
	WebCameraDescription description = WebCameraDescription();
	description.FriendlyName = L"Web Camera";
	description.SymLink = L"webcamera";
	webCams.push_back(description);
	return S_OK;
}