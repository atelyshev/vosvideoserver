#include "stdafx.h"
#include <cpprest/json.h>
#include "VosVideo.Data/CameraConfMsg.h"

using namespace std;
using namespace vosvideo::data;

CameraConfMsg::CameraConfMsg(CameraType ct) : 
	ReceivedData(),
	_cameraType(ct)
{
}

CameraConfMsg::CameraConfMsg(const std::wstring& jsonStr)
{
	web::json::value jObj = web::json::value::parse(jsonStr);
	SetFields(jObj);
}

CameraConfMsg CameraConfMsg::CreateFromDto(const wstring& archivePath, const web::json::value& camParms)
{
	CameraConfMsg conf = CameraConfMsg();

	conf._archivePath = archivePath;

	if (camParms.has_field(U("ModelType")))
	{
		if (camParms.at(U("ModelType")).is_number())
			conf._cameraType = static_cast<CameraType>(camParms.at(U("ModelType")).as_integer());
	}

	if (camParms.has_field(U("DeviceName")))
	{
		if (camParms.at(U("DeviceName")).is_string())
			conf._cameraName = camParms.at(U("DeviceName")).as_string();
	}

	if (camParms.has_field(U("DeviceId")))
	{
		if (camParms.at(U("DeviceId")).is_number())
			conf._cameraId = camParms.at(U("DeviceId")).as_integer();
	}

	if (camParms.has_field(U("CustomUri")))
	{
		if (camParms.at(U("CustomUri")).is_string())
			conf._videouri = camParms.at(U("CustomUri")).as_string();
	}

	if (camParms.has_field(U("IsRecordingEnabled")))
	{
		if (camParms.at(U("IsRecordingEnabled")).is_boolean())
			conf._isRecordingEnabled = camParms.at(U("IsRecordingEnabled")).as_bool();
	}

	if (camParms.has_field(U("RecordingLength")))
	{
		if (camParms.at(U("RecordingLength")).is_number())
			conf._recordLen = camParms.at(U("RecordingLength")).as_integer();
	}

	if (camParms.has_field(U("MaxFilesNum")))
	{
		if (camParms.at(U("MaxFilesNum")).is_number())
			conf._maxFilesNum = camParms.at(U("MaxFilesNum")).as_integer();
	}

	if (camParms.has_field(U("AudioUri")))
	{
		if (camParms.at(U("AudioUri")).is_string())
			conf._audiouri = camParms.at(U("AudioUri")).as_string();
	}

	if (camParms.has_field(U("DeviceUserName")))
	{
		if (camParms.at(U("DeviceUserName")).is_string())
			conf._username = camParms.at(U("DeviceUserName")).as_string();
	}

	if (camParms.has_field(U("DevicePassword")))
	{
		if (camParms.at(U("DevicePassword")).is_string())
			conf._pass = camParms.at(U("DevicePassword")).as_string();
	}

	if (camParms.has_field(U("IsActive")))
	{
		if (camParms.at(U("IsActive")).is_boolean())
			conf._isActive = camParms.at(U("IsActive")).as_bool();
	}

	return conf;
}

void CameraConfMsg::SetFields(const web::json::value& json)
{
	auto tmp = json.serialize();
	if (json.at(U("isActive")).is_boolean())
		_isActive = json.at(U("isActive")).as_bool();

	if (json.at(U("cameraType")).is_number())
		_cameraType = static_cast<CameraType>(json.at(U("cameraType")).as_integer());

	if (json.at(U("cameraId")).is_number())
		_cameraId = json.at(U("cameraId")).as_integer();

	if (json.at(U("cameraName")).is_string())
		_cameraName = json.at(U("cameraName")).as_string();

	if (json.at(U("archivePath")).is_string())
		_archivePath = json.at(U("archivePath")).as_string();

	if (json.at(U("isRecordingEnabled")).is_boolean())
		_isRecordingEnabled = json.at(U("isRecordingEnabled")).as_bool();

	if (json.at(U("recordLen")).is_number())
		_recordLen = json.at(U("recordLen")).as_integer();

	if (json.at(U("recordLen")).is_number())
		_recordLen = json.at(U("recordLen")).as_integer();

	if (json.at(U("videouri")).is_string())
		_videouri = json.at(U("videouri")).as_string();

	if (json.at(U("audiouri")).is_string())
		_audiouri = json.at(U("audiouri")).as_string();

	if (json.at(U("username")).is_string())
		_username = json.at(U("username")).as_string();

	if (json.at(U("pass")).is_string())
		_pass = json.at(U("pass")).as_string();
}

void CameraConfMsg::Init(std::shared_ptr<WebSocketMessageParser> parser)
{
	web::json::value obj;
	parser->GetPayload(obj);
	FromJsonValue(obj);
}

CameraType CameraConfMsg::GetCameraType()
{
	return _cameraType;
}

void CameraConfMsg::SetCameraId(int cameraId)
{
	_cameraId = cameraId;
}

void CameraConfMsg::SetCameraName(const wstring& cameraName)
{
	_cameraName = cameraName;
}

int CameraConfMsg::GetCameraId() const
{
	return _cameraId;
}

std::wstring CameraConfMsg::GetCameraName() const
{
	return _cameraName;
}

void CameraConfMsg::SetFileSinkParameters(
	const wstring& archivePath,
	uint32_t recordLen, 
	uint32_t maxFilesNum, 
	CameraRecordingMode recordingMode)
{
	_archivePath = archivePath;
	_recordLen = recordLen;
	_maxFilesNum = maxFilesNum;
	_recordingMode = recordingMode;
}

void CameraConfMsg::GetFileSinkParameters(
	bool& isRecordingEnabled, 
	wstring& archivePath,
	uint32_t& recordLen, 
	uint32_t& maxFilesNum,
	CameraRecordingMode& recordingMode) const
{
	isRecordingEnabled = _isRecordingEnabled;
	archivePath = _archivePath;
	recordLen = _recordLen;
	maxFilesNum = _maxFilesNum;
	recordingMode = _recordingMode;
}

void CameraConfMsg::SetUris(const wstring& audiouri, const wstring& videouri)
{
	_audiouri = audiouri;
	_videouri = videouri;
}

void CameraConfMsg::GetUris(wstring& audiouri, wstring& videouri) const
{
	audiouri = _audiouri;
	videouri = _videouri;
}

void CameraConfMsg::SetCredentials(const wstring& username, const wstring& pass)
{
	_username = username;
	_pass = pass;
}

void CameraConfMsg::GetCredentials(wstring& username, wstring& pass) const
{
	username = _username;
	pass = _pass;
}

void CameraConfMsg::SetIsActive(bool isActive)
{
	_isActive = isActive;
}

bool CameraConfMsg::GetIsActive()
{
	return _isActive;
}

web::json::value CameraConfMsg::ToJsonValue() const
{
	web::json::value jObj;
	jObj[L"mt"] = web::json::value::number(static_cast<int>(vosvideo::data::MsgType::CameraConfMsg));
	jObj[L"isActive"] = web::json::value::boolean(_isActive);
	jObj[L"cameraType"] = web::json::value::number(static_cast<int>(_cameraType));
	jObj[L"cameraId"] = web::json::value::number(_cameraId);
	jObj[L"cameraName"] = web::json::value::string(_cameraName);
	jObj[L"archivePath"] = web::json::value::string(_archivePath);
	jObj[L"recordLen"] = web::json::value::number(_recordLen);
	jObj[L"maxFilesNum"] = web::json::value::number(_maxFilesNum);
	jObj[L"isRecordingEnabled"] = web::json::value::boolean(_isRecordingEnabled);
	jObj[L"recordingMode"] = web::json::value::number(static_cast<int>(_recordingMode));
	jObj[L"videouri"] = web::json::value::string(_videouri);
	jObj[L"audiouri"] = web::json::value::string(_audiouri);
	jObj[L"username"] = web::json::value::string(_username);
	jObj[L"pass"] = web::json::value::string(_pass);
	return jObj;
}

void CameraConfMsg::FromJsonValue(const web::json::value& jpayload)
{
	SetFields(jpayload);
}

wstring CameraConfMsg::ToString() const
{
	auto jObj = ToJsonValue();
	return jObj.serialize();
}

bool CameraConfMsg::operator==(const CameraConfMsg &other) const
{
	return (
		_isActive == other._isActive && 
		_cameraType == other._cameraType   &&
		_cameraId == other._cameraId       &&
		_cameraName == other._cameraName   &&
		_archivePath == other._archivePath     &&
		_recordLen == other._recordLen     &&
		_isRecordingEnabled == other._isRecordingEnabled &&
		_maxFilesNum == other._maxFilesNum &&
		_recordingMode == other._recordingMode &&
		_videouri == other._videouri &&
		_audiouri == other._audiouri &&
		_username == other._username &&
		_pass == other._pass);
}

bool CameraConfMsg::operator!=(const CameraConfMsg &other) const 
{
	return !(*this == other);
}

CameraConfMsg& CameraConfMsg::operator=(const CameraConfMsg& other)
{
	if (this != &other) // protect against invalid self-assignment
	{
		_isActive = other._isActive;
		_cameraType  = other._cameraType;
		_cameraId    = other._cameraId;
		_cameraName  = other._cameraName;
		_archivePath = other._archivePath;
		_isRecordingEnabled = other._isRecordingEnabled;
		_recordLen   = other._recordLen;
		_maxFilesNum = other._maxFilesNum;
		_recordingMode = other._recordingMode;
		_videouri    = other._videouri;
		_audiouri    = other._audiouri;
		_username    = other._username;
		_pass        = other._pass;
	}
	// by convention, always return *this
	return *this;
}
