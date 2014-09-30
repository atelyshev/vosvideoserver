#pragma once
#include <cpprest/http_client.h>
#include "VosVideo.Communication/HttpClientEngine.h"

namespace vosvideo
{
	namespace communication
	{
		namespace casablanca
		{
			class CbHttpClientEngine :
				public vosvideo::communication::HttpClientEngine
			{
			public:
				CbHttpClientEngine(std::wstring& uri);
				~CbHttpClientEngine(void);
				virtual concurrency::task<web::json::value> Get(const std::wstring& path);
				virtual concurrency::task<web::json::value> Post(const std::wstring& path, const web::json::value& payload);
			private:
				web::json::value ExecuteRequest(pplx::task<web::http::http_response>& requestTask);

				web::http::client::http_client httpClient_;
			};
		}
	}
}

