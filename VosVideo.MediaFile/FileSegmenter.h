#include <string>
#include <cpprest/json.h>
#include "MediaFileManifest.h"

namespace vosvideo
{
	namespace mediafile
	{
		class FileSegmenter
		{
		public:
			FileSegmenter(const std::wstring& path){}
			virtual ~FileSegmenter(){}

			virtual std::shared_ptr<MediaFileManifest> GetManifest() = 0;
		};
	}
}
