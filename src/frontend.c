#include <frontend/lexer.h>
#include <frontend/parser.h>
#include <frontend/syntax_gen.h>

#include <program/gen.h>

p_program frontend_trans(const char *file_name) {
    yyscan_t scanner;
    p_syntax_info p_info = syntax_info_gen();
    yylex_init_extra(p_info, &scanner);
    yyrestart(fopen(file_name, "r"), scanner);

    yyparse(scanner);
    p_program p_program = p_info->p_program;
    program_mir_set_vmem_id(p_program);
    syntax_info_drop(p_info);

    yylex_destroy(scanner);
    return p_program;
}
