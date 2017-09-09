#include "stdafx.h"
#include "VosVideo.GSCameraPlayer/GSCameraPlayer.h"
#include "VosVideo.GSCameraPlayer/GSCameraPlayerBootstrapper.h"
#include "CameraPlayerFactory.h"


using vosvideo::camera::CameraPlayerFactory;
using namespace vosvideo::cameraplayer;

CameraPlayerFactory::CameraPlayerFactory()
{
}

CameraPlayerFactory::~CameraPlayerFactory()
{
}

void CameraPlayerFactory::Init(int* argc, char **argv[])
{
	GSCameraPlayerBootstrapper cameraPlayerBootstrapper;
	cameraPlayerBootstrapper.Init(argc, argv);
}

void CameraPlayerFactory::Shutdown()
{
	GSCameraPlayerBootstrapper cameraPlayerBootstrapper;
	cameraPlayerBootstrapper.Shutdown();
}

CameraPlayerBase* CameraPlayerFactory::CreateCameraPlayer()
{
	return new GSCameraPlayer();
}