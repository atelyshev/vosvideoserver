#pragma once
#include "GSPipelineBase.h"

namespace vosvideo
{
	namespace cameraplayer
	{
		class IpCameraPipeline : public GSPipelineBase
		{
		public:
			IpCameraPipeline(const std::string& uri, 
				const std::wstring& username, 
				const std::wstring& password,
				bool isRecordingEnabled,
				vosvideo::data::CameraRecordingMode recordingMode,
				const std::wstring& recordingFolder,
				uint32_t recordingLength,
				uint32_t maxFilesNum,
				const std::wstring& camName);
			void ConfigureVideoBin();

		protected:
			GstElement* CreateSource();
			bool LinkElements();

		private:
			static GstPadProbeReturn CbHaveSample(GstPad* pad, GstPadProbeInfo* info, IpCameraPipeline* ipCameraPipeline);
			static void CbPadAddedHandler(GstElement *src, GstPad *new_pad, IpCameraPipeline* cameraPlayer);
			static void CbSourceSetupHandler(GstElement *element, GstElement *source, IpCameraPipeline* cameraPlayer);
			static void CbDrainedHandler(GstElement *element, IpCameraPipeline* ipCameraPipeline);
			std::wstring _username;
			std::wstring _password;
			std::string _uri;
			GstClockTime _timestamp = 0;
			GstClockTime _startTimestamp = 0;
		};
	}
}