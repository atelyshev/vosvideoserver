#include "stdafx.h"
#include <cpprest/json.h>
#include "CameraConfMsg.h"

using namespace std;
using namespace vosvideo::data;

CameraConfMsg::CameraConfMsg() :
	cameraType_(CameraType::UNKNOWN),
	videoFormat_(CameraVideoFormat::UNKNOWN),
	recordingType_(CameraVideoRecording::DISABLED), // Do nothing by default
	cameraId_(-1),
	recordLen_(-1),
	isActive_(false)
{
}

CameraConfMsg::CameraConfMsg(CameraType ct, CameraVideoFormat vf) : 
	ReceivedData(),
	cameraType_(ct),
	videoFormat_(vf), 
	recordingType_(CameraVideoRecording::DISABLED), // Do nothing by default
	cameraId_(-1),
	recordLen_(-1),
	isActive_(false)
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

void CameraConfMsg::SetFields(const web::json::value& json){
	auto activeVal = json.at(U("isActive"));
	if (activeVal.is_string())
		isActive_ = activeVal.as_bool();

	auto cameraTypeVal = json.at(U("cameraType"));
	if (cameraTypeVal.is_number())
		cameraType_ = static_cast<CameraType>(cameraTypeVal.as_integer());

	auto videoFormatVal = json.at(U("videoFormat"));
	if (videoFormatVal.is_number())
		videoFormat_ = static_cast<CameraVideoFormat>(videoFormatVal.as_integer());

	auto cameraIdVal = json.at(U("cameraId"));
	if (cameraIdVal.is_number())
		cameraId_ = cameraIdVal.as_integer();

	auto cameraNameVal = json.at(U("cameraName"));
	if (cameraNameVal.is_string())
		cameraName_ = cameraNameVal.as_string();

	auto outFolderVal = json.at(U("outFolder"));
	if (outFolderVal.is_string())
		outFolder_ = outFolderVal.as_string();

	auto recordLenVal = json.at(U("recordLen"));
	if (recordLenVal.is_number())
		recordLen_ = recordLenVal.as_integer();

	auto recordingTypeVal = json.at(U("recordingType"));
	if (recordingTypeVal.is_number())
		recordingType_ = static_cast<CameraVideoRecording>(recordingTypeVal.as_integer());

	auto videoUriVal = json.at(U("videouri"));
	if (videoUriVal.is_string())
		videouri_ = videoUriVal.as_string();

	auto audioUriVal = json.at(U("audiouri"));
	if (audioUriVal.is_string())
		audiouri_ = audioUriVal.as_string();

	auto usernameVal = json.at(U("username"));
	if (usernameVal.is_string())
		username_ = usernameVal.as_string();

	auto passVal = json.at(U("pass"));
	if (passVal.is_string())
		pass_ = passVal.as_string();

}

void CameraConfMsg::Init(std::shared_ptr<WebSocketMessageParser> parser)
{
	web::json::value obj;
	parser->GetPayload(obj);
	FromJsonValue(obj);
}

CameraVideoFormat CameraConfMsg::GetVideoFormat()
{
	return videoFormat_;
}

CameraType CameraConfMsg::GetCameraType()
{
	return cameraType_;
}

void CameraConfMsg::SetCameraIds(int cameraId, const wstring& cameraName)
{
	cameraId_ = cameraId;
	cameraName_ = cameraName;
}

void CameraConfMsg::GetCameraIds(int& cameraId, wstring& cameraName) const
{
	cameraId = cameraId_;
	cameraName = cameraName_;
}

void CameraConfMsg::SetFileSinkParameters(const wstring& outFolder, uint32_t recordLen, CameraVideoRecording recordingType)
{
	outFolder_ = outFolder;
	recordLen_ = recordLen;
	recordingType_ = recordingType;
}

void CameraConfMsg::GetFileSinkParameters(wstring& outFolder, uint32_t& recordLen, CameraVideoRecording& recordingType) const
{
	outFolder = outFolder_;
	recordLen = recordLen_;
	recordingType = recordingType_;
}

void CameraConfMsg::SetUris(const wstring& audiouri, const wstring& videouri)
{
	audiouri_ = audiouri;
	videouri_ = videouri;
}

void CameraConfMsg::GetUris(wstring& audiouri, wstring& videouri) const
{
	audiouri = audiouri_;
	videouri = videouri_;
}

void CameraConfMsg::SetCredentials(const wstring& username, const wstring& pass)
{
	username_ = username;
	pass_ = pass;
}

void CameraConfMsg::GetCredentials(wstring& username, wstring& pass) const
{
	username = username_;
	pass = pass_;
}

void CameraConfMsg::SetIsActive(bool isActive)
{
	isActive_ = isActive;
}

bool CameraConfMsg::GetIsActive()
{
	return isActive_;
}

void CameraConfMsg::ToJsonValue(web::json::value& jObj) const
{
	jObj[L"mt"] = web::json::value::number(static_cast<int>(vosvideo::data::MsgType::CameraConfMsg));
	jObj[L"isActive"] = web::json::value::boolean(isActive_);
	jObj[L"cameraType"] = web::json::value::number(static_cast<int>(cameraType_));
	jObj[L"videoFormat"] = web::json::value::number(static_cast<int>(videoFormat_));
	jObj[L"cameraId"] = web::json::value::number(cameraId_);
	jObj[L"cameraName"] = web::json::value::string(cameraName_);
	jObj[L"outFolder"] = web::json::value::string(outFolder_);
	jObj[L"recordLen"] = web::json::value::number(recordLen_);
	jObj[L"recordingType"] = web::json::value::number(static_cast<int>(recordingType_));
	jObj[L"videouri"] = web::json::value::string(videouri_);
	jObj[L"audiouri"] = web::json::value::string(audiouri_);
	jObj[L"username"] = web::json::value::string(username_);
	jObj[L"pass"] = web::json::value::string(pass_);
}

void CameraConfMsg::FromJsonValue(web::json::value& jpayload)
{
	SetFields(jpayload);
}

wstring CameraConfMsg::ToString() const
{
	web::json::value jObj;
	ToJsonValue(jObj);
	return jObj.serialize();
}

bool CameraConfMsg::operator==(const CameraConfMsg &other) const
{
	return (
		isActive_ == other.isActive_ && 
		cameraType_ == other.cameraType_ &&
		videoFormat_ == other.videoFormat_ &&
		cameraId_ == other.cameraId_       &&
		cameraName_ == other.cameraName_   &&
		outFolder_ == other.outFolder_     &&
		recordLen_ == other.recordLen_     &&
		recordingType_ == other.recordingType_ &&
		videouri_ == other.videouri_ &&
		audiouri_ == other.audiouri_ &&
		username_ == other.username_ &&
		pass_ == other.pass_);
}

bool CameraConfMsg::operator!=(const CameraConfMsg &other) const 
{
	return !(*this == other);
}

CameraConfMsg& CameraConfMsg::operator=(const CameraConfMsg& other)
{
	if (this != &other) // protect against invalid self-assignment
	{
		isActive_ = other.isActive_;
		cameraType_  = other.cameraType_;
		videoFormat_ = other.videoFormat_;
		cameraId_    = other.cameraId_;
		cameraName_  = other.cameraName_;
		outFolder_   = other.outFolder_;
		recordLen_   = other.recordLen_;
		recordingType_   = other.recordingType_;
		videouri_    = other.videouri_;
		audiouri_    = other.audiouri_;
		username_    = other.username_;
		pass_        = other.pass_;
	}
	// by convention, always return *this
	return *this;
}
