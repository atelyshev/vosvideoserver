#pragma once
#include "GSPipelineBase.h"

namespace vosvideo
{
	namespace cameraplayer
	{
		class IpCameraPipeline : public GSPipelineBase
		{
		public:
			IpCameraPipeline(const std::string& uri, const std::wstring& username, const std::wstring& password);
			virtual ~IpCameraPipeline();
		protected:
			GstElement* CreateSource();
			gboolean LinkElements();
		private:
			static void PadAddedHandler(GstElement *src, GstPad *new_pad, IpCameraPipeline *cameraPlayer);
			static void SourceSetupHandler(GstElement *element, GstElement *source, IpCameraPipeline *cameraPlayer);
			static void DrainedHandler(GstElement *element, IpCameraPipeline *ipCameraPipeline);
			std::wstring _username;
			std::wstring _password;
			std::string _uri;
		};
	}
}