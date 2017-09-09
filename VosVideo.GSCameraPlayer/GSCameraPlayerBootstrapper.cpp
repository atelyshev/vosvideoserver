#include "stdafx.h"
#include <gst/gst.h>
#include <gst/video/video.h>
#include "GSCameraPlayerBootstrapper.h"

using vosvideo::cameraplayer::GSCameraPlayerBootstrapper;

void GSCameraPlayerBootstrapper::Init(int* argc, char **argv[])
{
	gst_init(argc, argv);
	if (gst_is_initialized())
		std::cout << "GStreamer initialized" << std::endl;
	else 
		std::cout << "Failed to init GStreamer" << std::endl;
}

void GSCameraPlayerBootstrapper::Shutdown()
{
}

