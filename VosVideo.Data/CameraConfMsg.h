#pragma once

#include "ReceivedData.h"

namespace vosvideo
{
	namespace data
	{
		enum class CameraVideoFormat
		{
			UNKNOWN = -1,
			MJPEG,
			MPEG4,
			H264
		};

		enum class CameraType
		{
			UNKNOWN = -1,
			IPCAM,
			WEBCAM
		};

		enum class CameraVideoRecording
		{
			DISABLED,
			PERMANENT,
			ONMOTION,
			ONSCHEDULER
		};

		class CameraConfMsg final : public ReceivedData
		{
		public:
			CameraConfMsg();
			CameraConfMsg(const std::wstring& jsonStr);
			CameraConfMsg(CameraType camType, CameraVideoFormat vf); 

			virtual ~CameraConfMsg();

			CameraVideoFormat GetVideoFormat();

			CameraType GetCameraType();

			void SetCameraIds(int cameraId, const std::wstring& cameraName);
			void GetCameraIds(int& cameraId, std::wstring& cameraName) const;

			void SetFileSinkParameters(const std::wstring& outFolder, int recordLen, CameraVideoRecording recordingType);
			void GetFileSinkParameters(std::wstring& outFolder, int& recordLen, CameraVideoRecording& recordingType) const;

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
			virtual void ToJsonValue(web::json::value& obj) const;
			virtual void FromJsonValue(web::json::value& obj);
			virtual std::wstring ToString() const;
			static void ToObject(const std::wstring& jsonStr, CameraConfMsg& objRef);

		private:
			bool isActive_;
			CameraType cameraType_;
			CameraVideoFormat videoFormat_;
			int cameraId_; 
			std::wstring cameraName_;
			std::wstring outFolder_;
			int recordLen_; 
			// Camera can have multiple modes and conditions when recording to the file is started
			CameraVideoRecording recordingType_; 
			std::wstring videouri_;
			std::wstring audiouri_;
			std::wstring username_;
			std::wstring pass_;
		};
	}
}
