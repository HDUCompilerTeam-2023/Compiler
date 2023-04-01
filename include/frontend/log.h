#ifndef __HIR_GEN_LOG__
#define __HIR_GEN_LOG__

#include <frontend/parser.h>

typedef enum {
    debug,
    log,
    err,
} loglevel;

int yylog(loglevel level, YYLTYPE *loc, yyscan_t scanner, const char *format, ...);

#define yyerror(loc, scan, fmt, ...) yylog(err, loc, scan, fmt, ##__VA_ARGS__)
#define yydebug(loc, scan, fmt, ...) yylog(debug, loc, scan, fmt, ##__VA_ARGS__)

#endif