#ifndef __FRONTEND_LOG__
#define __FRONTEND_LOG__

#include <stdio.h>

typedef enum {
    debug,
    log,
    err,
} loglevel;

int yylog(loglevel level, const char *format, ...);

#define yyerror(fmt, ...) yylog(err, fmt, ##__VA_ARGS__)
#define yydebug(fmt, ...) yylog(debug, fmt, ##__VA_ARGS__)

#endif