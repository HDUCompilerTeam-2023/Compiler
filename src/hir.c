#include <hir.h>
#include <stdlib.h>

#include <hir/syntaxtree_printer.h>
#include <hir/log.h>

pExtraInfo frontend_init_extra() {
    pExtraInfo ret = malloc(sizeof(*ret));
    *ret = (ExtraInfo) { .root = NULL, .fs = NULL };
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


void yyopen_file(const char *file_name, YYLTYPE *loc, YY_EXTRA_TYPE extra, yyscan_t scanner) {
    FILE *f;
    if (file_name) f = fopen(file_name, "r");
    else f = stdin;
    if (!f) {
        yyerror(loc, scanner, "preprocess error, can not open file \"%s\"", file_name);
        return;
    }

    if (extra->fs) extra->fs->loc = loc;

    pfile_stack fs = malloc(sizeof(*fs));
    fs->prev = extra->fs;
    extra->fs = fs;

    fs->bs = yy_create_buffer(f, YY_BUF_SIZE, scanner);
    yy_switch_to_buffer(fs->bs, scanner);
    yyset_lloc(&(YYLTYPE){1, 1, 1, 1}, scanner);
    fs->f = f;

    if (file_name) {
        fs->filename = malloc(strlen(file_name) + 1);
        strcpy(fs->filename, file_name);
        yylog(log, yyget_lloc(scanner), scanner, "in %s", extra->fs->filename);
    }
    else {
        fs->filename = NULL;
        yylog(log, yyget_lloc(scanner), scanner, "in stdin");
    }
}

bool yypop_file(YY_EXTRA_TYPE extra, yyscan_t scanner) {
    assert(extra->fs);

    fclose(extra->fs->f);
    yy_delete_buffer(extra->fs->bs, scanner);
    
    pfile_stack prev_fs = extra->fs->prev;
    if (extra->fs->filename)
        free(extra->fs->filename);
    free(extra->fs);
    extra->fs = prev_fs;

    if (!extra->fs) return true;

    yy_switch_to_buffer(extra->fs->bs, scanner);
    yyset_lloc(extra->fs->loc, scanner);

    if (extra->fs->filename) {
        yylog(log, yyget_lloc(scanner), scanner, "in %s", extra->fs->filename);
    }
    else {
        yylog(log, yyget_lloc(scanner), scanner, "in stdin");
    }
    return false;
}

void yyinit(const char *file_name, yyscan_t *scanner) {
    pExtraInfo extra = frontend_init_extra();
    yylex_init_extra(extra, scanner);
    yyopen_file(file_name, NULL, extra, *scanner);
}

void yyexit(yyscan_t *scanner) {
    frontend_drop_extra(yyget_extra(*scanner));
    yylex_destroy(*scanner);
}