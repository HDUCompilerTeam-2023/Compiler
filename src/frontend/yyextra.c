#include <frontend/yyextra.h>

#include <frontend/lexer.h>
#include <frontend/log.h>

struct file_stack {
    p_file_stack prev;
    YY_BUFFER_STATE bs;
    YYLTYPE *loc;
    char *filename;
    FILE *f;
};

void frontend_push_file(const char *file_name, YYLTYPE *loc, YY_EXTRA_TYPE extra, yyscan_t scanner) {
    FILE *f;
    if (file_name) f = fopen(file_name, "r");
    else f = stdin;
    if (!f) {
        yyerror(loc, scanner, "preprocess error, can not open file \"%s\"", file_name);
        return;
    }

    if (extra->fs) extra->fs->loc = loc;

    p_file_stack fs = malloc(sizeof(*fs));
    fs->prev = extra->fs;
    extra->fs = fs;

    fs->bs = yy_create_buffer(f, YY_BUF_SIZE, scanner);
    yy_switch_to_buffer(fs->bs, scanner);
    yyset_lloc(&(YYLTYPE){1, 1, 1, 1}, scanner);
    fs->f = f;

    if (file_name) {
        fs->filename = malloc(strlen(file_name) + 1);
        strcpy(fs->filename, file_name);
        yydebug(yyget_lloc(scanner), scanner, "in %s", extra->fs->filename);
    }
    else {
        fs->filename = NULL;
        yydebug(yyget_lloc(scanner), scanner, "in stdin");
    }
}

bool frontend_pop_file(YY_EXTRA_TYPE extra, yyscan_t scanner) {
    assert(extra->fs);

    fclose(extra->fs->f);
    yy_delete_buffer(extra->fs->bs, scanner);
    
    p_file_stack prev_fs = extra->fs->prev;
    if (extra->fs->filename)
        free(extra->fs->filename);
    free(extra->fs);
    extra->fs = prev_fs;

    if (!extra->fs) return true;

    yy_switch_to_buffer(extra->fs->bs, scanner);
    yyset_lloc(extra->fs->loc, scanner);

    if (extra->fs->filename) {
        yydebug(yyget_lloc(scanner), scanner, "in %s", extra->fs->filename);
    }
    else {
        yydebug(yyget_lloc(scanner), scanner, "in stdin");
    }
    return false;
}