#include <frontend/log.h>

#include <stdarg.h>
#include <stdio.h>

int yylog(loglevel level, YYLTYPE *loc, yyscan_t scanner, const char *format, ...) {
    va_list args;
    int ret;
    FILE *output = stdout;

    if (level == err) {
        output = stderr;
    }

    switch (level) {
    case err:
        fprintf(output, "[Error] ");
        break;
    case log:
        fprintf(output, "[Trace] ");
        break;
    case debug:
        fprintf(output, "[Debug] ");
        break;
    }

    if (loc) {
        fprintf(output, "At line %d, ", loc->first_line);
    }

    va_start(args, format);
    ret = vfprintf(output, format, args);
    va_end(args);
    fprintf(output, "\n");
    return ret;
}