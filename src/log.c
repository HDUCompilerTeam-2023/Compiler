#include <log.h>

int yylog(loglevel level, const char *format, ...) {
    va_list args;
    int ret;
    FILE *output = stdout;

    switch (level) {
    case err:
        output = stderr;
        fprintf(output, "[error] ");
    case debug:
#ifndef DEBUG
        ret = 0;
        break;
#endif
    case log:
        va_start(args, format);
        ret = vfprintf(output, format, args);
        va_end(args);
        fprintf(output, "\n");
    };
    return ret;
}