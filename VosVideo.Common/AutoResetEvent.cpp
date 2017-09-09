#include "stdafx.h"
#include "AutoResetEvent.h"

using namespace threading;

AutoResetEvent::AutoResetEvent(bool initial)
	: flag_(initial)
{
}

void AutoResetEvent::Set()
{
	std::lock_guard<std::mutex> _(protect_);
	flag_ = true;
	signal_.notify_one();
}

void AutoResetEvent::Reset()
{
	std::lock_guard<std::mutex> _(protect_);
	flag_ = false;
}

bool AutoResetEvent::WaitOne()
{
	std::unique_lock<std::mutex> lk(protect_);
	while( !flag_ ) // prevent spurious wakeups from doing harm
	{
		signal_.wait(lk);
	}
	flag_ = false; // waiting resets the flag
	return true;
}
