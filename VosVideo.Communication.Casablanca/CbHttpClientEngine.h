#pragma once
#include <agents.h>
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
				bool Init();

				web::json::value ExecuteRequest(const std::wstring& url, pplx::task<web::http::http_response>& requestTask);
				Concurrency::timer<CbHttpClientEngine*>* sessionRefreshTimer_;
				web::http::client::http_client httpClient_;
				const uint32_t sessionRefreshTimeout_ = 60000; // 1Hr in milliseconds
			};
		}
	}
}

