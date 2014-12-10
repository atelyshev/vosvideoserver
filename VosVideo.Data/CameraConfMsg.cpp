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
	web::json::value::iterator it;

	for(it = jObj.begin(); it != jObj.end(); ++it)
	{
		wstring key = (*it).first.as_string();
		web::json::value value = (*it).second;
		// For video url we should make decision
		if (key == L"isActive")
		{
			if(value.is_string())
				isActive_ = value.as_bool();
		}

		if (key == L"cameraType")
		{
			if(value.is_string())
				cameraType_ = static_cast<CameraType>(value.as_integer());
		}

		if (key == L"videoFormat")
		{
			if(value.is_string())
				videoFormat_ = static_cast<CameraVideoFormat>(value.as_integer());
		}

		if (key == L"cameraId")
		{
			if(value.is_string())
				cameraId_ = value.as_integer();
		}

		if (key == L"cameraName")
		{
			if(value.is_string())
				cameraName_ = value.as_string();
		}

		if (key == L"outFolder")
		{
			if(value.is_string())
				outFolder_ = value.as_string();
		}

		if (key == L"recordLen")
		{
			if(value.is_string())
				recordLen_ = value.as_integer();
		}

		if (key == L"recordingType")
		{
			if(value.is_string())
				recordingType_ = static_cast<CameraVideoRecording>(value.as_integer());
		}

		if (key == L"videouri")
		{
			if(value.is_string())
				videouri_ = value.as_string();
		}

		if (key == L"audiouri")
		{
			if(value.is_string())
				audiouri_ = value.as_string();
		}

		if (key == L"username")
		{
			if(value.is_string())
				username_ = value.as_string();
		}

		if (key == L"pass")
		{
			if(value.is_string())
				pass_ = value.as_string();
		}
	}
}


CameraConfMsg::~CameraConfMsg()
{
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
	for (web::json::value::iterator iter = jpayload.begin(); iter != jpayload.end(); iter++)
	{
		if (iter->first.as_string() == U("isActive"))
		{
			isActive_ = iter->second.as_bool();
		}
		else if (iter->first.as_string() == U("cameraType"))
		{
			cameraType_ = static_cast<CameraType>(iter->second.as_integer());
		}
		else if (iter->first.as_string() == U("videoFormat"))
		{
			videoFormat_ = static_cast<CameraVideoFormat>(iter->second.as_integer());
		}
		else if (iter->first.as_string() == U("cameraId"))
		{
			cameraId_ = iter->second.as_integer();
		}
		else if (iter->first.as_string() == U("cameraName"))
		{
			cameraName_ = iter->second.as_string();
		}
		else if (iter->first.as_string() == U("outFolder"))
		{
			outFolder_ = iter->second.as_string();
		}
		else if (iter->first.as_string() == U("recordLen"))
		{
			recordLen_ = iter->second.as_integer();
		}
		else if (iter->first.as_string() == U("recordingType"))
		{
			recordingType_ = static_cast<CameraVideoRecording>(iter->second.as_integer());
		}
		else if (iter->first.as_string() == U("videouri"))
		{
			videouri_ = iter->second.as_string();
		}
		else if (iter->first.as_string() == U("audiouri"))
		{
			audiouri_ = iter->second.as_string();
		}
		else if (iter->first.as_string() == U("username"))
		{
			username_ = iter->second.as_string();
		}
		else if (iter->first.as_string() == U("pass"))
		{
			pass_ = iter->second.as_string();
		}
	}
}

wstring CameraConfMsg::ToString() const
{
	web::json::value jObj;
	ToJsonValue(jObj);
	return jObj.to_string();
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
