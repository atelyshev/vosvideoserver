#include "stdafx.h"
#include "ArchiveCatalogRequestMsg.h"

using namespace std;
using namespace vosvideo::data;

ArchiveCatalogRequestMsg::ArchiveCatalogRequestMsg()
{
}


ArchiveCatalogRequestMsg::~ArchiveCatalogRequestMsg()
{
}

web::json::value ArchiveCatalogRequestMsg::ToJsonValue() const
{
	web::json::value jObj;
	return jObj;
}

void ArchiveCatalogRequestMsg::FromJsonValue(const web::json::value& obj )
{
}

std::wstring ArchiveCatalogRequestMsg::ToString() const
{
	return L"";
}
