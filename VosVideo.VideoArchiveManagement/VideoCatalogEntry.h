#pragma once
#include "VideoFile.h"

namespace vosvideo
{
	namespace archive
	{
		enum class VideoCatalogEntryTypes
		{
			LocalDrive,
			SkyDrive,
			GoogleDrive,
			DropBox,
			MegaDrive
		};

		class VideoCatalogEntry
		{
		public:
			VideoCatalogEntry(const std::wstring& originalName, VideoCatalogEntryTypes entryType);
			virtual ~VideoCatalogEntry();

			uint32_t GetLength();

		private:
			uint32_t length_;
			std::wstring filePath_;
			VideoCatalogEntryTypes entryType_;
			std::shared_ptr<VideoFile> videoFile_;
		};
	}
}