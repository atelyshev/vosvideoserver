#include "stdafx.h"
#include <cpprest/http_client.h>
#include <cpprest/http_client.h>
#include <vosvideocommon/StringUtil.h>
#include <VosVideo.Communication/HttpClientException.h>
#include "CbHttpClientEngine.h"

using namespace std;
using namespace util;
using namespace web::http;
using namespace web::http::client;
using namespace vosvideo::communication;
using vosvideo::communication::casablanca::CbHttpClientEngine;

CbHttpClientEngine::CbHttpClientEngine(std::wstring& uri) : httpClient_(uri)
{
}


CbHttpClientEngine::~CbHttpClientEngine(void)
{
}

concurrency::task<web::json::value> CbHttpClientEngine::Get(const std::wstring& path)
{
	concurrency::task<web::json::value> getTask
		(
		[&, path]() 
					{ 
						uri_builder uriBuilder = uri_builder(path);
						utility::string_t pathAndQueryFragment = uriBuilder.to_string();
						auto requestTask = httpClient_.request(methods::GET, pathAndQueryFragment);
						return ExecuteRequest(pathAndQueryFragment, requestTask);
					} 
		);
	return getTask;
}


concurrency::task<web::json::value> CbHttpClientEngine::Post(const std::wstring& path, const web::json::value& payload)
{
	concurrency::task<web::json::value> postTask
		(
			[&, path, payload]() 
				{ 
					uri_builder uriBuilder = uri_builder(path);
					utility::string_t pathAndQueryFragment = uriBuilder.to_string();
					auto requestTask = httpClient_.request(methods::POST, pathAndQueryFragment, payload);
					return ExecuteRequest(pathAndQueryFragment, requestTask);
				} 
		);
	return postTask;
}

web::json::value CbHttpClientEngine::ExecuteRequest(const std::wstring& url, pplx::task<http_response>& requestTask)
{
	http_response resp = requestTask.get();
	web::json::value jsonVal;

	if (resp.status_code() != 200)
	{
		string str = StringUtil::ToString(resp.reason_phrase());
		LOG_ERROR(L"Execute Request: There was an error calling url: " + url);
		throw HttpClientException(util::StringUtil::ToString(url), str.c_str());
	}
	else
	{
		try
		{
			jsonVal = resp.extract_json().get();
		}
		catch (...){
			LOG_ERROR(L"Execute Request: There was an error extracting the response from: " + url);
			throw HttpClientException(util::StringUtil::ToString(url), "There was an error extracting the response");
		}	
	}

	return jsonVal;
}