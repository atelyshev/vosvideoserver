#pragma once

#include <cpprest/json.h>

namespace vosvideo
{
	namespace data
	{
		class JsonObjectBase
		{
		public:
			virtual ~JsonObjectBase(){};
			virtual web::json::value ToJsonValue() const = 0;
			virtual void FromJsonValue(const web::json::value& obj) = 0;
			virtual std::wstring ToString() const = 0;
		};
	}
}

