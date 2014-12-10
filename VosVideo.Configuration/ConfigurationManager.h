#pragma once

namespace vosvideo
{
	namespace configuration
	{
		class ConfigurationManager final
		{
		public:
			ConfigurationManager(void);
			~ConfigurationManager(void);
			std::wstring GetRestServiceUri() const;
			std::wstring GetWebSiteUri() const;
			std::wstring GetWebsocketUri() const;
			std::wstring GetSiteId() const;
			std::wstring GetArchivePath() const;
			bool IsLoggerOn() const;

		private:
			const std::wstring FindConfValue(std::wstring) const;
			void GetConfigurationFilePath(std::wstring& confFilePath);

			static std::wstring configFileName_;
			std::unordered_map<std::wstring, std::wstring> keyValConf_;

			static std::wstring vosVideoWebUriKey_;
			static std::wstring websocketUriKey_;
			static std::wstring siteIdKey_;
			static std::wstring siteNameKey_;
			static std::wstring loggerKey_;
			static std::wstring archivePathKey_;
		};
	}
}


