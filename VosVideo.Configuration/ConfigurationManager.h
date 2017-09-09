#pragma once

namespace vosvideo
{
	namespace configuration
	{
		class ConfigurationManager final
		{
		public:
			ConfigurationManager();
			virtual ~ConfigurationManager();
			std::wstring GetRestServiceUri() const;
			std::wstring GetWebSiteUri() const;
			std::wstring GetWebsocketUri() const;
			std::wstring GetSiteId() const;
			std::wstring GetArchivePath() const;
			bool IsLoggerOn() const;

		private:
			std::wstring FindConfValue(const std::wstring& wKey) const;
			std::wstring GetConfigurationFilePath();

			std::unordered_map<std::wstring, std::wstring> keyValConf_;

			const std::wstring configFileName_ = L"rtbcserver.config.xml";
			const std::wstring webUriKey_ = L"VosVideoWebUri";
			const std::wstring restServiceUriKey_ = L"RestServiceUri";
			const std::wstring websocketUriKey_ = L"WebsocketUri";
			const std::wstring siteIdKey_ = L"SiteId";
			const std::wstring siteNameKey_ = L"SiteName";
			const std::wstring loggerKey_ = L"Logging";
			const std::wstring archivePathKey_ = L"ArchivePath";
			const std::wstring instDir_ = L"VosVideoServer";
		};
	}
}


