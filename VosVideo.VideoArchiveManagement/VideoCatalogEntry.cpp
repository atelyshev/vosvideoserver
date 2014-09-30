#include "stdafx.h"
#include "VideoCatalogEntry.h"

using namespace std;
using namespace vosvideo::archive;

VideoCatalogEntry::VideoCatalogEntry(const wstring& filePath, VideoCatalogEntryTypes entryType) : 
	filePath_(filePath),
	entryType_(entryType)
{
}

VideoCatalogEntry::~VideoCatalogEntry()
{
}

uint32_t VideoCatalogEntry::GetLength()
{
	return length_;
}
