#include <lexer.h>

#include <frontend/log.h>

void yyopen_file(const char *file_name, yyscan_t *scanner) {
    yylex_init(scanner);
    FILE *fp = stdin;
    if (file_name) {
        yylog(log, NULL, *scanner, "Lexer, open file \"%s\"", file_name);
        fp = fopen(file_name, "r");
    }
    yyrestart(fp, *scanner);
}

void yyclose_file(yyscan_t *scanner) {
    yylex_destroy(*scanner);
}