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

CameraConfMsg::~CameraConfMsg()
{
}

CameraConfMsg CameraConfMsg::CreateFromDto(const wstring& archPath, const web::json::value& camParms)
{
	vector<wstring> strVideoUri(3);
	wstring customUri;
	int camId;
	bool    isActive = false;
	CameraType modelType = CameraType::UNKNOWN;
	auto recordLen = CameraConfMsg::DEFAULT_REC_LEN; // default 60 min
	auto maxFilesNum = CameraConfMsg::DEFAULT_MAX_FILES_NUM; // default 10 files
	wstring devName;
	wstring audioUri;
	wstring camIpAddr;
	wstring camUsername;
	wstring camPass;
	CameraRecordingMode recordingMode = CameraRecordingMode::DISABLED;

	if (camParms.has_field(U("DeviceName")))
	{
		auto deviceNameVal = camParms.at(U("DeviceName"));
		if (deviceNameVal.is_string())
			devName = deviceNameVal.as_string();
	}

	if (camParms.has_field(U("DeviceId")))
	{
		auto deviceIdVal = camParms.at(U("DeviceId"));
		if (deviceIdVal.is_number())
			camId = deviceIdVal.as_integer();
	}

	if (camParms.has_field(U("CustomUri")))
	{
		auto customUriVal = camParms.at(U("CustomUri"));
		if (customUriVal.is_string())
			customUri = customUriVal.as_string();
	}

	if (camParms.has_field(U("RecordingLength")))
	{
		auto recordingLengthVal = camParms.at(U("RecordingLength"));
		if (recordingLengthVal.is_number())
			recordLen = recordingLengthVal.as_integer();
	}

	if (camParms.has_field(U("MaxFilesNum")))
	{
		auto maxFilesNumVal = camParms.at(U("MaxFilesNum"));
		if (maxFilesNumVal.is_number())
			maxFilesNum = maxFilesNumVal.as_integer();
	}

	if (camParms.has_field(U("AudioUri")))
	{
		auto audioUriVal = camParms.at(U("AudioUri"));
		if (audioUriVal.is_string())
			audioUri = audioUriVal.as_string();
	}

	if (camParms.has_field(U("DeviceUserName")))
	{
		auto deviceUserNameVal = camParms.at(U("DeviceUserName"));
		if (deviceUserNameVal.is_string())
			camUsername = deviceUserNameVal.as_string();
	}

	if (camParms.has_field(U("DevicePassword")))
	{
		auto devicePasswordVal = camParms.at(U("DevicePassword"));
		if (devicePasswordVal.is_string())
			camPass = devicePasswordVal.as_string();
	}

	if (camParms.has_field(U("IsActive")))
	{
		auto isActiveVal = camParms.at(U("IsActive"));
		if (isActiveVal.is_boolean())
			isActive = isActiveVal.as_bool();
	}

	if (camParms.has_field(U("ModelType")))
	{
		auto modelTypeVal = camParms.at(U("ModelType"));
		if (modelTypeVal.is_number())
			modelType = static_cast<CameraType>(modelTypeVal.as_integer());
	}
	CameraConfMsg conf = CameraConfMsg(modelType);
	conf.SetIsActive(isActive);
	conf.SetCameraId(camId);
	conf.SetCameraName(devName);
	conf.SetCredentials(camUsername, camPass);
	conf.SetUris(audioUri, customUri);
	conf.SetFileSinkParameters(archPath, recordLen, maxFilesNum, recordingMode);
	return conf;
}

void CameraConfMsg::SetFields(const web::json::value& json){
	auto activeVal = json.at(U("isActive"));
	if (activeVal.is_string())
		_isActive = activeVal.as_bool();

	auto cameraTypeVal = json.at(U("cameraType"));
	if (cameraTypeVal.is_number())
		_cameraType = static_cast<CameraType>(cameraTypeVal.as_integer());

	auto cameraIdVal = json.at(U("cameraId"));
	if (cameraIdVal.is_number())
		_cameraId = cameraIdVal.as_integer();

	auto cameraNameVal = json.at(U("cameraName"));
	if (cameraNameVal.is_string())
		_cameraName = cameraNameVal.as_string();

	auto outFolderVal = json.at(U("outFolder"));
	if (outFolderVal.is_string())
		_outFolder = outFolderVal.as_string();

	auto recordLenVal = json.at(U("recordLen"));
	if (recordLenVal.is_number())
		_recordLen = recordLenVal.as_integer();

	auto videoUriVal = json.at(U("videouri"));
	if (videoUriVal.is_string())
		_videouri = videoUriVal.as_string();

	auto audioUriVal = json.at(U("audiouri"));
	if (audioUriVal.is_string())
		_audiouri = audioUriVal.as_string();

	auto usernameVal = json.at(U("username"));
	if (usernameVal.is_string())
		_username = usernameVal.as_string();

	auto passVal = json.at(U("pass"));
	if (passVal.is_string())
		_pass = passVal.as_string();
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
	const wstring& outFolder, 
	uint32_t recordLen, 
	uint32_t maxFilesNum, 
	CameraRecordingMode recordingMode)
{
	_outFolder = outFolder;
	_recordLen = recordLen;
	_maxFilesNum = maxFilesNum;
	_recordingMode = recordingMode;
}

void CameraConfMsg::GetFileSinkParameters(wstring& outFolder, uint32_t& recordLen, CameraRecordingMode& recordingMode) const
{
	outFolder = _outFolder;
	recordLen = _recordLen;
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
	jObj[L"outFolder"] = web::json::value::string(_outFolder);
	jObj[L"recordLen"] = web::json::value::number(_recordLen);
	jObj[L"maxFilesNum"] = web::json::value::number(_maxFilesNum);
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
		_outFolder == other._outFolder     &&
		_recordLen == other._recordLen     &&
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
		_outFolder   = other._outFolder;
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
