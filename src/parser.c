#include <parser.h>
#include <stdlib.h>

#include <frontend/syntaxtree_printer.h>
#include <frontend/log.h>

pExtraInfo frontend_init_extra() {
    pExtraInfo ret = malloc(sizeof(*ret));
    *ret = (ExtraInfo) { .root = NULL };
    return ret;
}

void frontend_drop_extra(pExtraInfo ExtraInfo) {
    if (!ExtraInfo) return;
    if (ExtraInfo->root) {
        frontend_print_syntaxtree(ExtraInfo->root);
        frontend_drop_syntaxtree(ExtraInfo->root);
    }
    free(ExtraInfo);
}

void yyopen_file(const char *file_name, yyscan_t *scanner) {
    yylex_init_extra(frontend_init_extra(), scanner);
    FILE *fp = stdin;
    if (file_name) {
        yylog(log, NULL, *scanner, "Lexer, open file \"%s\"", file_name);
        fp = fopen(file_name, "r");
    }
    yyrestart(fp, *scanner);
}

void yyclose_file(yyscan_t *scanner) {
    frontend_drop_extra(yyget_extra(*scanner));
    yylex_destroy(*scanner);
}