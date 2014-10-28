#include "stdafx.h"
#include <Shlobj.h>
#include <unordered_map>
#include <boost/format.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/property_tree/detail/xml_parser_error.hpp>
#include <boost/foreach.hpp>
#include <boost/filesystem.hpp>
#include <vosvideocommon/SeverityLoggerMacros.h>
#include <vosvideocommon/StringUtil.h>
#include "ConfigurationParserException.h"
#include "ConfigurationManager.h"

using namespace std;
using namespace util;
using namespace boost::property_tree::xml_parser;
using namespace boost::filesystem;
using boost::property_tree::ptree;
using vosvideo::configuration::ConfigurationManager;
using boost::format;

// This file generated at build process and located in RTBC local folder
wstring ConfigurationManager::configFileName_ = L"rtbcserver.config.xml";
wstring ConfigurationManager::vosVideoWebUriKey_ = L"VosVideoWebUri";
wstring ConfigurationManager::websocketUriKey_ = L"WebsocketUri";
wstring ConfigurationManager::siteIdKey_ = L"SiteId";
wstring ConfigurationManager::siteNameKey_ = L"SiteName";
wstring ConfigurationManager::loggerKey_ = L"Logging";
wstring ConfigurationManager::archivePathKey_ = L"ArchivePath";

ConfigurationManager::ConfigurationManager()
{
	ptree pt;
	string confFilePath;

	try
	{
		wstring wconfFilePath;
		GetConfigurationFilePath(wconfFilePath);
		confFilePath = StringUtil::ToString(wconfFilePath);

		LOG_TRACE("Open configuration file: " << confFilePath);
		read_xml(confFilePath, pt);
	}
	catch(xml_parser_error& err)
	{
		string errmsg = str(format("Failed to read configuration file: %1%. Exception message: %2%") % confFilePath % err.message());
		throw ConfigurationParserException(errmsg);
	}

	const ptree & formats = pt.get_child("configuration.appSettings");

	if (!pt.empty())
	{
		BOOST_FOREACH(const ptree::value_type & f, formats)
		{
			const ptree & attributes = f.second.get_child("<xmlattr>");
			vector <wstring> tmpPair;
			BOOST_FOREACH(const ptree::value_type &v, attributes)
			{
				wstring wString = StringUtil::ToWstring(v.second.data());
				tmpPair.push_back(wString);
				LOG_DEBUG("First: " << v.first.data() << " Second: " << v.second.data());
			}

			// Perform basic validation
			if (tmpPair.size() < 2)
			{
				//exception
				throw ConfigurationParserException("Configuration element is not complete. Check your configuration file.");
			}

			if (tmpPair[0] != vosVideoWebUriKey_ &&
				tmpPair[0] != websocketUriKey_ && 
				tmpPair[0] != siteIdKey_ && 
				tmpPair[0] != siteNameKey_ && 
				tmpPair[0] != loggerKey_ &&
				tmpPair[0] != archivePathKey_)
			{
				//exception
				throw ConfigurationParserException("Unknown key is found. Key is case sensitive. Check your configuration file.");
			}

			keyValConf_.insert(make_pair(tmpPair[0], tmpPair[1]));
			tmpPair.clear();
		}
	}
	LOG_DEBUG("Configuration successfully parsed.");
}


ConfigurationManager::~ConfigurationManager()
{
}

void ConfigurationManager::GetConfigurationFilePath(wstring& configFile)
{
#ifndef _DEBUG
	// Identify path to and read configuration then
	PWSTR progData;
	HRESULT hr = SHGetKnownFolderPath(FOLDERID_ProgramData, 0, NULL, &progData);
	if (hr != S_OK)
	{
		string errmsg = "Failed to get ProgramData folder path.";
		throw ConfigurationParserException(errmsg);
	}

	// Take current folder and attach to standard ProgramData folder
	// It is going to be configuration location for Release mode
	wstring currFolder = boost::filesystem::current_path().leaf().wstring();		
	path configFilePath = path(progData) / currFolder / configFileName_; // appends

	// RTBC only consumes that file, ask user to execute Dashboard first
	if (!exists(configFilePath))
	{
		string errmsg = "Server configuration folder doesn't exists in ProgramData. Start Vos Video Dashboard first to create configuration.";
		throw ConfigurationParserException(errmsg);
	}
	configFile = configFilePath.wstring();
#else
	configFile = configFileName_;
#endif
}

wstring ConfigurationManager::GetWebSiteUri() const
{
	return FindConfValue(vosVideoWebUriKey_);
}

wstring ConfigurationManager::GetWebsocketUri() const
{
	return FindConfValue(websocketUriKey_);
}

wstring ConfigurationManager::GetSiteId() const
{
	return FindConfValue(siteIdKey_);
}

wstring ConfigurationManager::GetArchivePath() const
{
	return FindConfValue(archivePathKey_);
}

bool ConfigurationManager::IsLoggerOn() const
{
	wstring wsVal = FindConfValue(loggerKey_);
	std::transform(wsVal.begin(), wsVal.end(), wsVal.begin(), ::tolower);
	return (wsVal == L"true");
}

const wstring ConfigurationManager::FindConfValue(wstring wKey) const
{
	unordered_map<wstring, wstring>::const_iterator iter = keyValConf_.find(wKey);

	if (iter != keyValConf_.end())
	{
		return iter->second;
	}
	return L"";
}

