// rtbcserver.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include "RtbcService.h"
#include "Application.h"

using namespace std;
using namespace util;

int main(int argc, char* argv[])
{
	vector<wstring> argVect;

	for (int i = 0; i < argc; i++)
	{
		argVect.push_back(StringUtil::ToWstring(argv[i]));
	}

	Application app(argVect);

	if (app.Start())
	{
		// Have body only in case standalone app
		app.Wait();
	}

	return 0;
}

