#pragma once
#include <cpprest/json.h>

namespace vosvideo
{
	namespace mediafile
	{
		class MediaCluster
		{
		public:
			MediaCluster(int64_t timeCode, int64_t offset) : timeCode_(timeCode), offset_(offset){}

			void GetClusterData(int64_t& timeCode, int64_t& offset) { timeCode = timeCode_; offset = offset_; }
		private:
			int64_t timeCode_;
			int64_t offset_;
		};

		class MediaFileManifest
		{
		public:
			MediaFileManifest(int64_t totalSize, int64_t duration, const std::wstring& path);
			~MediaFileManifest();

			void AddMediaCluster(const MediaCluster& cluster);
			std::wstring ToJsonString();
			web::json::value ToJsonObject();

		private:
			int64_t totalSize_;
			int64_t duration_ns_;
			std::wstring filename_;
			std::vector<MediaCluster> clusters_;
		};
	}
}
