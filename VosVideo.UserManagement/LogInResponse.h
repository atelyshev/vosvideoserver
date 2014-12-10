#pragma once
#include "VosVideo.Data/JsonObjectBase.h"
#include "VosVideo.Communication/Peer.h"


namespace vosvideo{
	namespace usermanagement{

		class LogInResponse final : public vosvideo::data::JsonObjectBase
		{
		public:
			LogInResponse();
			~LogInResponse();
			virtual void ToJsonValue(web::json::value& obj) const;
			virtual void FromJsonValue(web::json::value& obj);
			virtual std::wstring ToString() const;

			vosvideo::communication::Peer const& GetPeer() const;
			void SetPeer(vosvideo::communication::Peer& peer);

			friend std::wstring operator+(std::wstring const& leftStr, LogInResponse const& rightResp);
			std::wstring operator+(std::wstring const& str) const;		
		private:
			vosvideo::communication::Peer peer_;
		};
	}
}
