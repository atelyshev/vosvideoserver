#include "stdafx.h"
#include <vosvideocommon/StringUtil.h>
#include "MediaFileException.h"
#include "WebmSegmenter.h"

using namespace std;
using namespace util;
using namespace vosvideo::mediafile;
using namespace mkvparser;

WebmSegmenter::WebmSegmenter(const std::wstring& wpath) : FileSegmenter(wpath)
{
	// Get parser header info
	string path = StringUtil::ToString(wpath);
	if (reader.Open(path.c_str())) 
	{
		throw MediaFileException("Filename is invalid or error while opening");
	}
	EBMLHeader ebmlHeader;
	int64_t pos = 0;
	ebmlHeader.Parse(&reader, pos);
	shared_ptr<Segment> seg_ptr;
	Segment* seg;

	int64_t ret = Segment::CreateInstance(&reader, pos, seg);
	if (ret) 
	{
		throw MediaFileException("Segment::CreateInstance() failed.");
	}
	seg_ptr.reset(seg);
}


WebmSegmenter::~WebmSegmenter()
{
}

void WebmSegmenter::GetManifest(const std::wstring&)
{

}

void WebmSegmenter::GetManifest(const web::json::value&)
{

}
