#include "stdafx.h"
#include "GSCameraPlayer.h"
#include "IpCameraPipeline.h"
#include "WebCameraPipeline.h"


using namespace vosvideo::cameraplayer;


GSCameraPlayer::GSCameraPlayer(){
	LOG_TRACE("GSCameraPlayer created");
}

GSCameraPlayer::~GSCameraPlayer(){
	LOG_TRACE("GSCameraPlayer destroying camera player");
	delete _pipeline;
}


HRESULT GSCameraPlayer::OpenURL(vosvideo::data::CameraConfMsg& cameraConf){
	if(_state == PlayerState::OpenPending || _state == PlayerState::Started || _state == PlayerState::Paused || _state == PlayerState::Stopped || _state == PlayerState::Closing)
		return E_FAIL;

	std::wstring waudioUri;
	std::wstring wvideoUri;
	std::wstring username;
	std::wstring password;

	cameraConf.GetUris(waudioUri, wvideoUri);
	cameraConf.GetCameraIds(this->_deviceId, this->_deviceName);
	cameraConf.GetCredentials(username, password);

	if (wvideoUri != L"webcamera")
	{
		_pipeline = new IpCameraPipeline(util::StringUtil::ToString(wvideoUri), username, password);
	}
	else
	{
		_pipeline = new WebCameraPipeline();
	}

	//
	////Need to convert to std::string due to LOG_TRACE not working with std::wstring
	//this->_deviceVideoUri = std::string(wvideoUri.begin(), wvideoUri.end());
	//std::string audioUri(waudioUri.begin(), waudioUri.end());

	//LOG_TRACE("GSCameraPlayer Opening Video URI " << this->_deviceVideoUri << " Audio URI " << audioUri);

	//this->_appThread = new boost::thread(boost::bind(&GSCameraPlayer::AppThreadStart, this));
	//this->_appThread->detach();

	_state = PlayerState::OpenPending;
	return S_OK;
}

void GSCameraPlayer::GetWebRtcCapability(webrtc::VideoCaptureCapability& webRtcCapability){
	LOG_TRACE("GSCameraPlayer GetWebRtcCapability called");
	_pipeline->GetWebRtcCapability(webRtcCapability);
}

HRESULT GSCameraPlayer::Play(){
	LOG_TRACE("GSCameraPlayer Play called");
	return E_FAIL;
}

HRESULT GSCameraPlayer::Pause(){
	LOG_TRACE("GSCameraPlayer Paused called");
	return E_FAIL;
}

HRESULT GSCameraPlayer::Stop(){
	LOG_TRACE("GSCameraPlayer Stop called");
	return E_FAIL;
}

HRESULT GSCameraPlayer::Shutdown(){
	LOG_TRACE("GSCameraPlayer Shutdown called");
	return E_FAIL;
}

PlayerState GSCameraPlayer::GetState(std::shared_ptr<vosvideo::data::SendData>& lastErrMsg) const{
	LOG_TRACE("GSCameraPlayer GetState(shared_ptr) called");
	return _state;
}

PlayerState GSCameraPlayer::GetState() const{
	LOG_TRACE("GSCameraPlayer GetState called");
	return _state;
}

// Probably most important method, through it camera communicates to WebRTC
void GSCameraPlayer::SetExternalCapturer(webrtc::VideoCaptureExternal* captureObserver){
	LOG_TRACE("GSCameraPlayer SetExternalCapturer called");
	_pipeline->AddExternalCapturer(captureObserver);
}

void GSCameraPlayer::RemoveExternalCapturers(){
	LOG_TRACE("GSCameraPlayer RemoveExternalCapturers called");	
	_pipeline->RemoveAllExternalCapturers();
}

void GSCameraPlayer::RemoveExternalCapturer(webrtc::VideoCaptureExternal* captureObserver){
	LOG_TRACE("GSCameraPlayer RemoveExternalCapturer called");
	_pipeline->RemoveExternalCapturer(captureObserver);
}

uint32_t GSCameraPlayer::GetDeviceId() const{
	LOG_TRACE("GSCameraPlayer GetDeviceId called");
	return this->_deviceId;
}
