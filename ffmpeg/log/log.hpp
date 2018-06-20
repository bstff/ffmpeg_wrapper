#ifndef _COMMON_WRAP_LOG_H_
#define _COMMON_WRAP_LOG_H_

#ifdef USE_LOG
#include "thirdparty/spdlog/spdlog.h"
#endif

namespace logif{
#ifdef USE_LOG
	void CreateLogger(const char *log_file, const bool show_stdout);
	void CreateLogger(const char *dir, const char *name, const bool show_stdout);
	void DestroyLogger();
#endif
	void logIt(const char *fmt, ...);
}
#endif