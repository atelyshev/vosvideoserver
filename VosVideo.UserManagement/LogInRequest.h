#pragma once
#include "VosVideo.Data/JsonObjectBase.h"

namespace vosvideo
{
	namespace usermanagement
	{
		class LogInRequest final : public vosvideo::data::JsonObjectBase
		{
		public:
			LogInRequest();
			LogInRequest(const std::wstring& username, const std::wstring& password);
			~LogInRequest();			
			virtual void ToJsonValue(web::json::value& obj) const;
			virtual void FromJsonValue(web::json::value& obj);
			virtual std::wstring ToString() const;

			std::wstring const& GetUserName() const;
		private:
			std::wstring username_;
			std::wstring password_;
		};
	}
}


