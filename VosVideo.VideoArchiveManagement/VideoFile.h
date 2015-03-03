#pragma once

namespace vosvideo
{
	namespace archive
	{
		enum class VideoFileType
		{
			LocalDrive,
			SkyDrive,
			GoogleDrive,
			DropBox,
			MegaDrive
		};

		class VideoFile
		{
		public:
			VideoFile() : isValid_(false){}
			VideoFile(const std::wstring& id, 
				const std::wstring& path, 
				uint64_t duration, 
				uint64_t startTime, 
				bool isSeekable) : 
				id_(id),
				path_(path),
				duration_(duration),
				startTime_(startTime),
				isSeekable_(isSeekable)
			{}

			virtual bool IsValid(){ return isValid_; }
			virtual std::wstring GetPath() { return path_; }
			virtual VideoFileType FileType() { return fileType_; };
			virtual bool IsSeekable() { return isSeekable_; }
			virtual uint64_t Duration() { return duration_; }
			virtual uint64_t StartTime() { return startTime_; }
			virtual std::wstring GetId() { return id_; }

		protected:
			bool isValid_;
			std::wstring path_;
			std::wstring id_;
			uint64_t duration_;
			uint64_t startTime_;
			bool isSeekable_;
			VideoFileType fileType_;
		};
	}
}