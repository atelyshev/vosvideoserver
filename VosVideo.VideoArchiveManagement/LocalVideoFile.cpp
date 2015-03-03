#include "stdafx.h"
#include "LocalVideoFile.h"

using namespace std;
using namespace vosvideo::archive;

LocalVideoFile::LocalVideoFile()
{
}

LocalVideoFile::LocalVideoFile(const std::wstring& id, const std::wstring& path, uint64_t duration, uint64_t startTime, bool isSeekable) :
	VideoFile(id, path, duration, startTime, isSeekable)
{
	fileType_ = VideoFileType::LocalDrive;
}

LocalVideoFile::~LocalVideoFile()
{
}
