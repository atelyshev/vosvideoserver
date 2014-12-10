#pragma once
#include "ReceivedData.h"

namespace vosvideo
{
	namespace data
	{
		class ArchiveCatalogRequestMsg final : public ReceivedData
		{
		public:
			ArchiveCatalogRequestMsg();
			virtual ~ArchiveCatalogRequestMsg();

			virtual void ToJsonValue(web::json::value& obj) const;
			virtual void FromJsonValue(web::json::value& obj);
			virtual std::wstring ToString() const;
		};
	}
}
