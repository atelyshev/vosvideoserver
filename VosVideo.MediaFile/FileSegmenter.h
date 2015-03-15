#include <string>
#include <cpprest/json.h>

namespace vosvideo
{
	namespace mediafile
	{
		class FileSegmenter 
		{
		public:
			FileSegmenter(const std::wstring& path);
			virtual ~FileSegmenter();

			virtual void GetManifest(const std::wstring&) = 0;
			virtual void GetManifest(const web::json::value&) = 0;
		};
	}
}
