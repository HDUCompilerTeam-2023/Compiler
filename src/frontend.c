#include <frontend/lexer.h>
#include <frontend/parser.h>
#include <frontend/symbol_table.h>

p_program frontend_trans(const char *file_name) {
    yyscan_t scanner;
    extra_info extra = (extra_info) {
        .fs = NULL,
        .p_table = NULL,
    };
    yylex_init_extra(&extra, &scanner);
    frontend_push_file(file_name, NULL, &extra, scanner);

    yyparse(scanner);
    p_program p_program = yyget_extra(scanner)->p_table->p_program;
    symbol_table_drop(yyget_extra(scanner)->p_table);

    yylex_destroy(scanner);
    return p_program;
}
