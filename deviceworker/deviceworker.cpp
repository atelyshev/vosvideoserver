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

void WaitForDebugger()
{
#ifdef _DEBUG
	bool isDone = false;
	cout << "Start debug waiting loop." << endl;

	while(!isDone)
	{
		boost::this_thread::sleep(boost::posix_time::milliseconds(1000));
		cout << "In waiting loop" << endl;
	}
#endif
}

int _tmain(int argc, wchar_t* argv[])
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
			(argVect[i].substr(logging_.length(), argVect[i].length()) == L"true") ?  isLogging = true : isLogging = false;
		}
		else if (argVect[i].substr(0, debug_.length()) == debug_)
		{
			WaitForDebugger();
		}
	}

	if (wqueueName.length() > 0)
	{
		DeviceWorkerApp app(wqueueName, isLogging);
		app.Start();
	}

	return 0;
}

