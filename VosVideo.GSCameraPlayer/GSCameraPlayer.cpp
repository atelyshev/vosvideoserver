#include "stdafx.h"
#include "GSCameraPlayer.h"
#include "IpCameraPipeline.h"
#include "WebCameraPipeline.h"


using namespace vosvideo::cameraplayer;


GSCameraPlayer::GSCameraPlayer()
{
	LOG_TRACE("GSCameraPlayer created");
}

GSCameraPlayer::~GSCameraPlayer()
{
	LOG_TRACE("GSCameraPlayer destroying camera player");
	delete _pipeline;
}


int32_t GSCameraPlayer::OpenURL(vosvideo::data::CameraConfMsg& cameraConf)
{
	if (_state == PlayerState::OpenPending ||
		_state == PlayerState::Started ||
		_state == PlayerState::Paused ||
		_state == PlayerState::Stopped ||
		_state == PlayerState::Closing)
	{
		return -1;
	}


	std::wstring waudioUri;
	std::wstring wvideoUri;
	cameraConf.GetUris(waudioUri, wvideoUri);

	_deviceId = cameraConf.GetCameraId();
	_deviceName = cameraConf.GetCameraName();

	std::wstring username;
	std::wstring password;
	cameraConf.GetCredentials(username, password);

	bool isRecordingEnabled = false;
	std::wstring recordingFolder;
	uint32_t recordingLength = 0;
	uint32_t maxFilesNum = 0;
	vosvideo::data::CameraRecordingMode recordingMode;
	cameraConf.GetFileSinkParameters(isRecordingEnabled, recordingFolder, recordingLength, maxFilesNum, recordingMode);
	cameraType_ = cameraConf.GetCameraType();

	if (wvideoUri != L"webcamera")
	{
		_pipeline = new IpCameraPipeline(
			util::StringUtil::ToString(wvideoUri), 
			username, 
			password, 
			isRecordingEnabled,
			recordingMode,
			recordingFolder, 
			recordingLength, 
			maxFilesNum,
			_deviceName);
	}
	else
	{
		_pipeline = new WebCameraPipeline(
			isRecordingEnabled, 
			recordingMode, 
			recordingFolder, 
			recordingLength, 
			maxFilesNum, 
			_deviceName);
	}

	_pipeline->Create();

	////Need to convert to std::string due to LOG_TRACE not working with std::wstring
	//this->_deviceVideoUri = std::string(wvideoUri.begin(), wvideoUri.end());
	//std::string audioUri(waudioUri.begin(), waudioUri.end());

	//LOG_TRACE("GSCameraPlayer Opening Video URI " << this->_deviceVideoUri << " Audio URI " << audioUri);

	//this->_appThread = new boost::thread(boost::bind(&GSCameraPlayer::AppThreadStart, this));
	//this->_appThread->detach();

	_state = PlayerState::OpenPending;
	return 0;
}

void GSCameraPlayer::GetWebRtcCapability(webrtc::VideoCaptureCapability& webRtcCapability)
{
	LOG_TRACE("GetWebRtcCapability called");
	_pipeline->GetWebRtcCapability(webRtcCapability);
}

int32_t GSCameraPlayer::Play(){
	LOG_TRACE("Play called");
	return -1;
}

int32_t GSCameraPlayer::Pause()
{
	LOG_TRACE("Paused called");
	return -1;
}

int32_t GSCameraPlayer::Stop()
{
	LOG_TRACE("Stop called");
	return -1;
}

int32_t GSCameraPlayer::Shutdown()
{
	LOG_TRACE("Shutdown called");
	return -1;
}

PlayerState GSCameraPlayer::GetState(std::shared_ptr<vosvideo::data::SendData>& lastErrMsg) const
{
	LOG_TRACE("GetState(shared_ptr) called");
	return _state;
}

PlayerState GSCameraPlayer::GetState() const
{
	return _state;
}

// Probably most important method, through it camera communicates to WebRTC
void GSCameraPlayer::SetExternalCapturer(webrtc::VideoCaptureExternal* captureObserver)
{
	LOG_TRACE("SetExternalCapturer called");
	_pipeline->AddExternalCapturer(captureObserver);
}

void GSCameraPlayer::RemoveExternalCapturers()
{
	LOG_TRACE("RemoveExternalCapturers called");	
	_pipeline->RemoveAllExternalCapturers();
}

void GSCameraPlayer::RemoveExternalCapturer(webrtc::VideoCaptureExternal* captureObserver)
{
	LOG_TRACE("RemoveExternalCapturer called");
	_pipeline->RemoveExternalCapturer(captureObserver);
}

uint32_t GSCameraPlayer::GetDeviceId() const
{
	LOG_TRACE("GetDeviceId called");
	return _deviceId;
}
