// deviceworker.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <ostream>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/thread/thread.hpp> 
#include "DeviceWorkerApp.h"

using namespace std;

static wstring deviceId_ = L"-deviceid";
static wstring logging_ = L"-logging";
static wstring debug_ = L"-debug";


int _tmain(int argc, _TCHAR* argv[])
{
	vector<wstring> argVect;

	for (int i = 0; i < argc; i++)
	{
		argVect.push_back(argv[i]);
	}

	wstring wqueueName;
	bool isLogging = false;

	for (uint32_t i = 0; i < argVect.size(); i++)
	{
		if (argVect[i].substr(0, deviceId_.length()) == deviceId_)
		{
			wqueueName = argVect[i].substr(deviceId_.length() + 1, argVect[i].length());
		}
		else if (argVect[i].substr(0, logging_.length()) == logging_)
		{
			(argVect[i].substr(logging_.length() + 1, argVect[i].length()) == L"true") ?  isLogging = true : isLogging = false;
		}
	}

	if (wqueueName.length() > 0)
	{
		DeviceWorkerApp app(wqueueName, isLogging);
		app.Start();
	}

	return 0;
}

