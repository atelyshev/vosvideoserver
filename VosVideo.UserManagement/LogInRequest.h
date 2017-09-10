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
			virtual ~LogInRequest();	

			virtual web::json::value ToJsonValue() const override;
			virtual void FromJsonValue(const web::json::value& obj) override;
			virtual std::wstring ToString() const override;

			std::wstring const& GetUserName() const;
		private:
			std::wstring username_;
			std::wstring password_;
		};
	}
}


