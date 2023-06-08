#include <frontend/lexer.h>
#include <frontend/parser.h>
#include <frontend/syntax_gen.h>

#include <program/gen.h>

p_program frontend_trans(const char *file_name) {
    yyscan_t scanner;
    extra_info extra = (extra_info) {
        .fs = NULL,
        .p_info = NULL,
    };
    yylex_init_extra(&extra, &scanner);
    frontend_push_file(file_name, NULL, &extra, scanner);

    yyparse(scanner);
    p_program p_program = yyget_extra(scanner)->p_info->p_program;
    program_mir_set_vmem_id(p_program);
    syntax_info_drop(yyget_extra(scanner)->p_info);

    yylex_destroy(scanner);
    return p_program;
}
