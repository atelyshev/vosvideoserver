#pragma once
#include <cpprest/json.h>

namespace vosvideo
{
	namespace data
	{
		class MediaInfo
		{
		public:
			MediaInfo(){}
			virtual ~MediaInfo(){}

			virtual web::json::value GetMediaInfo() = 0;
		};
	}
}
