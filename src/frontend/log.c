#include <frontend/log.h>

#include <util/log.h>
#include <stdarg.h>

void frontend_log(loglevel level, YYLTYPE *yylloc, yyscan_t yyscanner, const char *format, ...) {
    const char *pos = "Line 1";

    va_list args;
    va_start(args, format);
    vaLOG(level, "frontend", pos, format, args);
    va_end(args);
}