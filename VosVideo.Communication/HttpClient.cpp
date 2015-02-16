#include "stdafx.h"
#include "HttpClient.h"

using vosvideo::communication::HttpClient;
using vosvideo::communication::HttpClientEngine;

HttpClient::HttpClient(std::shared_ptr<HttpClientEngine> engine) : engine_(engine)
{
}


HttpClient::~HttpClient()
{
}

boost::signals2::connection HttpClient::ConnectToConnectionProblemSignal(boost::signals2::signal<void()>::slot_function_type subscriber)
{
	return engine_->ConnectToConnectionProblemSignal(subscriber);
}

concurrency::task<web::json::value> HttpClient::Get(const std::wstring& path)
{
	return engine_->Get(path);
}

concurrency::task<web::json::value> HttpClient::Post(const std::wstring& path, const web::json::value& payload)
{
	return engine_->Post(path, payload);
}
