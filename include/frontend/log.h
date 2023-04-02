#ifndef __FRONTEND_LOG__
#define __FRONTEND_LOG__

#include <frontend/parser.h>

typedef enum {
    trace,
    log,
    error,
} loglevel;

void frontend_log(loglevel level, YYLTYPE *loc, yyscan_t scanner, const char *format, ...);

#define yyerror(loc, scan, fmt, ...) frontend_log(error, loc, scan, fmt, ##__VA_ARGS__)
#define yylog(loc, scan, fmt, ...) frontend_log(log, loc, scan, fmt, ##__VA_ARGS__)

#endif