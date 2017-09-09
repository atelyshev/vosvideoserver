// deviceworker.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <ostream>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/thread/thread.hpp> 
#include "VosVideo.Camera/CameraVideoCapturer.h"
#include "DeviceWorkerApp.h"

using namespace std;
using util::StringUtil;

static wstring deviceId_ = L"-deviceid";
static wstring logging_ = L"-logging";
static wstring debug_ = L"-debug";


int main(int argc, char* argv[])
{
	vector<wstring> argVect;

	for (int i = 0; i < argc; i++)
	{
		string tmp = argv[i];
		argVect.push_back(StringUtil::ToWstring(tmp));
	}

	wstring wqueueName;
	bool isLogging = false;

	for (const auto& arg : argVect)
	{
		if (arg.substr(0, deviceId_.length()) == deviceId_)
		{
			wqueueName = arg.substr(deviceId_.length() + 1, arg.length());
		}
		else if (arg.substr(0, logging_.length()) == logging_)
		{
			(arg.substr(logging_.length() + 1, arg.length()) == L"true") ?  isLogging = true : isLogging = false;
		}
	}

	if (wqueueName.length() > 0)
	{
		DeviceWorkerApp app(wqueueName, isLogging);
		app.Start();
	}

	return 0;
}

