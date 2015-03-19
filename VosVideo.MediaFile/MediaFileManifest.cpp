#include "stdafx.h"
#include "MediaFileManifest.h"

using namespace std;
using namespace vosvideo::mediafile;

MediaFileManifest::MediaFileManifest(int64_t totalSize, int64_t duration_ns, const std::wstring& filename) :
totalSize_(totalSize), duration_ns_(duration_ns), filename_(filename)
{
}

MediaFileManifest::~MediaFileManifest()
{
}

void MediaFileManifest::AddMediaCluster(const MediaCluster& cluster)
{
	clusters_.push_back(cluster);
}

std::wstring MediaFileManifest::ToJsonString()
{
	return L"";
}

web::json::value MediaFileManifest::ToJsonObject()
{
	web::json::value manifestObj;
	manifestObj[L"total_size"] = web::json::value::number((double)totalSize_);
	manifestObj[L"duration"] = web::json::value::number((double)duration_ns_);
	manifestObj[L"filename"] = web::json::value::string(filename_);
	vector<web::json::value> clustArr;

	for (auto iter = clusters_.begin(); iter != clusters_.end(); iter++)
	{
		int64_t timeCode, offset;
		iter->GetClusterData(timeCode, offset);
		web::json::value cluster;
		cluster[L"timecode"] = web::json::value::number((double)timeCode);
		cluster[L"offset"] = web::json::value::number((double)offset);
		clustArr.push_back(cluster);
	}

	manifestObj[L"clusters"] = web::json::value::array(clustArr);
	auto str = manifestObj.to_string();
	return manifestObj;
}
