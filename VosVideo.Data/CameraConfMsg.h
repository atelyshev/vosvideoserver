#pragma once

#include "ReceivedData.h"

namespace vosvideo
{
	namespace data
	{
		enum class CameraType
		{
			UNKNOWN = -1,
			IPCAM,
			WEBCAM
		};

		enum class CameraRecordingMode
		{
			DISABLED,
			PERMANENT,
			ONMOTION,
			ONSCHEDULER
		};

		class CameraConfMsg final : public ReceivedData
		{
		public:
			CameraConfMsg() {}
			CameraConfMsg(const std::wstring& jsonStr);
			CameraConfMsg(CameraType ct);
			virtual ~CameraConfMsg();

			static CameraConfMsg CreateFromDto(const std::wstring& archPath, const web::json::value& camParmsDto);
			CameraType GetCameraType();

			void SetCameraId(int cameraId);
			void SetCameraName(const std::wstring& cameraName);
			int GetCameraId() const;
			std::wstring GetCameraName() const;

			void SetFileSinkParameters(const std::wstring& outFolder, uint32_t recordLen, uint32_t maxFilesNum, CameraRecordingMode recordingMode);
			void GetFileSinkParameters(std::wstring& outFolder, uint32_t& recordLen, CameraRecordingMode& recordingMode) const;

			void SetUris(const std::wstring& audiouri, const std::wstring& videouri);
			void GetUris(std::wstring& audiouri, std::wstring& videouri) const;

			void SetCredentials(const std::wstring& username, const std::wstring& pass);
			void GetCredentials(std::wstring& username, std::wstring& pass) const;

			void SetIsActive(bool);
			bool GetIsActive();
			bool operator==(const CameraConfMsg& other) const;
			bool operator!=(const CameraConfMsg& other) const; 
			CameraConfMsg& operator=(const CameraConfMsg& other);

			virtual void Init(std::shared_ptr<WebSocketMessageParser> parser);
			virtual web::json::value ToJsonValue() const override;
			virtual void FromJsonValue(const web::json::value& obj) override;
			virtual std::wstring ToString() const;

			static const int32_t DEFAULT_REC_LEN = 60;
			static const int32_t DEFAULT_MAX_FILES_NUM = 10;
		private:
			void SetFields(const web::json::value& json);

			int32_t _cameraId = -1;
			bool _isActive = false;
			int32_t _recordLen = DEFAULT_REC_LEN;
			int32_t _maxFilesNum = DEFAULT_MAX_FILES_NUM;
			CameraType _cameraType = CameraType::UNKNOWN;
			// Camera can have multiple modes and conditions when recording to the file is started
			CameraRecordingMode _recordingMode = CameraRecordingMode::DISABLED;
			std::wstring _cameraName;
			std::wstring _outFolder;
			std::wstring _videouri;
			std::wstring _audiouri;
			std::wstring _username;
			std::wstring _pass;
		};
	}
}
