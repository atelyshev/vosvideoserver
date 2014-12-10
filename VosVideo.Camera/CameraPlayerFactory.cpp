#include "stdafx.h"
#include "CameraPlayerFactory.h"
//#include "VosVideo.MFCameraPlayer/MFCameraPlayer.h"
//#include "VosVideo.MFCameraPlayer/MFCameraPlayerBootstrapper.h"
#include "VosVideo.GSCameraPlayer/GSCameraPlayer.h"
#include "VosVideo.GSCameraPlayer/GSCameraPlayerBootstrapper.h"


using vosvideo::camera::CameraPlayerFactory;
using namespace vosvideo::cameraplayer;

CameraPlayerFactory::CameraPlayerFactory(){
}

CameraPlayerFactory::~CameraPlayerFactory(){
}

void CameraPlayerFactory::Init(int* argc, char **argv[]){
	GSCameraPlayerBootstrapper cameraPlayerBootstrapper;
	cameraPlayerBootstrapper.Init(argc, argv);
}

void CameraPlayerFactory::Shutdown(){
	GSCameraPlayerBootstrapper cameraPlayerBootstrapper;
	cameraPlayerBootstrapper.Shutdown();
}

CameraPlayerBase* CameraPlayerFactory::CreateCameraPlayer(){
	GSCameraPlayer* cameraPlayer = new GSCameraPlayer();
	return cameraPlayer;
}