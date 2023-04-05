#include <util/log.h>

#include <stdarg.h>
#include <stdio.h>

int LOG(const char *tag, const char *pos, const char *format, ...) {
    va_list args;
    va_start(args, format);
    int ret = vaLOG(tag, pos, format, args);
    va_end(args);
    return ret;
}

int vaLOG(const char *tag, const char *pos, const char *format, __gnuc_va_list args) {
    FILE *output = stdout;

    fprintf(output, "[%s] ", tag);

    fprintf(output, "%s: ", pos);

    int ret = vfprintf(output, format, args);
    fprintf(output, "\n");
    return ret;
}