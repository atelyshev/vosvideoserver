#include "stdafx.h"
#include <boost/filesystem.hpp>
#include <vosvideocommon/StringUtil.h>
#include "DirectoryChangesNotifier.h"
#include "WinDirectoryMonitor.h"

using namespace std;
using namespace util;
using namespace vosvideo::archive;
using namespace vosvideo::configuration;

DWORD WINAPI LocalDirectoryMonitorStarter(LPVOID lpParam)
{
	WinDirectoryMonitor* directoryWatcher = static_cast<WinDirectoryMonitor*>(lpParam);
	directoryWatcher->DoMonitorArchiveDirectory();
	return 0;
}

WinDirectoryMonitor::WinDirectoryMonitor(std::shared_ptr<ConfigurationManager> configManager, DirectoryChangesNotifier* changesNotifier) :
		changesNotifier_(changesNotifier),
		configManager_(configManager),
		inRemovingState_(false)
{
	thrHandle_ = CreateThread(NULL, 0, &LocalDirectoryMonitorStarter, this, 0, nullptr);
}

WinDirectoryMonitor::~WinDirectoryMonitor()
{
	SetEvent(endLocalDirectoryEvent_);
	CloseHandle(endLocalDirectoryEvent_);
	CloseHandle(thrHandle_);
}

// dwAction meaning
//FILE_ACTION_ADDED            : "Added";
//FILE_ACTION_REMOVED          : "Deleted";
//FILE_ACTION_MODIFIED         : "Modified";
//FILE_ACTION_RENAMED_OLD_NAME : "Renamed From"
//FILE_ACTION_RENAMED_NEW_NAME : "Renamed To"

void WinDirectoryMonitor::DoMonitorArchiveDirectory()
{
	if (configManager_->GetArchivePath() == L"")
	{
		return;
	}

	const DWORD dwNotificationFlags =
		FILE_NOTIFY_CHANGE_LAST_WRITE
		| FILE_NOTIFY_CHANGE_CREATION
		| FILE_NOTIFY_CHANGE_FILE_NAME;

	CReadDirectoryChanges changes;
	changes.AddDirectory(configManager_->GetArchivePath(), false, dwNotificationFlags);
	endLocalDirectoryEvent_ = CreateEvent(
		NULL,               // default security attributes
		TRUE,               // manual-reset event
		FALSE,              // initial state is nonsignaled
		TEXT("WriteEvent")  // object name
		);
	const HANDLE handles[] = { endLocalDirectoryEvent_, changes.GetWaitHandle() };

	bool bTerminate = false;

	while (!bTerminate)
	{
		DWORD rc = ::WaitForMultipleObjectsEx(_countof(handles), handles, false, INFINITE, true);
		switch (rc)
		{
		case WAIT_OBJECT_0:
			// signaled to finish thread
			bTerminate = true;
			break;

		case WAIT_OBJECT_0 + 1:
			//We've received a notification in the queue.
		{
			uint32_t dwAction;
			wstring wstrFilename;
			if (!changes.CheckOverflow())
			{
				changes.Pop(dwAction, wstrFilename);
				if (dwAction == FILE_ACTION_MODIFIED)
				{
					ChangeDiskSpace(wstrFilename);
				}
				else if (dwAction == FILE_ACTION_ADDED)
				{
					changesNotifier_->notifierSignal_(wstrFilename);
				}
			}
		}
		break;
		case WAIT_IO_COMPLETION:
			//Nothing to do.
			break;
		}
	}

	changes.Terminate();
}

void WinDirectoryMonitor::ChangeDiskSpace(const wstring& wstrDir)
{
	if (inRemovingState_ == true)
	{
		inRemovingState_ = false;
		return;
	}

	LOG_TRACE("Created new file in video archive: " << StringUtil::ToString(wstrDir));

	boost::filesystem::path p(wstrDir);
	if (!p.empty())
	{
		boost::filesystem::path::iterator it = p.begin();
		boost::filesystem::space_info si = boost::filesystem::space(*it);

		// Check if time to remove old files
		if (si.available < reservedDiskSpace_)
		{
			LOG_TRACE("Available disk space is less then 10G, will remove oldest file");
			boost::posix_time::ptime nowTime(boost::posix_time::second_clock::local_time());
			boost::filesystem::directory_iterator dirIter(p), dirIterEnd;
			map<time_t, boost::filesystem::path> files;

			while (dirIter != dirIterEnd)
			{
				if (boost::filesystem::exists(*dirIter) && !boost::filesystem::is_directory(*dirIter))
				{
					time_t t = boost::filesystem::last_write_time(*dirIter);
					auto p = dirIter->path();
					files.insert(make_pair(t, dirIter->path()));
				}
				++dirIter;
			}

			if (!files.empty())
			{
				LOG_TRACE("File was removed: " << files.begin()->second);
				boost::filesystem::remove(files.begin()->second);
				inRemovingState_ = true;
			}
		}
	}
}
