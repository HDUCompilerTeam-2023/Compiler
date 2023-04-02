#include <frontend/log.h>

#include <util/log.h>
#include <stdarg.h>

void frontend_log(loglevel level, YYLTYPE *yylloc, yyscan_t yyscanner, const char *format, ...) {
    va_list args;
    const char *tag;
    switch (level) {
    case error:
        tag = "ERROR";
        break;
    case log:
    case trace:
        tag = "FRONTEND";
        break;
    }

    const char *pos = "Line 1";

    va_start(args, format);
    LOG(tag, pos, format, args);
    va_end(args);
}