#include "stdafx.h"
#include <cpprest/json.h>
#include "CameraConf.h"

using namespace std;
using namespace vosvideo::camera;

CameraConf::CameraConf() :
	cameraType_(CameraType::UNKNOWN),
	videoFormat_(CameraVideoFormat::UNKNOWN),
	recordingType_(CameraVideoRecording::DISABLED), // Do nothing by default
	cameraId_(-1),
	recordLen_(-1),
	isActive_(false)
{
}

CameraConf::CameraConf(CameraType ct, CameraVideoFormat vf) : 
	cameraType_(ct),
	videoFormat_(vf), 
	recordingType_(CameraVideoRecording::DISABLED), // Do nothing by default
	cameraId_(-1),
	recordLen_(-1),
	isActive_(false)
{
}


CameraConf::~CameraConf()
{
}

CameraVideoFormat CameraConf::GetVideoFormat()
{
	return videoFormat_;
}

CameraType CameraConf::GetCameraType()
{
	return cameraType_;
}

void CameraConf::SetCameraIds(int cameraId, const wstring& cameraName)
{
	cameraId_ = cameraId;
	cameraName_ = cameraName;
}

void CameraConf::GetCameraIds(int& cameraId, wstring& cameraName) const
{
	cameraId = cameraId_;
	cameraName = cameraName_;
}

void CameraConf::SetFileSinkParameters(const wstring& outFolder, int recordLen, CameraVideoRecording recordingType)
{
	outFolder_ = outFolder;
	recordLen_ = recordLen;
	recordingType_ = recordingType;
}

void CameraConf::GetFileSinkParameters(wstring& outFolder, int& recordLen, CameraVideoRecording& recordingType) const
{
	outFolder = outFolder_;
	recordLen = recordLen_;
	recordingType = recordingType_;
}

void CameraConf::SetUris(const wstring& audiouri, const wstring& videouri)
{
	audiouri_ = audiouri;
	videouri_ = videouri;
}

void CameraConf::GetUris(wstring& audiouri, wstring& videouri) const
{
	audiouri = audiouri_;
	videouri = videouri_;
}

void CameraConf::SetCredentials(const wstring& username, const wstring& pass)
{
	username_ = username;
	pass_ = pass;
}

void CameraConf::GetCredentials(wstring& username, wstring& pass) const
{
	username = username_;
	pass = pass_;
}

void CameraConf::SetIsActive(bool isActive)
{
	isActive_ = isActive;
}

bool CameraConf::GetIsActive()
{
	return isActive_;
}

wstring CameraConf::ToString() const
{
	web::json::value jObj;
	jObj[L"mt"] = web::json::value::number(static_cast<int>(vosvideo::data::InMsgType::CameraConfMsg));
	jObj[L"isActive"] = web::json::value::boolean(isActive_);
	jObj[L"cameraType"] = web::json::value::number(static_cast<int>(cameraType_));
	jObj[L"videoFormat"] = web::json::value::number(static_cast<int>(videoFormat_));
	jObj[L"cameraId"] = web::json::value::number(cameraId_);
	jObj[L"cameraName"] = web::json::value::string(cameraName_);
	jObj[L"outFolder"] = web::json::value::string(outFolder_);
	jObj[L"recordLen_"] = web::json::value::number(recordLen_);
	jObj[L"recordingType"] = web::json::value::number(static_cast<int>(recordingType_));
	jObj[L"videouri"] = web::json::value::string(videouri_);
	jObj[L"audiouri"] = web::json::value::string(audiouri_);
	jObj[L"username"] = web::json::value::string(username_);
	jObj[L"pass"] = web::json::value::string(pass_);
	return jObj.to_string();
}

void CameraConf::ToObject(const std::wstring& jsonStr, CameraConf& objRef)
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
				objRef.isActive_ = value.as_bool();
		}

		if (key == L"cameraType")
		{
			if(value.is_string())
				objRef.cameraType_ = static_cast<CameraType>(value.as_integer());
		}

		if (key == L"videoFormat")
		{
			if(value.is_string())
				objRef.videoFormat_ = static_cast<CameraVideoFormat>(value.as_integer());
		}

		if (key == L"cameraId")
		{
			if(value.is_string())
				objRef.cameraId_ = value.as_integer();
		}

		if (key == L"cameraName")
		{
			if(value.is_string())
				objRef.cameraName_ = value.as_string();
		}

		if (key == L"outFolder")
		{
			if(value.is_string())
				objRef.outFolder_ = value.as_string();
		}

		if (key == L"recordLen")
		{
			if(value.is_string())
				objRef.recordLen_ = value.as_integer();
		}

		if (key == L"recordingType")
		{
			if(value.is_string())
				objRef.recordingType_ = static_cast<CameraVideoRecording>(value.as_integer());
		}

		if (key == L"videouri")
		{
			if(value.is_string())
				objRef.videouri_ = value.as_string();
		}

		if (key == L"audiouri")
		{
			if(value.is_string())
				objRef.audiouri_ = value.as_string();
		}

		if (key == L"username")
		{
			if(value.is_string())
				objRef.username_ = value.as_string();
		}

		if (key == L"pass")
		{
			if(value.is_string())
				objRef.pass_ = value.as_string();
		}
	}
}

bool CameraConf::operator==(const CameraConf &other) const
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

bool CameraConf::operator!=(const CameraConf &other) const 
{
	return !(*this == other);
}

CameraConf& CameraConf::operator=(const CameraConf& other)
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
