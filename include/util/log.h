#ifndef __UTIL_LOG__
#define __UTIL_LOG__

#include <stdarg.h>

int LOG(const char *tag, const char *pos, const char *format, ...);
int vaLOG(const char *tag, const char *pos, const char *format, __gnuc_va_list va_list);

#endif