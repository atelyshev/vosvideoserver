#pragma once
// libwebm parser includes
#include <libwebm/mkvreader.hpp>
#include "FileSegmenter.h"

namespace vosvideo
{
	namespace mediafile
	{
		class WebmSegmenter : public FileSegmenter
		{
		public:
			WebmSegmenter(const std::wstring& path);
			virtual ~WebmSegmenter();

			std::shared_ptr<MediaFileManifest> GetManifest();

		private:
			mkvparser::MkvReader reader_;
			std::shared_ptr<MediaFileManifest> manifest_;
		};
	}
}
