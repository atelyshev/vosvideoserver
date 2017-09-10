#include "stdafx.h"
#include "VideoFileDiscovererException.h"
#include "LocalVideoFileDiscoverer.h"


using namespace std;
using namespace util;
using namespace vosvideo::archive;

LocalVideoFileDiscoverer::LocalVideoFileDiscoverer()
{
/*	GError *err = nullptr;
	// Instantiate the Discoverer 
	data_.discoverer = gst_discoverer_new(5 * GST_SECOND, &err);
	if (!data_.discoverer)
	{
		string strErr = "Error creating discoverer instance: " + string(err->message);
		g_clear_error(&err);
		throw new VideoFileDiscovererException(strErr);
	}

	// Connect to the interesting signals 
	g_signal_connect(data_.discoverer, "discovered", G_CALLBACK(&LocalVideoFileDiscoverer::on_discovered_cb), &data_);
	g_signal_connect(data_.discoverer, "finished", G_CALLBACK(&LocalVideoFileDiscoverer::on_finished_cb), &data_);
	// Start the discoverer process (nothing to do yet) 
	gst_discoverer_start(data_.discoverer);*/
}

LocalVideoFileDiscoverer::~LocalVideoFileDiscoverer()
{
	g_object_unref(data_.discoverer);
	g_main_loop_unref(data_.loop);
}

shared_ptr<VideoFile> LocalVideoFileDiscoverer::Discover(const wstring& path)
{
	vector<wstring> vectPath;
	vectPath.push_back(path);
	auto lvf = Discover(vectPath);
	shared_ptr<LocalVideoFile> retFile;

	if (lvf.size() == 0) // return in invalid state
	{
		return shared_ptr<LocalVideoFile>(new LocalVideoFile());
	}
	return lvf[0];
}

vector<shared_ptr<VideoFile> > LocalVideoFileDiscoverer::Discover(vector<wstring>& vectPath)
{
	vector<shared_ptr<VideoFile>> outInfo;

	for (const auto& vp : vectPath)
	{
		string strPath = "file:///" + StringUtil::ToString(vp);
		const gchar *uri = strPath.c_str();
//		data_.fileInfo.reset(new LocalVideoFile(*iter));

		if (!gst_discoverer_discover_uri_async(data_.discoverer, uri))
		{
			LOG_ERROR("Failed to start discovering URI" << uri);
			g_object_unref(data_.discoverer);
		}
		else
		{
			/* Create a GLib Main Loop and set it to run, so we can wait for the signals */
			data_.loop = g_main_loop_new(NULL, FALSE);
			g_main_loop_run(data_.loop);
			outInfo.push_back(data_.fileInfo);
		}
	}

	return outInfo;
}

// This function is called when the discoverer has finished examining
// all the URIs we provided.
void LocalVideoFileDiscoverer::on_finished_cb(GstDiscoverer *discoverer, LocalVideoFileDiscoverer::CustomData *data)
{
	g_main_loop_quit(data->loop);
}

// This function is called every time the discoverer has information regarding
// one of the URIs we provided.
void LocalVideoFileDiscoverer::on_discovered_cb(GstDiscoverer *discoverer, GstDiscovererInfo *info, GError *err, CustomData *data)
{
	GstDiscovererResult result;
	const gchar *uri;
	const GstTagList *tags;
	GstDiscovererStreamInfo *sinfo;

	uri = gst_discoverer_info_get_uri(info);
	result = gst_discoverer_info_get_result(info);
	switch (result)
	{
	case GST_DISCOVERER_URI_INVALID:
		LOG_ERROR("Invalid URI" << uri);
		break;
	case GST_DISCOVERER_ERROR:
		LOG_ERROR("Discoverer error: " << err->message);
		break;
	case GST_DISCOVERER_TIMEOUT:
		LOG_ERROR("Timeout");
		break;
	case GST_DISCOVERER_BUSY:
		LOG_ERROR("Busy");
		break;
	case GST_DISCOVERER_MISSING_PLUGINS:
	{
		const GstStructure *s;
		gchar *str;

		s = gst_discoverer_info_get_misc(info);
		str = gst_structure_to_string(s);
		LOG_ERROR("Missing plugins: " << str);
		g_free(str);
		break;
	}
	case GST_DISCOVERER_OK:
		LOG_TRACE("Discovered " << uri);
		break;
	}

	if (result != GST_DISCOVERER_OK)
	{
		g_printerr("This URI cannot be played\n");
		return;
	}
	
	// If we got no error, store the retrieved information 
	// Duration
	GstClockTime duration = gst_discoverer_info_get_duration(info);
//	g_print("\nDuration: %" GST_TIME_FORMAT "\n", GST_TIME_ARGS(gst_discoverer_info_get_duration(info)));
	bool isSeekable = gst_discoverer_info_get_seekable(info) ? true : false;

	string suri(uri);
	wstring wuri = StringUtil::ToWstring(suri);
	data->fileInfo.reset(new LocalVideoFile(wuri, wuri, duration, 0, isSeekable));

	//tags = gst_discoverer_info_get_tags(info);
	//if (tags) 
	//{
	//	g_print("Tags:\n");
	//	gst_tag_list_foreach(tags, &LocalVideoFileDiscoverer::print_tag_foreach, GINT_TO_POINTER(1));
	//}

	//g_print("Seekable: %s\n", (gst_discoverer_info_get_seekable(info) ? "yes" : "no"));
	//sinfo = gst_discoverer_info_get_stream_info(info);

	//if (sinfo)
	//{
	//	gst_discoverer_stream_info_unref(sinfo);
	//}
}

/* Print a tag in a human-readable format (name: value) */
void LocalVideoFileDiscoverer::print_tag_foreach(const GstTagList *tags, const gchar *tag, gpointer user_data) 
{
	GValue val = { 0, };
	gchar *str;
	gint depth = GPOINTER_TO_INT(user_data);

	gst_tag_list_copy_value(&val, tags, tag);

	if (G_VALUE_HOLDS_STRING(&val))
		str = g_value_dup_string(&val);
	else
		str = gst_value_serialize(&val);

	g_print("%*s%s: %s\n", 2 * depth, " ", gst_tag_get_nick(tag), str);
	g_free(str);

	g_value_unset(&val);
}