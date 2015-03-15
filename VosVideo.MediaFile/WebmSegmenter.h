#pragma once
// libwebm parser includes
#include <libwebm/source/mkvreader.hpp>
#include "FileSegmenter.h"

namespace vosvideo
{
	namespace mediafile
	{
		class WebmSegmenter : public FileSegmenter
		{
		public:
			WebmSegmenter(const std::wstring& path);
			~WebmSegmenter();

			void GetManifest(const std::wstring&);
			void GetManifest(const web::json::value&);

		private:
			mkvparser::MkvReader reader;
		};
	}
}
