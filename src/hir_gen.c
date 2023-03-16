#include <hir_gen.h>

p_hir_program hir_gen(const char *file_name) {
    yyscan_t scanner;
    extra_info extra = (extra_info) { .fs = NULL, .p_ast = NULL, };
    yylex_init_extra(&extra, &scanner);
    frontend_push_file(file_name, NULL, &extra, scanner);

    yyparse(scanner);
    p_hir_program p_ast = yyget_extra(scanner)->p_ast;

    yylex_destroy(scanner);
    return p_ast;
}

void hir_drop(p_hir_program p_hir) {
    hir_program_drop(p_hir);
}