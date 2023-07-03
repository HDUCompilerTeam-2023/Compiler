#include <frontend/log.h>

#include <stdarg.h>
#include <util/log.h>
#include <stdio.h>

int yylloc2str(char *yyo, YYLTYPE *yylocp) {
    int res = 0;
    int end_col = 0 != yylocp->last_column ? yylocp->last_column - 1 : 0;
    if (0 <= yylocp->first_line) {
        res += sprintf (yyo + res, "%d", yylocp->first_line);
        if (0 <= yylocp->first_column)
            res += sprintf (yyo + res, ".%d", yylocp->first_column);
    }
    if (0 <= yylocp->last_line) {
        if (yylocp->first_line < yylocp->last_line) {
            res += sprintf (yyo + res, "-%d", yylocp->last_line);
            if (0 <= end_col)
                res += sprintf (yyo + res, ".%d", end_col);
        }
        else if (0 <= end_col && yylocp->first_column < end_col)
            res += sprintf (yyo + res, "-%d", end_col);
    }
    yyo[res++] = '\0';
    return res;
}

void frontend_log(loglevel level, YYLTYPE *yylloc, yyscan_t yyscanner, const char *format, ...) {
    char pos[50];
    yylloc2str(pos, yylloc);

    va_list args;
    va_start(args, format);
    vaLOG(level, "frontend", pos, format, args);
    va_end(args);
}