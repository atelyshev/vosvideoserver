#include "stdafx.h"
#include "MFCameraPlayerBootstrapper.h"

using vosvideo::cameraplayer::MFCameraPlayerBootstrapper;

MFCameraPlayerBootstrapper::MFCameraPlayerBootstrapper(){
}

MFCameraPlayerBootstrapper::~MFCameraPlayerBootstrapper(){
}

void MFCameraPlayerBootstrapper::Init(int* argc, char **argv[]){
	MFStartup(MF_VERSION);
}

void MFCameraPlayerBootstrapper::Shutdown(){
	MFShutdown();
}

