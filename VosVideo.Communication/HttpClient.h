#pragma once
#include <memory>
#include <ppltasks.h>
#include "HttpClientEngine.h"

namespace vosvideo
{
	namespace communication
	{
		class HttpClient final
		{
			public:
				HttpClient(std::shared_ptr<HttpClientEngine> engine);
				~HttpClient(void);
				concurrency::task<web::json::value> Get(const std::wstring& path);
				concurrency::task<web::json::value> Post(const std::wstring& path, const web::json::value& payload);
				boost::signals2::connection ConnectToConnectionProblemSignal(boost::signals2::signal<void()>::slot_function_type subscriber);

		private:
				std::shared_ptr<HttpClientEngine> engine_;
		};
	}
}


