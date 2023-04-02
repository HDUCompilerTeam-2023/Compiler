#include <util/log.h>

#include <stdarg.h>
#include <stdio.h>

int LOG(const char *tag, const char *pos, const char *format, ...) {
    va_list args;
    int ret;
    FILE *output = stdout;

    fprintf(output, "[%s] ", tag);

    fprintf(output, "%s: ", pos);

    va_start(args, format);
    ret = vfprintf(output, format, args);
    va_end(args);
    fprintf(output, "\n");
    return ret;
}