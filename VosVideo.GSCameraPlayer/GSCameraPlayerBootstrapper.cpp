#include "stdafx.h"
#include "GSCameraPlayerBootstrapper.h"

using vosvideo::cameraplayer::GSCameraPlayerBootstrapper;

GSCameraPlayerBootstrapper::GSCameraPlayerBootstrapper(){
}

GSCameraPlayerBootstrapper::~GSCameraPlayerBootstrapper(){
}

void GSCameraPlayerBootstrapper::Init(int* argc, char **argv[]){
	gst_init(argc, argv);
	std::cout << "GStreamer initialized" << std::endl;
}

void GSCameraPlayerBootstrapper::Shutdown(){
}

