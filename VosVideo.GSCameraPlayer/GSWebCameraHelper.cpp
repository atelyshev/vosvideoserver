#include "stdafx.h"
#include <opencv/cv.h>
#include <opencv/highgui.h>
#include "GSWebCameraHelper.h"

using vosvideo::cameraplayer::GSWebCameraHelper;

void GSWebCameraHelper::CreateVideoCaptureDevices(GSWebCameraHelper::WebCamsList& webCams)
{
	WebCameraDescription description = WebCameraDescription();
	CvCapture* capture = cvCaptureFromCAM(CV_CAP_ANY);

	if (capture != nullptr)
	{
		description.FriendlyName = L"Web Camera";
		description.SymLink = L"webcamera";
		webCams.push_back(description);
	}
	cvReleaseCapture(&capture);
}