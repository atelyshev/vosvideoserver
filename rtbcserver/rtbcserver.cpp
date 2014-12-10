// rtbcserver.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include "RtbcService.h"
#include "Application.h"

using namespace std;

int _tmain(int argc, _TCHAR* argv[])
{
	vector<wstring> argVect;

	for (int i = 0; i < argc; i++)
	{
		argVect.push_back(argv[i]);
	}

	Application app(argVect);

	if (app.Start())
	{
		// Have body only in case standalone app
		app.Wait();
	}

	return 0;
}

