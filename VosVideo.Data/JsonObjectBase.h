#pragma once

#include <cpprest/json.h>

namespace vosvideo
{
	namespace data
	{
		class JsonObjectBase
		{
		public:
			JsonObjectBase(){};
			virtual ~JsonObjectBase(){};
			virtual void ToJsonValue(web::json::value& obj) const = 0;
			virtual void FromJsonValue(web::json::value& obj) = 0;
			virtual std::wstring ToString() const = 0;
		};
	}
}

