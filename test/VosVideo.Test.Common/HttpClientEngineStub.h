#pragma once
#include <cpprest/json.h>
#include "VosVideo.Communication/HttpClientEngine.h"


using vosvideo::communication::HttpClientEngine;

class HttpClientEngineStub : public HttpClientEngine
{
public:
	HttpClientEngineStub(void);
	~HttpClientEngineStub(void);
	virtual concurrency::task<web::json::value> Get(const std::wstring& path);
	virtual concurrency::task<web::json::value> Post(const std::wstring& path, const web::json::value& payload);
};

