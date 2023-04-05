#ifndef __UTIL_LOG__
#define __UTIL_LOG__

#include <stdarg.h>

typedef enum {
    debug,
    info,
    error,
} loglevel;

void set_loglevel(loglevel level);

int LOG(loglevel level, const char *tag, const char *pos, const char *format, ...);
int vaLOG(loglevel level, const char *tag, const char *pos, const char *format, __gnuc_va_list va_list);

#endif