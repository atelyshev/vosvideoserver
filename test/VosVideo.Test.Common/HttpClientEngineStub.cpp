#include "stdafx.h"
#include "HttpClientEngineStub.h"


HttpClientEngineStub::HttpClientEngineStub(void)
{
	
}


HttpClientEngineStub::~HttpClientEngineStub(void)
{
	std::cout << "HttpClientEngineStub removed";
}

concurrency::task<web::json::value> HttpClientEngineStub::Get(const std::wstring& path)
{
	//httpClient_.request(methods::GET, uri_builder(U("/search")).append_query(U("q"), "blabla").to_string());
	concurrency::task<web::json::value> t([]() 
											{ 
												web::json::value jsonVal;
												return jsonVal; 
											} 
										 );
	return t;
}


concurrency::task<web::json::value> HttpClientEngineStub::Post(const std::wstring& path, const web::json::value& payload)
{
	concurrency::task<web::json::value> postTask
		(
		[&, path, payload]() 
				{ 
					web::json::value jsonVal;
					return jsonVal; 
				} 
			);
	return postTask;
}
