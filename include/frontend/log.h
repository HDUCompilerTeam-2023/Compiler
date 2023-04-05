#ifndef __FRONTEND_LOG__
#define __FRONTEND_LOG__

#include <frontend/parser.h>

#include <util/log.h>

void frontend_log(loglevel level, YYLTYPE *loc, yyscan_t scanner, const char *format, ...);

#define yyerror(loc, scan, fmt, ...) frontend_log(error, loc, scan, fmt, ##__VA_ARGS__)
#define yyinfo(loc, scan, fmt, ...) frontend_log(info, loc, scan, fmt, ##__VA_ARGS__)
#define yydebug(loc, scan, fmt, ...) frontend_log(debug, loc, scan, fmt, ##__VA_ARGS__)

#endif