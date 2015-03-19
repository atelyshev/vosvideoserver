#include "stdafx.h"
#include "VideoFileDiscoverer.h"

using namespace std;
using namespace vosvideo::archive;

VideoFileDiscoverer::VideoFileDiscoverer()
{
}

VideoFileDiscoverer::~VideoFileDiscoverer()
{
}

shared_ptr<VideoFile> VideoFileDiscoverer::Discover(const wstring& path)
{
	return localDiscoverer_.Discover(path);
}

vector<shared_ptr<VideoFile>> VideoFileDiscoverer::Discover(vector<wstring>& pathVect)
{
	return localDiscoverer_.Discover(pathVect);
}