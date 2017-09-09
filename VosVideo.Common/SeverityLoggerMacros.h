#pragma once

#include <cassert>
#include <iostream>
#include <fstream>
#include <boost/shared_ptr.hpp>
#include <boost/log/common.hpp>
#include <boost/log/attributes.hpp>
#include <boost/log/sinks.hpp>
#include <boost/core/null_deleter.hpp>
#include <boost/log/attributes/scoped_attribute.hpp>
#include "LoggersCommon.h"

#define LOG_TRACE(txt) \
do { \
	BOOST_LOG_NAMED_SCOPE(__FUNCTION__); \
	src::severity_logger< severity_level > slg; \
	BOOST_LOG_SEV(slg, vv_trace) << txt; \
}while (0);

#define LOG_DEBUG(txt) \
do { \
	BOOST_LOG_NAMED_SCOPE(__FUNCTION__); \
	src::severity_logger< severity_level > slg; \
	BOOST_LOG_SEV(slg, vv_debug) << txt; \
}while (0);

#define LOG_WARNING(txt) \
do { \
	BOOST_LOG_NAMED_SCOPE(__FUNCTION__); \
	src::severity_logger< severity_level > slg; \
	BOOST_LOG_SEV(slg, vv_warning) << txt; \
}while (0);

#define LOG_ERROR(txt) \
do{ \
	BOOST_LOG_NAMED_SCOPE(__FUNCTION__); \
	src::severity_logger< severity_level > slg; \
	BOOST_LOG_SEV(slg, vv_error) << txt; \
}while (0);

#define LOG_CRITICAL(txt) \
do { \
	BOOST_LOG_NAMED_SCOPE(__FUNCTION__); \
	src::severity_logger< severity_level > slg; \
	BOOST_LOG_SEV(slg, vv_critical) << txt; \
}while (0);