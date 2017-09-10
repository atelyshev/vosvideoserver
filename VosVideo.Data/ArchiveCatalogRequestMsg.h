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

			virtual web::json::value ToJsonValue() const override;
			virtual void FromJsonValue(const web::json::value& obj) override;
			virtual std::wstring ToString() const override;
		};
	}
}
