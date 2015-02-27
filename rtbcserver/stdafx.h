// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once
#define WIN32_LEAN_AND_MEAN

#include "targetver.h"

#include <cstdio>
#include <tchar.h>

// reference additional headers your program requires here
#include <strsafe.h>
#include <atlstr.h>
#include <windows.h>

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <stdexcept>
#include <deque>
#include <map>
#include <set>
#include <atomic>
#include <iostream>
#include <thread>
#include <list>

#include <boost/math/tools/rational.hpp>
#include <boost/signals2.hpp>
#include <boost/filesystem.hpp>

#include <vosvideocommon/EventLogLogger.h>
#include <vosvideocommon/SeverityLogger.h>
#include <vosvideocommon/SeverityLoggerMacros.h>

#pragma warning(disable: 4251)
#pragma warning(disable: 4275)
#pragma warning(disable: 4996)

