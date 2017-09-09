#include "stdafx.h"
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
	if (reader_.Open(path.c_str()))
	{
		throw MediaFileException("Filename is invalid or error while opening");
	}
	EBMLHeader ebmlHeader;
	int64_t pos = 0;
	ebmlHeader.Parse(&reader_, pos);
	shared_ptr<Segment> seg_ptr;
	Segment* seg;

	int64_t ret = Segment::CreateInstance(&reader_, pos, seg);
	if (ret)
	{
		throw MediaFileException("Segment::CreateInstance() failed.");
	}

	seg_ptr.reset(seg);

	ret = seg_ptr->Load();
	if (ret)
	{
		throw MediaFileException("Segment::Load() failed.");
	}

	const unsigned long clusterCount = seg_ptr->GetCount();
	if (clusterCount > 0)
	{
		const SegmentInfo* const pSegmentInfo = seg_ptr->GetInfo();
		int64_t duration_ns = pSegmentInfo->GetDuration();
		const mkvparser::Cluster* pCluster = seg_ptr->GetFirst();
		const mkvparser::Cues* const cues = seg_ptr->GetCues();
		manifest_.reset(new MediaFileManifest(seg_ptr->m_size + seg_ptr->m_start, duration_ns, wpath));

		do
		{
			MediaCluster mc(pCluster->GetTimeCode(), pCluster->GetPosition() + seg_ptr->m_start);
			manifest_->AddMediaCluster(mc);
		} while ((pCluster = seg_ptr->GetNext(pCluster)) != &seg_ptr->m_eos);
	}
	reader_.Close();
}


WebmSegmenter::~WebmSegmenter()
{
	reader_.Close();
}

std::shared_ptr<MediaFileManifest> WebmSegmenter::GetManifest()
{
	return manifest_;
}
