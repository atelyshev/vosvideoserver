#pragma once
#include <gst/gst.h>
#include <gst/pbutils/pbutils.h>
#include "LocalVideoFile.h"

namespace vosvideo
{
	namespace archive
	{
		class LocalVideoFileDiscoverer
		{
		public:
			LocalVideoFileDiscoverer();
			~LocalVideoFileDiscoverer();

			std::vector<std::shared_ptr<VideoFile> > Discover(std::vector<std::wstring>& vectPath);
			std::shared_ptr<VideoFile> Discover(const std::wstring& vectPath);

		private:
			// Structure to contain all our information, so we can pass it around 
			struct CustomData 
			{
				GstDiscoverer *discoverer;
				GMainLoop *loop;
				std::shared_ptr<LocalVideoFile> fileInfo;
			};

			LocalVideoFile lf;
			CustomData data_;
			static void on_discovered_cb(GstDiscoverer *discoverer, GstDiscovererInfo *info, GError *err, CustomData *data);
			static void on_finished_cb(GstDiscoverer *discoverer, CustomData *data); 
			static void print_tag_foreach(const GstTagList *tags, const gchar *tag, gpointer user_data);
		};
	}
}
