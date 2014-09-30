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

			virtual void GetMediaInfo(web::json::value& mi) = 0;
		};
	}
}
