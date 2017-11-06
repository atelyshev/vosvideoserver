#include "stdafx.h"
#include <boost/lexical_cast.hpp>
#include <webrtc/modules/video_capture/video_capture_factory.h>
#include <webrtc/modules/video_capture/video_capture_defines.h>
#include <webrtc/base/logging.h>
#include <vpx/vpx_encoder.h>
#include <webrtc/media/engine/webrtcvideocapturer.h>
#include <webrtc/base/arraysize.h>
#include "CameraVideoCapturer.h"
#include "CameraVideoCaptureImpl.h"

using namespace std;
using namespace cricket;
using namespace webrtc;

using boost::lexical_cast;
using boost::bad_lexical_cast;
using vosvideo::camera::CameraVideoCapturer;
using vosvideo::camera::CameraVideoCaptureImpl;
using vosvideo::cameraplayer::CameraPlayerBase;


struct kVideoFourCCEntry 
{
	uint32_t fourcc;
	webrtc::VideoType webrtc_type;
};

// This indicates our format preferences and defines a mapping between
// webrtc::RawVideoType (from video_capture_defines.h) to our FOURCCs.
static kVideoFourCCEntry kSupportedFourCCs[] = 
{
	{ FOURCC_I420, webrtc::VideoType::kI420 },   // 12 bpp, no conversion.
	{ FOURCC_YV12, webrtc::VideoType::kYV12 },   // 12 bpp, no conversion.
	{ FOURCC_NV12, webrtc::VideoType::kNV12 },   // 12 bpp, fast conversion.
	{ FOURCC_NV21, webrtc::VideoType::kNV21 },   // 12 bpp, fast conversion.
	{ FOURCC_YUY2, webrtc::VideoType::kYUY2 },   // 16 bpp, fast conversion.
	{ FOURCC_UYVY, webrtc::VideoType::kUYVY },   // 16 bpp, fast conversion.
	{ FOURCC_MJPG, webrtc::VideoType::kMJPEG },  // compressed, slow conversion.
	{ FOURCC_ARGB, webrtc::VideoType::kARGB },   // 32 bpp, slow conversion.
	{ FOURCC_24BG, webrtc::VideoType::kRGB24 },  // 24 bpp, slow conversion.
};


static bool CapabilityToFormat(const webrtc::VideoCaptureCapability& cap, VideoFormat* format) 
{
	uint32_t fourcc = 0;
	for (size_t i = 0; i < arraysize(kSupportedFourCCs); ++i) 
	{
		if (kSupportedFourCCs[i].webrtc_type == cap.videoType) 
		{
			fourcc = kSupportedFourCCs[i].fourcc;
			break;
		}
	}
	if (fourcc == 0) 
	{
		return false;
	}

	format->fourcc = fourcc;
	format->width = cap.width;
	format->height = cap.height;
	format->interval = VideoFormat::FpsToInterval(cap.maxFPS);
	return true;
}

static bool FormatToCapability(const VideoFormat& format, webrtc::VideoCaptureCapability* cap) 
{
	webrtc::VideoType webrtc_type = webrtc::VideoType::kUnknown;
	for (size_t i = 0; i < arraysize(kSupportedFourCCs); ++i)
	{
		if (kSupportedFourCCs[i].fourcc == format.fourcc) 
		{
			webrtc_type = kSupportedFourCCs[i].webrtc_type;
			break;
		}
	}
	if (webrtc_type == webrtc::VideoType::kUnknown)
	{
		return false;
	}

	cap->width = format.width;
	cap->height = format.height;
	cap->maxFPS = VideoFormat::IntervalToFps(format.interval);
	cap->videoType= webrtc_type;
	cap->interlaced = false;
	return true;
}

CameraVideoCapturer::~CameraVideoCapturer() 
{
	delete videoCapturerImpl_;
}

bool CameraVideoCapturer::Init(int camId, CameraPlayerBase* device) 
{
	if (videoCapturerImpl_ != nullptr)
	{
		LOG(LS_ERROR) << "The capturer is already initialized";
		return false;
	}

	videoCapturerImpl_ = new CameraVideoCaptureImpl(device);
	SetId(std::to_string(camId));

	webrtc::VideoCaptureCapability cap;
	device->GetWebRtcCapability(cap);
	cap.videoType = webrtc::VideoType::kI420;
	vector<VideoFormat> supported;
	VideoFormat format;

	if (CapabilityToFormat(cap, &format)) 
	{
		supported.push_back(format);
	}
    SetSupportedFormats(supported);

	return true;
}

bool CameraVideoCapturer::Init(const Device& device) 
{
	throw exception("Unsupported called to this version of Init");
}

bool CameraVideoCapturer::Init(webrtc::VideoCaptureModule* module) 
{
	throw exception("Unsupported called to this version of Init");
}

CaptureState CameraVideoCapturer::Start(const VideoFormat& capture_format) 
{
	if (videoCapturerImpl_ == nullptr)
	{
		LOG(LS_ERROR) << "The capturer has not been initialized";
		return CS_FAILED;
	}

	// TODO(hellner): weird to return failure when it is in fact actually running.
	if (IsRunning()) 
	{
		LOG(LS_ERROR) << "The capturer is already running";
		return CS_FAILED;
	}

	SetCaptureFormat(&capture_format);
	webrtc::VideoCaptureCapability cap;

	if (!FormatToCapability(capture_format, &cap)) 
	{
		LOG(LS_ERROR) << "Invalid capture format specified";
		return CS_FAILED;
	}

	string camera_id(GetId());
	uint32_t start = (uint32_t)rtc::Time();
	videoCapturerImpl_->RegisterCaptureDataCallback(this);

	if (videoCapturerImpl_->StartCapture(cap) != 0)
	{
		LOG(LS_ERROR) << "Camera '" << camera_id << "' failed to start";
		return CS_FAILED;
	}

	LOG(LS_INFO) << "Camera '" << camera_id << "' started with format "<< capture_format.ToString() << ", elapsed time "<< rtc::TimeSince(start) << " ms";

	captured_frames_ = 0;
	SetCaptureState(CS_RUNNING);
	return CS_STARTING;
}

void CameraVideoCapturer::Stop() 
{
	if (IsRunning()) 
	{
//		rtc::Thread::Current()->Clear(this);
		videoCapturerImpl_->StopCapture();
		videoCapturerImpl_->DeRegisterCaptureDataCallback();

		// TODO(juberti): Determine if the VCM exposes any drop stats we can use.
		double drop_ratio = 0.0;
		string camera_id(GetId());
		LOG(LS_INFO) << "Camera '" << camera_id << "' stopped after capturing "<< captured_frames_ << " frames and dropping "<< drop_ratio << "%";
	}
	SetCaptureFormat(nullptr);
}

bool CameraVideoCapturer::IsRunning() 
{
	return (videoCapturerImpl_ != nullptr && videoCapturerImpl_->CaptureStarted());
}

bool CameraVideoCapturer::GetPreferredFourccs(vector<uint32_t>* fourccs) 
{
	if (!fourccs) 
	{
		return false;
	}

	fourccs->clear();
	for (size_t i = 0; i < arraysize(kSupportedFourCCs); ++i)
	{
		fourccs->push_back(kSupportedFourCCs[i].fourcc);
	}
	return true;
}

void CameraVideoCapturer::OnFrame(const webrtc::VideoFrame& sample)
{
    ++captured_frames_;
	VideoCapturer::OnFrame(sample, sample.width(), sample.height());
}

