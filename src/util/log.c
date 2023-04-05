#include <util/log.h>

#include <stdarg.h>
#include <stdio.h>

static loglevel lowestlevel = info;

void set_loglevel(loglevel level) {
    lowestlevel = level;
}

int LOG(loglevel level, const char *tag, const char *pos, const char *format, ...) {
    va_list args;
    va_start(args, format);
    int ret = vaLOG(level, tag, pos, format, args);
    va_end(args);
    return ret;
}

int vaLOG(loglevel level, const char *tag, const char *pos, const char *format, __gnuc_va_list args) {
    if (level < lowestlevel) return 0;

    FILE *output = stdout;

    switch (level) {
    case error:
        output = stderr;
        fprintf(output, "[Error] ");
        break;
    case info:
        fprintf(output, "[Info] ");
        break;
    case debug:
        fprintf(output, "[Ddbug] ");
        break;
    }

    fprintf(output, "[%s] ", tag);

    if (pos) fprintf(output, "%s ", pos);

    fprintf(output, ": ");

    int ret = vfprintf(output, format, args);
    fprintf(output, "\n");
    return ret;
}